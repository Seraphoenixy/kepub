#include <filesystem>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "compress.h"
#include "download.h"
#include "epub.h"
#include "error.h"
#include "parse_xml.h"
#include "trans.h"
#include "util.h"

// urls titles book_name author cover description
std::tuple<std::vector<std::string>, std::vector<std::string>, std::string,
           std::string, std::string, std::vector<std::string>>
get_content(const std::string &url) {
  std::vector<std::string> urls;
  std::vector<std::string> titles;
  std::string book_name;
  std::string author;
  std::string cover_url;
  std::vector<std::string> description;

  kepub::XHTML html(kepub::html_tidy(kepub::get_page(url)));

  html.move_by_name("body");
  html.move_by_attr_class("div", "offcanvas-wrapper");
  html.move_by_attr_class("section", "container");
  html.move_by_attr_class("div", "row");
  html.move_by_attr_class("div", "col-xl-9 col-lg-8 p-r-30");
  html.move_by_attr_class("div", "row mb-3");
  html.move_by_attr_class("div", "col-md-9 book-detail");
  html.move_by_attr_class("h2", "p-t-10 text-normal");
  book_name = kepub::trans_str(html.get_text());

  html.previous();
  html.move_by_attr_class("ul", "list-unstyled mb-2 book-detail");

  std::string prefix = "作者:";
  for (const auto &line : html.get_children_text("li")) {
    if (line.starts_with(prefix)) {
      author = kepub::trans_str(line.substr(std::size(prefix)));
    }
  }

  html.previous();
  html.previous();
  html.move_by_attr_class("div", "col-md-3");
  html.move_by_attr_class("div", "product-gallery text-center mb-3");
  html.move_by_name("a");
  cover_url = html.first_child_attr("src");

  html.previous();
  html.previous();
  html.previous();
  html.previous();
  html.move_by_attr_class("div", "bg-secondary p-20 margin-top-1x");
  html.move_by_attr_class("div", "row");
  html.move_by_attr_class("div", "col-md-12");
  html.move_by_attr_class("div", "description");
  for (const auto &line : html.get_children_text()) {
    kepub::push_back(description, kepub::trans_str(line));
  }

  html.previous();
  html.previous();
  html.previous();
  html.previous();
  html.move_by_attr_class("div", "row padding-top-1x mb-3");
  html.move_by_attr_class("div", "col-lg-12");
  html.move_by_attr_class("div", "tab-content");
  html.move_by_attr_class("div", "tab-pane fade active show");
  html.move_by_attr("div", "id", "chapterList");

  for (const auto &line : html.get_children_text("a")) {
    auto temp = kepub::trans_str(line);
    if (std::empty(temp)) {
      kepub::error("the title is empty");
    }
    titles.push_back(temp);
  }

  for (const auto &item : html.get_children_attr("a", "href")) {
    urls.push_back(item);
  }

  if (std::empty(urls)) {
    kepub::error("urls is empty");
  }
  if (std::empty(titles)) {
    kepub::error("titles is empty");
  }
  if (std::empty(book_name)) {
    kepub::error("book name is empty");
  }
  if (std::empty(author)) {
    kepub::error("author name is empty");
  }
  if (std::empty(cover_url)) {
    kepub::error("cover url is empty");
  }
  if (std::empty(description)) {
    kepub::error("description is empty");
  }

  if (kepub::max_chapter > 0) {
    urls.resize(kepub::max_chapter);
    titles.resize(kepub::max_chapter);
  }

  for (const auto &item : urls) {
    kepub::check_is_url(item);
  }

  if (std::size(urls) != std::size(titles)) {
    kepub::error("the number of urls != the number of titles: {} vs {}",
                 std::size(urls), std::size(titles));
  }

  return {urls, titles, book_name, author, cover_url, description};
}

std::vector<std::string> get_text(const std::string &url) {
  std::vector<std::string> result;

  kepub::XHTML html(kepub::html_tidy(kepub::get_page(url)));

  html.move_by_name("body");
  html.move_by_attr_class("div", "offcanvas-wrapper");
  html.move_by_attr_class("section", "container");
  html.move_by_attr_class("div", "row");
  html.move_by_attr_class("div", "col-xl-9 col-lg-8 p-r-30");
  html.move_by_attr_class("div", "forum-content mt-3");

  for (const auto &line : html.get_children_text()) {
    kepub::push_back(result, kepub::trans_str(line));
  }

  if (std::empty(result)) {
    kepub::error("text result is empty");
  }

  return result;
}

int main(int argc, char *argv[]) try {
  auto url = kepub::processing_cmd(argc, argv);
  kepub::check_is_url(url);

  auto [urls, titles, book_name, author, cover_url, description] =
      get_content(url);

  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name(book_name);
  epub.set_author(author);
  epub.set_cover_url(cover_url);
  epub.set_description(description);

  epub.generate_for_web(titles, urls, get_text);

  kepub::compress(book_name);
  std::filesystem::rename(book_name + ".zip", book_name + ".epub");
} catch (const std::exception &err) {
  kepub::error(err.what());
} catch (...) {
  kepub::error("unknown exception");
}
