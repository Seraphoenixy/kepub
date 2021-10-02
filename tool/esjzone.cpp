#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <klib/error.h>
#include <klib/html.h>
#include <klib/http.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "trans.h"
#include "util.h"

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

klib::Response http_get(const std::string &url) {
  static klib::Request request;
  request.set_no_proxy();
  request.set_browser_user_agent();
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url);
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("HTTP GET fail: {}", response.status_code());
  }

  return response;
}

pugi::xml_document get_xml(const std::string &url) {
  auto response = http_get(url);

  auto xml = klib::html_tidy(response.text(), true);
  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  return doc;
}

std::tuple<std::string, std::string, std::vector<std::string>,
           std::vector<std::pair<std::string, std::string>>>
get_info(const std::string &book_id, bool connect_chinese) {
  auto doc = get_xml("https://www.esjzone.cc/detail/" + book_id + ".html");

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                     "mb-3']/div[@class='col-md-9 book-detail']/h2")
                  .node();
  auto book_name = kepub::trans_str(node.text().as_string());

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-9 book-detail']/ul")
             .node();

  std::string author;
  std::string prefix = "作者:";
  for (const auto &child : node.children()) {
    if (child.child("strong").text().as_string() == prefix) {
      author = kepub::trans_str(child.child("a").text().as_string());
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
    kepub::push_back(description, kepub::trans_str(child.text().as_string()),
                     connect_chinese);
  }

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "padding-top-1x mb-3']/div/div/div[@class='tab-pane fade "
                "active show']/div[@id='chapterList']")
             .node();

  std::vector<std::pair<std::string, std::string>> titles_and_urls;
  for (const auto &child : node.children("a")) {
    auto title = kepub::trans_str(child.child("p").text().as_string());

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

  spdlog::info("Book name: {}", book_name);
  spdlog::info("Author: {}", author);
  spdlog::info("Cover url: {}", cover_url);

  return {book_name, author, description, titles_and_urls};
}

std::vector<std::string> get_content(const std::string &url,
                                     bool connect_chinese) {
  auto doc = get_xml(url);

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 "
                     "p-r-30']/div[@class='forum-content mt-3']")
                  .node();

  std::vector<std::string> result;
  for (const auto &line : get_children_content(node)) {
    kepub::push_back(result, kepub::trans_str(line), connect_chinese);
  }

  return result;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);
  auto [book_name, author, description, titles_and_urls] =
      get_info(book_id, options.connect_chinese_);

  std::vector<std::pair<std::string, std::string>> chapters;
  for (const auto &[title, urls] : titles_and_urls) {
    chapters.emplace_back(
        title, boost::join(get_content(urls, options.connect_chinese_), "\n"));

    spdlog::info("Successfully obtained chapter: {}", title);
  }

  kepub::generate_txt(book_name, author, description, chapters);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
