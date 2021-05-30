#include "epub.h"

#include <unistd.h>
#include <wait.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <boost/algorithm/string.hpp>

#include "data.h"
#include "download.h"
#include "error.h"
#include "util.h"

namespace {

std::string get_date() {
  if (!std::empty(kepub::data)) {
    return kepub::data;
  }

  return fmt::format("{:%Y-%m-%d}", fmt::localtime(std::time(nullptr)));
}

}  // namespace

namespace kepub {

Content::Content(const std::string& title,
                 const std::vector<std::string>& lines)
    : title_(title) {
  push_lines(lines);
}

void Content::push_line(const std::string& line) { push_back(lines_, line); }

void Content::push_lines(const std::vector<std::string>& lines) {
  for (const auto& line : lines) {
    push_line(line);
  }
}

const std::string& Content::get_title() const { return title_; }

const std::vector<std::string>& Content::get_lines() const { return lines_; }

void Epub::set_creator(const std::string& creator) { creator_ = creator; }

void Epub::set_book_name(const std::string& book_name) {
  book_name_ = book_name;
}

void Epub::set_author(const std::string& author) { author_ = author; }

void Epub::set_description(const std::vector<std::string>& description) {
  description_.clear();

  for (const auto& line : description) {
    push_back(description_, line);
  }
}

void Epub::set_cover_url(const std::string& url) { cover_url_ = url; }

void Epub::add_content(const Content& content) { contents_.push_back(content); }

void Epub::generate() {
  common_generate();

  generate_cover();
  generate_message();
  generate_introduction();
  generate_illustration();
  generate_chapter();
  generate_postscript();

  std::ofstream content_ofs(root_ / "OEBPS" / "content.opf");
  check_and_write_file(content_ofs, content_opf_.to_string());

  std::ofstream toc_ofs(root_ / "OEBPS" / "toc.ncx");
  check_and_write_file(toc_ofs, toc_ncx_.to_string());
}

void Epub::generate_for_web(
    const std::vector<std::string>& titles,
    const std::vector<std::string>& urls,
    const std::function<std::vector<std::string>(const std::string& url)>&
        get_text) {
  common_generate();

  // cover.jpg
  if (download_cover) {
    get_file(cover_url_, root_ / "OEBPS" / "Images" / "cover.jpg");
  }

  generate_cover();
  generate_message();
  generate_introduction();
  generate_illustration();

  std::int32_t id = 1;
  for (const auto& title : titles) {
    auto file_name = num_to_chapter_name(id++);
    Epub::add_file_in_content_opf(content_opf_, file_name, "Text/" + file_name,
                                  "application/xhtml+xml");
    Epub::add_nav_point(toc_ncx_, title, "Text/" + file_name);
  }

#ifdef KEPUB_NO_FORK
  auto size = std::size(titles);
  for (std::size_t i = 0; i < size; ++i) {
    Content content(titles[i]);
    content.push_lines(get_text(urls[i]));

    std::ofstream ofs(root_ / "OEBPS" / "Text" / num_to_chapter_name(i + 1));
    check_and_write_file(ofs, generate_chapter(content));
  }
#else
  auto size = std::size(titles);
  for (std::size_t i = 0; i < size; ++i) {
    auto pid = fork();
    if (pid < 0) {
      error("fork error");
    } else if (pid == 0) {
      Content content(titles[i]);
      content.push_lines(get_text(urls[i]));

      std::ofstream ofs(root_ / "OEBPS" / "Text" / num_to_chapter_name(i + 1));
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
#endif

  generate_postscript();

  std::ofstream content_ofs(root_ / "OEBPS" / "content.opf");
  check_and_write_file(content_ofs, content_opf_.to_string());

  std::ofstream toc_ofs(root_ / "OEBPS" / "toc.ncx");
  check_and_write_file(toc_ofs, toc_ncx_.to_string());
}

std::string Epub::generate_chapter(const Content& content) {
  XHTML xhtml(chapter_str);
  xhtml.move_by_name("body");
  xhtml.move_by_name("div");

  for (const auto& item : content.get_lines()) {
    xhtml.push_back(item);
  }

  return boost::replace_all_copy(xhtml.to_string(), "@title@",
                                 content.get_title());
}

void Epub::add_file_in_content_opf(XHTML& content_opf, const std::string& id,
                                   const std::string& href,
                                   const std::string& media_type) {
  content_opf.reset();
  content_opf.move_by_name("manifest");

  content_opf.push_back(
      "item", {{"id", id}, {"href", href}, {"media-type", media_type}});

  if (href.ends_with(".xhtml")) {
    content_opf.previous();
    content_opf.move_by_name("spine");
    content_opf.push_back("itemref", {{"idref", id}});
  }
}

void Epub::add_nav_point(XHTML& toc_ncx, const std::string& title,
                         const std::string& src) {
  toc_ncx.reset();
  toc_ncx.move_by_name("navMap");

  auto start_id =
      (toc_ncx.has_child() ? std::stoi(toc_ncx.last_child_attr("playOrder")) + 1
                           : 1);

  Node node("navPoint");
  node.add_attr("id", "navPoint-" + std::to_string(start_id));
  node.add_attr("playOrder", std::to_string(start_id));

  Node nav_label("navLabel");
  Node text("text");
  text.set_text(title);
  nav_label.add_child(text);

  Node content("content");
  content.add_attr("src", src);

  node.add_child(nav_label);
  node.add_child(content);

  toc_ncx.push_back(node);
}

void Epub::common_generate() {
  root_ = book_name_;

  create_dir(root_);
  create_dir(root_ / "META-INF");
  create_dir(root_ / "OEBPS");
  create_dir(root_ / "OEBPS" / "Fonts");
  create_dir(root_ / "OEBPS" / "Images");
  create_dir(root_ / "OEBPS" / "Styles");
  create_dir(root_ / "OEBPS" / "Text");

  // container.xml
  std::ofstream container_ofs(root_ / "META-INF" / "container.xml");
  check_and_write_file(container_ofs, container_str);

  // DFNMing.ttf
  std::ofstream font_ofs(root_ / "OEBPS" / "Fonts" / "DFNMing.ttf",
                         std::ofstream::binary);
  check_and_write_file(font_ofs, DFNMing_str);
  font_ofs.close();

  // DFPMingLight.ttf
  font_ofs.open(root_ / "OEBPS" / "Fonts" / "DFPMingLight.ttf",
                std::ofstream::binary);
  check_and_write_file(font_ofs, DFPMingLight_str);
  font_ofs.close();

  // FZCYS.ttf
  font_ofs.open(root_ / "OEBPS" / "Fonts" / "FZCYS.ttf", std::ofstream::binary);
  check_and_write_file(font_ofs, FZCYS_str);
  font_ofs.close();

  // SourceHanSansCN-Normal.ttf
  font_ofs.open(root_ / "OEBPS" / "Fonts" / "SourceHanSansCN-Normal.ttf",
                std::ofstream::binary);
  check_and_write_file(font_ofs, SourceHanSansCN_Normal_str);

  // style.css
  std::ofstream styles_ofs(root_ / "OEBPS" / "Styles" / "style.css");
  check_and_write_file(styles_ofs, style_str);

  // mimetype
  std::ofstream mimetype_ofs(root_ / "mimetype");
  check_and_write_file(mimetype_ofs, mimetype_str);

  // content.opf
  content_opf_ = XHTML(content_str);
  content_opf_.move_by_name("metadata");

  content_opf_.set_child_text("dc:title", book_name_);
  content_opf_.set_child_attr("dc:creator", "opf:file-as", creator_);
  content_opf_.set_child_text("dc:creator", author_);
  content_opf_.set_child_text("dc:rights", creator_);
  content_opf_.set_child_text("dc:description",
                              boost::join(description_, "\n"));
  content_opf_.set_child_text("dc:date", get_date());

  // toc.ncx
  toc_ncx_ = XHTML(toc_str);

  toc_ncx_.move_by_name("docTitle");
  toc_ncx_.set_child_text("text", book_name_);

  toc_ncx_.previous();
  toc_ncx_.move_by_name("docAuthor");
  toc_ncx_.set_child_text("text", author_);
}

void Epub::generate_cover() {
  if (no_cover) {
    return;
  }

  std::ofstream cover_ofs(root_ / "OEBPS" / "Text" / "cover.xhtml");
  check_and_write_file(cover_ofs, cover_str);

  content_opf_.reset();
  content_opf_.move_by_name("metadata");
  content_opf_.push_back("meta", {{"name", "cover"}, {"content", "cover.jpg"}});

  Epub::add_file_in_content_opf(content_opf_, "cover.xhtml", "Text/cover.xhtml",
                                "application/xhtml+xml");
  Epub::add_file_in_content_opf(content_opf_, "cover.jpg", "Images/cover.jpg",
                                "image/jpeg");

  content_opf_.previous();
  content_opf_.move_by_name("guide");
  content_opf_.push_back(
      "reference",
      {{"type", "cover"}, {"title", "Cover"}, {"href", "Text/cover.xhtml"}});

  Epub::add_nav_point(toc_ncx_, "封面", "Text/cover.xhtml");
}

void Epub::generate_message() {
  boost::replace_all(message_str, "@creator@", creator_);

  std::ofstream message_ofs(root_ / "OEBPS" / "Text" / "message.xhtml");
  check_and_write_file(message_ofs, message_str);

  Epub::add_file_in_content_opf(content_opf_, "message.xhtml",
                                "Text/message.xhtml", "application/xhtml+xml");
  Epub::add_nav_point(toc_ncx_, "制作信息", "Text/message.xhtml");
}

void Epub::generate_introduction() {
  XHTML xhtml(introduction_str);
  xhtml.move_by_name("body");
  xhtml.move_by_name("div");

  for (const auto& item : description_) {
    xhtml.push_back(item);
  }

  std::ofstream introduction_ofs(root_ / "OEBPS" / "Text" /
                                 "introduction.xhtml");
  check_and_write_file(introduction_ofs, xhtml.to_string());

  Epub::add_file_in_content_opf(content_opf_, "introduction.xhtml",
                                "Text/introduction.xhtml",
                                "application/xhtml+xml");
  Epub::add_nav_point(toc_ncx_, "简介", "Text/introduction.xhtml");
}

void Epub::generate_illustration() {
  for (std::int32_t i = 1; i <= illustration_num; ++i) {
    auto copy = boost::replace_all_copy(illustration_str, "@title@",
                                        "彩页" + num_to_str(i));
    boost::replace_all(copy, "@num@", num_to_str(i));

    auto file_name = num_to_illustration_name(i);

    std::ofstream illustration_ofs(root_ / "OEBPS" / "Text" / file_name);
    check_and_write_file(illustration_ofs, copy);

    Epub::add_file_in_content_opf(content_opf_, file_name, "Text/" + file_name,
                                  "application/xhtml+xml");

    if (i == 1) {
      Epub::add_nav_point(toc_ncx_, "彩页", "Text/" + file_name);
    }
  }
}

void Epub::generate_chapter() {
  std::int32_t id = 1;
  for (const auto& item : contents_) {
    XHTML xhtml(chapter_str);
    xhtml.move_by_name("head");
    xhtml.set_child_text("title", item.get_title());

    xhtml.previous();
    xhtml.move_by_name("body");
    xhtml.move_by_name("div");

    xhtml.set_child_text("h1", item.get_title());

    for (const auto& line : item.get_lines()) {
      xhtml.push_back(line);
    }

    auto file_name = num_to_chapter_name(id++);
    std::ofstream ofs(root_ / "OEBPS" / "Text" / file_name);
    check_and_write_file(ofs, xhtml.to_string());

    Epub::add_file_in_content_opf(content_opf_, file_name, "Text/" + file_name,
                                  "application/xhtml+xml");
    Epub::add_nav_point(toc_ncx_, item.get_title(), "Text/" + file_name);
  }
}

void Epub::generate_postscript() {
  if (!postscript) {
    return;
  }

  std::ofstream postscript_ofs(root_ / "OEBPS" / "Text" / "postscript.xhtml");
  check_and_write_file(postscript_ofs, postscript_str);

  Epub::add_file_in_content_opf(content_opf_, "postscript.xhtml",
                                "Text/postscript.xhtml",
                                "application/xhtml+xml");
  Epub::add_nav_point(toc_ncx_, "后记", "Text/postscript.xhtml");
}

}  // namespace kepub
