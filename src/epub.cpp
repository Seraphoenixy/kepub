#include "epub.h"

#include <unistd.h>
#include <wait.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <boost/algorithm/string.hpp>

#include "data.h"
#include "download.h"
#include "error.h"
#include "parse_html.h"
#include "trans.h"
#include "util.h"

namespace kepub {

void Epub::set_book_name(const std::string& book_name) {
  book_name_ = trans_str(book_name);
}

void Epub::set_author(const std::string& author) {
  author_ = trans_str(author);
}

void Epub::set_description(const std::vector<std::string>& description) {
  description_.clear();
  for (const auto& line : description) {
    push_back(description_, trans_str(line));
  }
}

void Epub::generate() const {
  do_generate();

  std::filesystem::path root = book_name_;

  // content.opf
  std::ofstream content_ofs(root / "OEBPS" / "content.opf");
  check_and_write_file(content_ofs, generate_content_opf(std::size(contents_)));

  // toc.ncx
  std::vector<std::string> titles;
  for (const auto& item : contents_) {
    titles.push_back(item.get_title());
  }

  std::ofstream toc_ofs(root / "OEBPS" / "toc.ncx");
  check_and_write_file(toc_ofs, generate_toc_ncx(titles));

  std::int32_t id = 1;
  for (const auto& item : contents_) {
    std::ofstream ofs(root / "OEBPS" / "Text" / num_to_chapter_name(id++));
    check_and_write_file(ofs, generate_chapter(item));
  }
}

std::string Epub::generate_content_opf(std::size_t size) const {
  boost::replace_all(content_str, "@creator@", creator_);
  boost::replace_all(content_str, "@book_name@", book_name_);
  boost::replace_all(content_str, "@author@", author_);
  boost::replace_all(content_str, "@description@",
                     boost::join(description_, "\n"));
  boost::replace_all(
      content_str, "@date@",
      fmt::format("{:%Y-%m-%d}", fmt::localtime(std::time(nullptr))));

  XHTML xhtml(content_str);
  xhtml.move_by_name("manifest");

  for (std::size_t i = 1; i <= size; ++i) {
    kepub::Node node("item");
    node.add_attr("id", num_to_chapter_name(i));
    node.add_attr("href", "Text/" + num_to_chapter_name(i));
    node.add_attr("media-type", "application/xhtml+xml");
    xhtml.push_back(node);
  }

  xhtml.previous();
  xhtml.move_by_name("spine");
  for (std::size_t i = 1; i <= size; ++i) {
    kepub::Node node("itemref");
    node.add_attr("idref", num_to_chapter_name(i));
    xhtml.push_back(node);
  }

  return xhtml.to_string();
}

void Epub::set_creator(const std::string& creator) { creator_ = creator; }

std::string Epub::generate_introduction() const {
  XHTML xhtml(introduction_str);
  xhtml.move_by_name("body");
  xhtml.move_by_name("div");

  for (const auto& item : description_) {
    Node node("p");
    node.set_text(item);
    xhtml.push_back(node);
  }

  return xhtml.to_string();
}

std::string Epub::generate_chapter(const Content& content) {
  auto str =
      boost::replace_all_copy(chapter_str, "@title@", content.get_title());

  XHTML xhtml(str);
  xhtml.move_by_name("body");
  xhtml.move_by_name("div");

  for (const auto& item : content.get_lines()) {
    Node node("p");
    node.set_text(item);
    xhtml.push_back(node);
  }

  return xhtml.to_string();
}

void Epub::add_content(const Content& content) { contents_.push_back(content); }

std::string Epub::generate_toc_ncx(
    const std::vector<std::string>& titles) const {
  boost::replace_all(toc_str, "@book_name@", book_name_);
  boost::replace_all(toc_str, "@author@", author_);

  XHTML xhtml(toc_str);
  xhtml.move_by_name("navMap");

  auto size = std::size(titles);
  std::int32_t id = 3;
  for (std::size_t i = 0; i < size; ++i) {
    Node node("navPoint");
    node.add_attr("id", "navPoint-" + std::to_string(id));
    node.add_attr("playOrder", std::to_string(id++));

    Node nav_label("navLabel");
    Node text("text");
    text.set_text(titles[i]);
    nav_label.add_child(text);

    Node content("content");
    content.add_attr("src", "Text/" + num_to_chapter_name(i + 1));
    node.add_child(nav_label);
    node.add_child(content);

    xhtml.push_back(node);
  }

  return xhtml.to_string();
}

void Epub::generate_for_web(
    const std::vector<std::string>& titles,
    const std::vector<std::string>& urls,
    std::function<std::vector<std::string>(const std::string& url)> get_text)
    const {
  do_generate();

  std::filesystem::path root = book_name_;
  auto size = std::size(titles);

  // cover.jpg
  if (!std::empty(cover_url_)) {
    get_file(cover_url_, root / "OEBPS" / "Images" / "cover.jpg");
  }

  // content.opf
  std::ofstream content_ofs(root / "OEBPS" / "content.opf");
  check_and_write_file(content_ofs, generate_content_opf(size));

  // toc.ncx
  std::ofstream toc_ofs(root / "OEBPS" / "toc.ncx");
  check_and_write_file(toc_ofs, generate_toc_ncx(titles));

  for (std::size_t i = 0; i < size; ++i) {
    auto pid = fork();
    if (pid < 0) {
      error("fork error");
    } else if (pid == 0) {
      Content content(titles[i]);

      auto text_url = urls[i];
      if (!text_url.starts_with("https://www.esjzone.cc/")) {
        kepub::error("url error: {}", text_url);
      }
      content.push_vec(get_text(text_url));

      std::ofstream ofs(root / "OEBPS" / "Text" / num_to_chapter_name(i + 1));
      check_and_write_file(ofs, generate_chapter(content));

      std::exit(EXIT_SUCCESS);
    }
  }

  std::int32_t status{};

  while (waitpid(-1, &status, 0) > 0) {
    if (!WIFEXITED(status) || WEXITSTATUS(status)) {
      error("waitpid Error");
    }
  }
}

void Epub::do_generate() const {
  if (std::filesystem::exists(book_name_) &&
      std::filesystem::is_directory(book_name_)) {
    if (std::filesystem::remove_all(book_name_) == 0) {
      error("can not remove directory: {}", book_name_);
    }
  }

  std::filesystem::path root = book_name_;
  create_dir(root);
  create_dir(root / "META-INF");
  create_dir(root / "OEBPS");
  create_dir(root / "OEBPS" / "Fonts");
  create_dir(root / "OEBPS" / "Images");
  create_dir(root / "OEBPS" / "Styles");
  create_dir(root / "OEBPS" / "Text");

  // container.xml
  std::ofstream container_ofs(root / "META-INF" / "container.xml");
  check_and_write_file(container_ofs, container_str);

  // DFNMing.ttf
  std::ofstream font_ofs(root / "OEBPS" / "Fonts" / "DFNMing.ttf",
                         std::ofstream::binary);
  check_and_write_file(font_ofs, DFNMing_str);
  font_ofs.close();

  // DFPMingLight.ttf
  font_ofs.open(root / "OEBPS" / "Fonts" / "DFPMingLight.ttf",
                std::ofstream::binary);
  check_and_write_file(font_ofs, DFPMingLight_str);
  font_ofs.close();

  // FZCYS.ttf
  font_ofs.open(root / "OEBPS" / "Fonts" / "FZCYS.ttf", std::ofstream::binary);
  check_and_write_file(font_ofs, FZCYS_str);
  font_ofs.close();

  // SourceHanSansCN_Normal.ttf
  font_ofs.open(root / "OEBPS" / "Fonts" / "SourceHanSansCN_Normal.ttf",
                std::ofstream::binary);
  check_and_write_file(font_ofs, SourceHanSansCN_Normal_str);

  // style.css
  std::ofstream styles_ofs(root / "OEBPS" / "Styles" / "style.css");
  check_and_write_file(styles_ofs, style_str);

  // mimetype
  std::ofstream mimetype_ofs(root / "mimetype");
  check_and_write_file(mimetype_ofs, mimetype_str);

  // cover.xhtml
  std::ofstream cover_ofs(root / "OEBPS" / "Text" / "cover.xhtml");
  check_and_write_file(cover_ofs, cover_str);

  // introduction.xhtml
  std::ofstream introduction_ofs(root / "OEBPS" / "Text" /
                                 "introduction.xhtml");
  check_and_write_file(introduction_ofs, generate_introduction());
}

void Epub::set_cover_url(const std::string& url) { cover_url_ = url; }

Content::Content(const std::string& title,
                 const std::vector<std::string>& lines) {
  title_ = trans_str(title);
  push_vec(lines);
}

void Content::push_line(const std::string& line) {
  kepub::push_back(lines_, trans_str(line));
}

void Content::push_vec(const std::vector<std::string>& lines) {
  for (const auto& item : lines) {
    push_line(item);
  }
}

const std::string& Content::get_title() const { return title_; }

const std::vector<std::string>& Content::get_lines() const { return lines_; }

}  // namespace kepub
