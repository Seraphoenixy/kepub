#include "epub.h"

#include <cassert>
#include <ctime>
#include <filesystem>
#include <memory>

#include <dbg.h>
#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/exception.h>
#include <klib/log.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <gsl/gsl-lite.hpp>
#include <pugixml.hpp>

#include "util.h"

extern char font[];
extern int font_size;

extern char style[];
extern int style_size;

namespace kepub {

namespace {

std::string num_to_volume_name(std::int32_t i) {
  return "volume" + num_to_str(i) + ".xhtml";
}

std::string num_to_chapter_name(std::int32_t i) {
  return "chapter" + num_to_str(i) + ".xhtml";
}

std::string num_to_illustration_name(std::int32_t i) {
  return "illustration" + num_to_str(i) + ".xhtml";
}

std::string get_date() {
  return fmt::format("{:%Y-%m-%d}", fmt::localtime(std::time(nullptr)));
}

void save_file(const pugi::xml_document &doc, std::string_view path) {
  if (!doc.save_file(path.data(), "    ")) {
    throw klib::RuntimeError("can not save: {}", path);
  }
}

bool has_children(const pugi::xml_node &node, std::string_view name) {
  auto children = node.children(name.data());
  return children.begin() != children.end();
}

void append_manifest_and_spine(pugi::xml_node &manifest, const std::string &id,
                               const std::string &href) {
  auto item = manifest.append_child("item");
  item.append_attribute("id") = id.c_str();
  item.append_attribute("href") = href.c_str();

  std::string media_type;
  if (href.ends_with("xhtml")) {
    media_type = "application/xhtml+xml";
  } else if (href.ends_with("jpg")) {
    media_type = "image/jpeg";
  } else if (href.ends_with("ncx")) {
    media_type = "application/x-dtbncx+xml";
  } else if (href.ends_with("css")) {
    media_type = "text/css";
  } else if (href.ends_with("woff2")) {
    // https://www.w3.org/publishing/epub3/epub-spec.html#sec-cmt-supported
    media_type = "font/woff2";
  } else {
    throw klib::RuntimeError("Unknown media type");
  }
  item.append_attribute("media-type") = media_type.c_str();

  if (href.ends_with("xhtml")) {
    auto package = manifest.parent();
    auto spine = package.child("spine");
    if (spine.empty()) {
      spine = package.append_child("spine");
      spine.append_attribute("toc") = "ncx";
    }

    auto itemref = spine.append_child("itemref");
    itemref.append_attribute("idref") = id.c_str();
  }
}

void append_nav_map(pugi::xml_node &nav_map, const std::string &text,
                    const std::string &src) {
  std::int32_t first = 1;

  auto last_child = nav_map.last_child();
  if (!last_child.empty()) {
    if (last_child.name() == std::string("navPoint")) {
      if (last_child.last_child().name() == std::string("navPoint")) {
        first = last_child.last_child().attribute("playOrder").as_int() + 1;
      } else {
        first = last_child.attribute("playOrder").as_int() + 1;
      }
    } else if (last_child.name() == std::string("content")) {
      first = nav_map.attribute("playOrder").as_int() + 1;
    } else {
      assert(false);
    }
  }

  auto nav_point = nav_map.append_child("navPoint");
  nav_point.append_attribute("id") =
      ("navPoint-" + std::to_string(first)).c_str();
  nav_point.append_attribute("playOrder") = std::to_string(first).c_str();

  auto nav_label = nav_point.append_child("navLabel");
  nav_label.append_child("text").text().set(text.c_str());

  nav_point.append_child("content").append_attribute("src") = src.c_str();
}

pugi::xml_document generate_declaration() {
  pugi::xml_document doc;
  auto decl = doc.prepend_child(pugi::node_declaration);
  decl.append_attribute("version") = "1.0";
  decl.append_attribute("encoding") = "UTF-8";

  return doc;
}

pugi::xml_document generate_xhtml(const std::string &title,
                                  const std::string &div_class = "",
                                  bool has_h1 = false) {
  auto doc = generate_declaration();

  doc.append_child(pugi::node_doctype)
      .set_value(
          R"(html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd")");

  auto html = doc.append_child("html");
  html.append_attribute("xmlns") = "http://www.w3.org/1999/xhtml";
  html.append_attribute("xml:lang") = "zh-CN";
  html.append_attribute("xmlns:epub") = "http://www.idpf.org/2007/ops";

  auto head = html.append_child("head");

  auto link = head.append_child("link");
  link.append_attribute("href") = "../Styles/style.css";
  link.append_attribute("rel") = "stylesheet";
  link.append_attribute("type") = "text/css";

  head.append_child("title").text() = title.c_str();

  auto body = html.append_child("body");
  auto div = body.append_child("div");

  if (!std::empty(div_class)) {
    div.append_attribute("class") = div_class.c_str();
  }

  if (has_h1) {
    auto h1 = div.append_child("h1");
    h1.append_attribute("class") = "bold";
    h1.text() = title.c_str();
  }

  return doc;
}

void append_texts(pugi::xml_document &doc,
                  const std::vector<std::string> &texts) {
  auto div = doc.select_node("/html/body/div").node();
  if (div.empty()) {
    throw klib::RuntimeError("no div");
  }

  std::string image_prefix = "[IMAGE] ";
  auto image_prefix_size = std::size(image_prefix);
  for (const auto &text : texts) {
    if (text.starts_with(image_prefix)) {
      auto image_num = text.substr(image_prefix_size);

      auto d = div.append_child("div");
      d.append_attribute("class") = "center";
      auto img = d.append_child("img");
      img.append_attribute("alt") = image_num.c_str();
      img.append_attribute("src") = ("../Images/" + image_num + ".jpg").c_str();

      div.append_child("p").append_child("br");
    } else {
      auto p = div.append_child("p");
      p.text() = text.c_str();
    }
  }
}

void generate_volume(const std::string &volume_name, std::int32_t id) {
  auto volume_file_name = num_to_volume_name(id);
  auto volume = generate_xhtml(volume_name, "", true);
  auto volume_path = std::filesystem::path(Epub::text_dir) / volume_file_name;
  save_file(volume, volume_path.c_str());
}

std::int32_t last_num(const pugi::xml_node &node, const std::string &prefix,
                      const std::string &suffix) {
  auto item = node.select_node("/package/manifest").node();
  auto prefix_size = std::size(prefix);
  auto suffix_size = std::size(suffix);

  std::vector<std::int32_t> ids;

  for (const auto &child : item.children("item")) {
    std::string id = child.attribute("id").as_string();
    if (id.starts_with(prefix)) {
      auto num_str = id.substr(prefix_size);
      num_str = num_str.substr(0, std::size(num_str) - suffix_size);
      ids.push_back(std::stoi(num_str));
    }
  }
  std::sort(std::begin(ids), std::end(ids));

  return std::empty(ids) ? 0 : ids.back();
}

void do_deal_with_content(
    pugi::xml_node &manifest, std::int32_t first_chapter_id,
    std::int32_t first_volume_id,
    const std::vector<std::tuple<std::string, std::string,
                                 std::vector<std::string>>> &content) {
  std::string last_volume_name;

  for (const auto &[volume_name, title, text] : content) {
    if (volume_name != last_volume_name) {
      auto name = num_to_volume_name(first_volume_id++);
      append_manifest_and_spine(manifest, name, "Text/" + name);
    }
    auto name = num_to_chapter_name(first_chapter_id++);
    append_manifest_and_spine(manifest, name, "Text/" + name);

    last_volume_name = volume_name;
  }
}

std::pair<std::int32_t, std::int32_t> deal_with_content(
    const std::vector<std::tuple<std::string, std::string,
                                 std::vector<std::string>>> &content) {
  pugi::xml_document doc;
  doc.load_file(Epub::content_path.data(), pugi::parse_default |
                                               pugi::parse_declaration |
                                               pugi::parse_doctype);

  auto manifest = doc.select_node("/package/manifest").node();
  auto first_chapter_id = last_num(doc, "chapter", ".xhtml") + 1;
  auto first_volume_id = last_num(doc, "volume", ".xhtml") + 1;
  do_deal_with_content(manifest, first_chapter_id, first_volume_id, content);

  save_file(doc, Epub::content_path);

  return {first_chapter_id, first_volume_id};
}

void do_deal_with_toc(
    pugi::xml_node &nav_map, std::int32_t first_chapter_id,
    std::int32_t first_volume_id,
    const std::vector<std::tuple<std::string, std::string,
                                 std::vector<std::string>>> &content) {
  std::string last_volume_name;
  for (const auto &[volume_name, title, text] : content) {
    if (std::empty(volume_name)) {
      if (has_children(nav_map.last_child(), "navPoint")) {
        nav_map = nav_map.last_child();
      }

      append_nav_map(nav_map, title,
                     "Text/" + num_to_chapter_name(first_chapter_id++));
      nav_map = nav_map.select_node("/ncx/navMap").node();
    } else {
      if (std::empty(last_volume_name)) {
        generate_volume(volume_name, first_volume_id);
        append_nav_map(nav_map, volume_name,
                       "Text/" + num_to_volume_name(first_volume_id++));
        nav_map = nav_map.last_child();
        append_nav_map(nav_map, title,
                       "Text/" + num_to_chapter_name(first_chapter_id++));
      } else {
        if (volume_name != last_volume_name) {
          generate_volume(volume_name, first_volume_id);
          nav_map = nav_map.parent();
          append_nav_map(nav_map, volume_name,
                         "Text/" + num_to_volume_name(first_volume_id++));
          nav_map = nav_map.last_child();
        }

        append_nav_map(nav_map, title,
                       "Text/" + num_to_chapter_name(first_chapter_id++));
      }
    }

    last_volume_name = volume_name;
  }
}

void deal_with_toc(
    const std::vector<std::tuple<std::string, std::string,
                                 std::vector<std::string>>> &content,
    std::int32_t first_chapter_id, std::int32_t first_volume_id) {
  pugi::xml_document doc;
  doc.load_file(Epub::toc_path.data(), pugi::parse_default |
                                           pugi::parse_declaration |
                                           pugi::parse_doctype);

  auto nav_map = doc.select_node("/ncx/navMap").node();
  do_deal_with_toc(nav_map, first_chapter_id, first_volume_id, content);

  save_file(doc, Epub::toc_path);
}

void deal_with_chapter(
    const std::vector<std::tuple<std::string, std::string,
                                 std::vector<std::string>>> &content,
    std::int32_t first_chapter_id) {
  for (const auto &[volume, title, texts] : content) {
    auto doc = generate_xhtml(title, "", true);
    append_texts(doc, texts);

    auto path = std::filesystem::path(Epub::text_dir) /
                num_to_chapter_name(first_chapter_id++);
    save_file(doc, path.c_str());
  }
}

}  // namespace

Epub::Epub() {
  font_ = std::string_view(font, font_size);
  style_ = std::string_view(style, style_size);
}

void Epub::set_creator(const std::string &creator) { creator_ = creator; }

void Epub::set_book_name(const std::string &book_name) {
  book_name_ = book_name;
}

void Epub::set_author(const std::string &author) { author_ = author; }

void Epub::set_introduction(const std::vector<std::string> &introduction) {
  introduction_ = introduction;
}

void Epub::set_postscript(const std::vector<std::string> &postscript) {
  postscript_ = postscript;
}

void Epub::set_generate_cover(bool generate_cover) {
  generate_cover_ = generate_cover;
}

void Epub::set_generate_postscript(bool generate_postscript) {
  generate_postscript_ = generate_postscript;
}

void Epub::set_illustration_num(std::int32_t illustration_num) {
  illustration_num_ = illustration_num;
}

void Epub::set_image_num(std::int32_t image_num) { image_num_ = image_num; }

void Epub::set_uuid(const std::string &uuid) { uuid_ = "urn:uuid:" + uuid; }

void Epub::set_date(const std::string &date) { date_ = date; }

void Epub::add_content(const std::string &title,
                       const std::vector<std::string> &text) {
  add_content("", title, text);
}

void Epub::add_content(const std::string &volume_name, const std::string &title,
                       const std::vector<std::string> &content) {
  content_.emplace_back(volume_name, title, content);
  font_words_ += volume_name;
  font_words_ += title;
}

void Epub::generate() {
  if (std::empty(uuid_)) {
    uuid_ = "urn:uuid:" + klib::uuid();
  }

  if (std::empty(date_)) {
    date_ = get_date();
  }

  std::filesystem::create_directory(book_name_);
  auto ptr = std::make_unique<klib::ChangeWorkingDir>(book_name_);

  std::filesystem::create_directory(Epub::meta_inf_dir);
  std::filesystem::create_directory(Epub::oebps_dir);
  std::filesystem::create_directory(Epub::fonts_dir);
  if (image_num_ > 0 || generate_cover_) {
    std::filesystem::create_directory(Epub::images_dir);
  }
  std::filesystem::create_directory(Epub::styles_dir);
  std::filesystem::create_directory(Epub::text_dir);

  generate_container();
  generate_font();
  generate_style();
  generate_chapter();
  generate_cover();
  generate_illustration();
  generate_introduction();
  generate_message();
  generate_postscript();
  generate_content();
  generate_toc();
  generate_mimetype();
}

void Epub::append_chapter(
    const std::string &book_name,
    const std::vector<std::tuple<std::string, std::string,
                                 std::vector<std::string>>> &content) {
  if (!std::filesystem::is_directory(book_name)) {
    throw klib::RuntimeError("The dir not exists: {}", book_name);
  }

  klib::ChangeWorkingDir change_working_dir(book_name);

  auto [first_chapter_id, first_volume_id] = deal_with_content(content);
  deal_with_toc(content, first_chapter_id, first_volume_id);
  deal_with_chapter(content, first_chapter_id);

  pugi::xml_document doc;
  doc.load_file(Epub::toc_path.data(), pugi::parse_default |
                                           pugi::parse_declaration |
                                           pugi::parse_doctype);

  auto nav_map = doc.select_node("/ncx/navMap").node();
  for (const auto &may_be_volume : nav_map.children("navPoint")) {
    Expects(has_children(may_be_volume, "navLabel"));
    font_words_ +=
        may_be_volume.child("navLabel").child("text").text().as_string();

    if (has_children(may_be_volume, "navPoint")) {
      for (const auto &chapter : may_be_volume.children("navPoint")) {
        Expects(has_children(chapter, "navLabel"));
        font_words_ +=
            chapter.child("navLabel").child("text").text().as_string();
      }
    }
  }
  dbg(font_words_);

  remove_file_or_dir(font_path.data());
  generate_font();
}

void Epub::generate_container() const {
  auto doc = generate_declaration();

  auto container = doc.append_child("container");
  container.append_attribute("version") = "1.0";
  container.append_attribute("xmlns") =
      "urn:oasis:names:tc:opendocument:xmlns:container";

  auto rootfile = container.append_child("rootfiles").append_child("rootfile");
  rootfile.append_attribute("full-path") = content_path.data();
  rootfile.append_attribute("media-type") = "application/oebps-package+xml";

  save_file(doc, container_path);
}

void Epub::generate_font() const {
  spdlog::info("Start generating woff2 font");

  klib::write_file(Epub::temp_font_path, true, font_);
  klib::exec(fmt::format(
      FMT_COMPILE(
          R"(pyftsubset --flavor=woff2 --output-file={} --text="{}" {})"),
      Epub::font_path, font_words_, Epub::temp_font_path));
}

void Epub::generate_style() const {
  if (std::empty(style_)) {
    throw klib::RuntimeError("The style is empty");
  }

  klib::write_file(Epub::style_path, false, style_);
}

void Epub::generate_chapter() const { deal_with_chapter(content_, 1); }

void Epub::generate_cover() const {
  if (generate_cover_) {
    auto doc = generate_xhtml("封面", "cover", false);

    auto div = doc.select_node("/html/body/div").node();
    auto img = div.append_child("img");
    img.append_attribute("alt") = "";
    img.append_attribute("src") = "../Images/cover.jpg";

    save_file(doc, Epub::cover_path);
  }
}

void Epub::generate_illustration() const {
  for (std::int32_t i = 1; i <= illustration_num_; ++i) {
    auto num_str = num_to_str(i);
    auto doc = generate_xhtml("彩页" + num_str, "", false);
    auto file_name = num_to_illustration_name(i);

    auto div = doc.select_node("/html/body/div").node();
    div = div.append_child("div");
    div.append_attribute("class") = "center";

    auto img = div.append_child("img");
    img.append_attribute("alt") = num_str.c_str();
    img.append_attribute("src") = ("../Images/" + num_str + ".jpg").c_str();

    auto path = std::filesystem::path(Epub::text_dir) / file_name;
    save_file(doc, path.c_str());
  }
}

void Epub::generate_introduction() const {
  auto doc = generate_xhtml("简介", "", true);
  append_texts(doc, introduction_);
  save_file(doc, Epub::introduction_path);
}

void Epub::generate_message() const {
  auto doc = generate_xhtml("制作信息", "", true);

  auto div = doc.select_node("/html/body/div").node();
  div = div.append_child("div");
  div.append_attribute("class") = "cutline";

  auto p = div.append_child("p");
  p.append_attribute("class") = "makerifm";
  p.text() = ("制作者：" + creator_).c_str();

  save_file(doc, Epub::message_path);
}

void Epub::generate_postscript() const {
  if (generate_postscript_) {
    auto doc = generate_xhtml("后记", "", true);
    append_texts(doc, postscript_);
    save_file(doc, Epub::postscript_path);
  }
}

void Epub::generate_content() const {
  auto doc = generate_declaration();

  auto package = doc.append_child("package");
  package.append_attribute("version") = "2.0";
  package.append_attribute("unique-identifier") = "BookId";
  package.append_attribute("xmlns") = "http://www.idpf.org/2007/opf";

  auto metadata = package.append_child("metadata");
  metadata.append_attribute("xmlns:dc") = "http://purl.org/dc/elements/1.1/";
  metadata.append_attribute("xmlns:opf") = "http://www.idpf.org/2007/opf";

  metadata.append_child("dc:title").text() = book_name_.c_str();

  auto dc_creator = metadata.append_child("dc:creator");
  dc_creator.append_attribute("opf:file-as") = creator_.c_str();
  dc_creator.append_attribute("opf:role") = "aut";
  dc_creator.text() = author_.c_str();

  metadata.append_child("dc:language").text() = "zh-CN";
  metadata.append_child("dc:rights").text() = creator_.c_str();

  auto dc_date = metadata.append_child("dc:date");
  dc_date.append_attribute("opf:event") = "modification";
  dc_date.append_attribute("xmlns:opf") = "http://www.idpf.org/2007/opf";
  dc_date.text() = date_.c_str();

  auto dc_identifier = metadata.append_child("dc:identifier");
  dc_identifier.append_attribute("id") = "BookId";
  dc_identifier.append_attribute("opf:scheme") = "UUID";
  dc_identifier.text() = uuid_.c_str();

  if (generate_cover_) {
    auto meta = metadata.append_child("meta");
    meta.append_attribute("name") = "cover";
    meta.append_attribute("content") = "cover.jpg";
  }

  auto manifest = package.append_child("manifest");
  append_manifest_and_spine(manifest, "ncx", "toc.ncx");
  append_manifest_and_spine(manifest, "style.css", "Styles/style.css");
  append_manifest_and_spine(manifest, "SourceHanSansSC-Bold.woff2",
                            "Fonts/SourceHanSansSC-Bold.woff2");

  for (std::int32_t i = 1; i <= image_num_; ++i) {
    append_manifest_and_spine(manifest, "x" + num_to_str(i) + ".jpg",
                              "Images/" + num_to_str(i) + ".jpg");
  }

  if (generate_cover_) {
    append_manifest_and_spine(manifest, "cover.jpg", "Images/cover.jpg");
    append_manifest_and_spine(manifest, "cover.xhtml", "Text/cover.xhtml");
  }

  append_manifest_and_spine(manifest, "message.xhtml", "Text/message.xhtml");
  append_manifest_and_spine(manifest, "introduction.xhtml",
                            "Text/introduction.xhtml");

  for (std::int32_t i = 1; i <= illustration_num_; ++i) {
    auto name = num_to_illustration_name(i);
    append_manifest_and_spine(manifest, name, "Text/" + name);
  }

  do_deal_with_content(manifest, 1, 1, content_);

  if (generate_postscript_) {
    append_manifest_and_spine(manifest, "postscript.xhtml",
                              "Text/postscript.xhtml");
  }

  auto guide = package.append_child("guide");
  if (generate_cover_) {
    auto reference = guide.append_child("reference");
    reference.append_attribute("type") = "cover";
    reference.append_attribute("title") = "Cover";
    reference.append_attribute("href") = "Text/cover.xhtml";
  }

  save_file(doc, Epub::content_path);
}

void Epub::generate_toc() {
  auto doc = generate_declaration();

  doc.append_child(pugi::node_doctype)
      .set_value(
          R"(ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN" "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd")");

  auto ncx = doc.append_child("ncx");
  ncx.append_attribute("version") = "2005-1";
  ncx.append_attribute("xmlns") = "http://www.daisy.org/z3986/2005/ncx/";

  auto head = ncx.append_child("head");

  auto meta = head.append_child("meta");
  meta.append_attribute("name") = "dtb:uid";
  meta.append_attribute("content") = uuid_.c_str();

  meta = head.append_child("meta");
  meta.append_attribute("name") = "dtb:depth";
  meta.append_attribute("content") = 1;

  meta = head.append_child("meta");
  meta.append_attribute("name") = "dtb:totalPageCount";
  meta.append_attribute("content") = 0;

  meta = head.append_child("meta");
  meta.append_attribute("name") = "dtb:maxPageNumber";
  meta.append_attribute("content") = 0;

  auto doc_title = ncx.append_child("docTitle");
  doc_title.append_child("text").text() = book_name_.c_str();

  auto doc_author = ncx.append_child("docAuthor");
  doc_author.append_child("text").text() = author_.c_str();

  auto nav_map = ncx.append_child("navMap");
  if (generate_cover_) {
    append_nav_map(nav_map, "封面", "Text/cover.xhtml");
  }
  append_nav_map(nav_map, "制作信息", "Text/message.xhtml");
  append_nav_map(nav_map, "简介", "Text/introduction.xhtml");

  if (illustration_num_ > 0) {
    append_nav_map(nav_map, "彩页", "Text/illustration001.xhtml");
  }

  do_deal_with_toc(nav_map, 1, 1, content_);
  nav_map = doc.select_node("/ncx/navMap").node();

  if (generate_postscript_) {
    append_nav_map(nav_map, "后记", "Text/postscript.xhtml");
  }

  save_file(doc, Epub::toc_path);
}

void Epub::generate_mimetype() const {
  std::string text = "application/epub+zip\n";
  klib::write_file(Epub::mimetype_path, false, text);
}

}  // namespace kepub
