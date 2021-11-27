#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <klib/html.h>
#include <klib/http.h>
#include <klib/log.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "progress_bar.h"
#include "trans.h"
#include "util.h"
#include "version.h"

namespace {

void get_node_content(pugi::xml_node node, std::string &str) {
  if (node.children().begin() == node.children().end()) {
    str += node.text().as_string();
  } else {
    for (const auto &child : node.children()) {
      if (node.name() == std::string("p") || node.name() == std::string("br")) {
        str += "\n";
      }
      get_node_content(child, str);
    }
  }
}

std::vector<std::string> get_children_content(const pugi::xml_node &node) {
  std::vector<std::string> result;

  for (const auto &child : node.children()) {
    std::string str;
    get_node_content(child, str);
    result.push_back(str);
  }

  if (std::empty(result)) {
    klib::error("get_children_text no data");
  }

  return result;
}

klib::Response http_get(const std::string &url, const std::string &proxy) {
  static klib::Request request;
  request.set_browser_user_agent();
  if (!std::empty(proxy)) {
    request.set_proxy(proxy);
  } else {
    request.set_no_proxy();
  }
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url);
  if (!response.ok()) {
    klib::error("HTTP GET fail: {}", response.status_code());
  }

  return response;
}

pugi::xml_document get_xml(const std::string &url, const std::string &proxy) {
  auto response = http_get(url, proxy);

  auto xml = klib::html_tidy(response.text(), true);
  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  return doc;
}

std::tuple<std::string, std::string, std::vector<std::string>,
           std::vector<std::pair<std::string, std::string>>>
get_info(const std::string &book_id, bool translation,
         const std::string &proxy) {
  auto doc =
      get_xml("https://www.esjzone.cc/detail/" + book_id + ".html", proxy);

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                     "mb-3']/div[@class='col-md-9 book-detail']/h2")
                  .node();
  auto book_name = kepub::trans_str(node.text().as_string(), translation);

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-9 book-detail']/ul")
             .node();

  std::string author;
  std::string prefix = "作者:";
  for (const auto &child : node.children()) {
    if (child.child("strong").text().as_string() == prefix) {
      author =
          kepub::trans_str(child.child("a").text().as_string(), translation);
    }
  }

  node =
      doc.select_node(
             "/html/body/div[@class='offcanvas-wrapper']/section/div/"
             "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='bg-secondary "
             "p-20 margin-top-1x']/div/div/div")
          .node();

  std::vector<std::string> description;
  for (const auto &child : node.children()) {
    kepub::push_back(description,
                     kepub::trans_str(child.text().as_string(), translation),
                     false);
  }

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "padding-top-1x mb-3']/div/div/div[@class='tab-pane fade "
                "active show']/div[@id='chapterList']")
             .node();

  std::vector<std::pair<std::string, std::string>> titles_and_urls;
  for (const auto &child : node.children("a")) {
    auto title =
        kepub::trans_str(child.child("p").text().as_string(), translation);

    std::string may_be_url = child.attribute("href").as_string();
    if (!may_be_url.starts_with("https://www.esjzone.cc/")) {
      klib::warn("url error: {}, title: {}", may_be_url, title);
    } else {
      titles_and_urls.emplace_back(title, may_be_url);
    }
  }

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-3']/div[@class='product-gallery "
                "text-center mb-3']/a/img")
             .node();
  std::string cover_url = node.attribute("src").as_string();

  klib::info("Book name: {}", book_name);
  klib::info("Author: {}", author);
  klib::info("Cover url: {}", cover_url);

  std::string cover_name = "cover.jpg";
  auto response = http_get(cover_url, proxy);
  response.save_to_file(cover_name, true);
  klib::info("Cover downloaded successfully: {}", cover_name);

  return {book_name, author, description, titles_and_urls};
}

std::vector<std::string> get_content(const std::string &url, bool translation,
                                     const std::string &proxy) {
  auto doc = get_xml(url, proxy);

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 "
                     "p-r-30']/div[@class='forum-content mt-3']")
                  .node();

  std::vector<std::string> result;
  for (const auto &line : get_children_content(node)) {
    kepub::push_back(result, kepub::trans_str(line, translation), false);
  }

  return result;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string book_id;
  app.add_option("book-id", book_id, "The book id of the book to be downloaded")
      ->required();

  bool translation = false;
  app.add_flag("-t,--translation", translation,
               "Translate Traditional Chinese to Simplified Chinese");

  std::string proxy;
  app.add_flag("-p{http://127.0.0.1:1080},--proxy{http://127.0.0.1:1080}",
               proxy, "Use proxy")
      ->expected(0, 1);

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);
  if (!std::empty(proxy)) {
    klib::info("Use proxy: {}", proxy);
  }

  auto [book_name, author, description, titles_and_urls] =
      get_info(book_id, translation, proxy);

  klib::info("Start downloading novel content");
  kepub::ProgressBar bar(book_name, std::size(titles_and_urls));
  std::vector<std::pair<std::string, std::string>> chapters;
  for (const auto &[title, urls] : titles_and_urls) {
    bar.set_postfix_text(title);
    chapters.emplace_back(
        title, boost::join(get_content(urls, translation, proxy), "\n"));
    bar.tick();
  }

  kepub::generate_txt(book_name, author, description, chapters);
  klib::info("Novel '{}' download completed", book_name);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
