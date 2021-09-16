#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <klib/error.h>
#include <klib/html.h>
#include <klib/http.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "trans.h"
#include "util.h"

namespace {

void get_text(pugi::xml_node node, std::string &str) {
  if (node.children().begin() == node.children().end()) {
    str += node.text().as_string();
  } else {
    for (const auto &child : node.children()) {
      if (node.name() == std::string("p") || node.name() == std::string("br")) {
        str += "\n";
      }
      get_text(child, str);
    }
  }
}

std::vector<std::string> get_children_text(const pugi::xml_node &node) {
  std::vector<std::string> result;

  for (const auto &child : node.children()) {
    std::string str;
    get_text(child, str);
    result.push_back(str);
  }

  if (std::empty(result)) {
    klib::error("get_children_text no data");
  }

  return result;
}

std::tuple<std::string, std::string, std::vector<std::string>,
           std::vector<std::pair<std::string, std::string>>>
get_content(const std::string &book_id, bool connect_chinese) {
  std::string book_name;
  std::string author;
  std::vector<std::string> description;
  std::vector<std::pair<std::string, std::string>> titles_and_urls;

  auto url = "https://www.esjzone.cc/detail/" + book_id + ".html";

  klib::Request request;
  request.set_no_proxy();
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url);
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("Status code is not ok: {}, url: {}", response.status_code(),
                url);
  }

  auto xml = klib::html_tidy(response.text());
  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                     "mb-3']/div[@class='col-md-9 book-detail']/h2")
                  .node();
  book_name = kepub::trans_str(node.text().as_string());

  node = doc.select_node(
                "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                "div[@class='col-xl-9 col-lg-8 p-r-30']/div[@class='row "
                "mb-3']/div[@class='col-md-9 book-detail']/ul")
             .node();

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

  for (const auto &child : node.children("a")) {
    auto title = kepub::trans_str(child.child("p").text().as_string());
    kepub::check_chapter_name(title);
    if (std::empty(title)) {
      klib::error("the title is empty");
    }

    std::string may_be_url = child.attribute("href").as_string();
    if (!may_be_url.starts_with("https://www.esjzone.cc/")) {
      klib::warn("url error: {}, title: {}", may_be_url, title);
    } else {
      titles_and_urls.emplace_back(title, may_be_url);
    }
  }

  if (std::empty(titles_and_urls)) {
    klib::error("titles_and_urls is empty");
  }
  if (std::empty(book_name)) {
    klib::error("book name is empty");
  }
  if (std::empty(author)) {
    klib::error("author name is empty");
  }
  if (std::empty(description)) {
    klib::error("description is empty");
  }

  return {book_name, author, description, titles_and_urls};
}

std::vector<std::string> get_text(const std::string &url,
                                  bool connect_chinese) {
  std::vector<std::string> result;

  klib::Request request;
  request.set_no_proxy();
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url);
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("Status code is not ok: {}, url: {}", response.status_code(),
                url);
  }

  auto xml = klib::html_tidy(response.text());
  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  auto node = doc.select_node(
                     "/html/body/div[@class='offcanvas-wrapper']/section/div/"
                     "div[@class='col-xl-9 col-lg-8 "
                     "p-r-30']/div[@class='forum-content mt-3']")
                  .node();

  for (const auto &line : get_children_text(node)) {
    kepub::push_back(result, kepub::trans_str(line), connect_chinese);
  }

  if (std::empty(result)) {
    klib::error("text result is empty, url: {}", url);
  }

  return result;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);
  auto [book_name, author, description, titles_and_urls] =
      get_content(book_id, options.connect_chinese_);

  std::filesystem::create_directory(book_name);
  auto p = std::make_unique<klib::ChangeWorkingDir>(book_name);

  for (const auto &[title, urls] : titles_and_urls) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(200ms);

    auto pid = fork();
    if (pid < 0) {
      klib::error("Fork error");
    } else if (pid == 0) {
      klib::write_file(
          title, false,
          boost::join(get_text(urls, options.connect_chinese_), "\n"));
      spdlog::info("{} ok", title);
      std::exit(EXIT_SUCCESS);
    }
  }

  klib::wait_for_child_process();

  std::vector<std::pair<std::string, std::string>> v;
  for (const auto &[title, urls] : titles_and_urls) {
    v.emplace_back(title, klib::read_file(title, false));
  }

  p.reset();
  std::filesystem::remove_all(book_name);

  std::ofstream ofs(book_name + ".txt");

  ofs << book_name << '\n';
  ofs << author << '\n';

  for (const auto &line : description) {
    ofs << line << '\n';
  }

  for (const auto &[title, text] : v) {
    ofs << "[WEB] " << title << '\n';
    ofs << text << '\n';
  }
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
