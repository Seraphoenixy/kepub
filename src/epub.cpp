#include "epub.h"

#include <ctime>
#include <memory>

#include <dbg.h>
#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/archive.h>
#include <klib/font.h>
#include <klib/image.h>
#include <klib/log.h>
#include <klib/unicode.h>
#include <klib/util.h>
#include <boost/algorithm/string.hpp>
#include <boost/sort/pdqsort/pdqsort.hpp>
#include <gsl/assert>
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

std::string get_datetime() {
  return fmt::format(FMT_COMPILE("{:%Y-%m-%dT%H:%M:%SZ}"),
                     fmt::localtime(std::time(nullptr)));
}

// https://www.w3.org/publishing/epub3/epub-spec.html#sec-cmt-supported
std::string media_type(const std::string &file_name) {
  std::string result;

  if (file_name.ends_with(".gif")) {
    result = "image/gif";
  } else if (file_name.ends_with(".jpg") || file_name.ends_with(".jpeg")) {
    result = "image/jpeg";
  } else if (file_name.ends_with(".png")) {
    result = "image/png";
  } else if (file_name.ends_with(".webp")) {
    result = "image/webp";
  } else if (file_name.ends_with(".css")) {
    result = "text/css";
  } else if (file_name.ends_with(".woff2")) {
    result = "font/woff2";
  } else if (file_name.ends_with(".xhtml")) {
    result = "application/xhtml+xml";
  } else {
    klib::error("Unknown media type: {}", file_name);
  }

  return result;
}

pugi::xml_document generate_declaration() {
  pugi::xml_document doc;
  auto decl = doc.prepend_child(pugi::node_declaration);
  decl.append_attribute("version") = "1.0";
  decl.append_attribute("encoding") = "UTF-8";

  return doc;
}

pugi::xml_document generate_xhtml_template(const std::string &title,
                                           const std::string &div_class = "",
                                           bool has_h1 = false) {
  auto doc = generate_declaration();

  auto html = doc.append_child("html");
  html.append_attribute("xmlns") = "http://www.w3.org/1999/xhtml";
  html.append_attribute("xmlns:epub") = "http://www.idpf.org/2007/ops";
  html.append_attribute("xml:lang") = "zh-CN";

  auto head = html.append_child("head");
  head.append_child("title").text() = title.c_str();

  auto link = head.append_child("link");
  link.append_attribute("rel") = "stylesheet";
  link.append_attribute("href") = "../css/style.css";

  auto body = html.append_child("body");
  auto div = body.append_child("div");

  if (!std::empty(div_class)) {
    div.append_attribute("class") = div_class.c_str();
  }

  if (has_h1) {
    auto h1 = div.append_child("h1");
    h1.text() = title.c_str();
  }

  return doc;
}

void save_file(const pugi::xml_document &doc, std::string_view path) {
  const char *space_2 = "  ";
  if (!doc.save_file(std::data(path), space_2)) {
    klib::error("Can not save: {}", path);
  }
}

void append_texts(pugi::xml_document &doc,
                  const std::vector<std::string> &texts) {
  auto div = doc.select_node("/html/body/div").node();
  Ensures(!div.empty());

  const static std::string image_prefix = "[IMAGE] ";
  const static auto image_prefix_size = std::size(image_prefix);
  for (const auto &text : texts) {
    if (text.starts_with(image_prefix)) [[unlikely]] {
      auto image_name = text.substr(image_prefix_size);

      auto d = div.append_child("div");
      d.append_attribute("class") = "center";
      auto img = d.append_child("img");

      auto stem = std::filesystem::path(image_name).stem().string();
      img.append_attribute("alt") = stem.c_str();
      img.append_attribute("src") = ("../image/" + stem + ".webp").c_str();
    } else {
      auto p = div.append_child("p");
      p.text() = text.c_str();
    }
  }
}

void append_manifest(pugi::xml_node &manifest, const std::string &id,
                     const std::string &href,
                     const std::string &properties = "") {
  auto item = manifest.append_child("item");
  item.append_attribute("id") = id.c_str();
  item.append_attribute("href") = href.c_str();
  item.append_attribute("media-type") = media_type(href).c_str();
  if (!std::empty(properties)) {
    item.append_attribute("properties") = properties.c_str();
  }
}

void append_manifest_and_spine(pugi::xml_node &manifest, const std::string &id,
                               const std::string &href,
                               const std::string &properties = "") {
  append_manifest(manifest, id, href, properties);

  if (href.ends_with("xhtml")) {
    auto package = manifest.parent();
    auto spine = package.child("spine");
    if (spine.empty()) {
      spine = package.append_child("spine");
    }

    auto itemref = spine.append_child("itemref");
    itemref.append_attribute("idref") = id.c_str();
  }
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
  boost::sort::pdqsort(std::begin(ids), std::end(ids));

  return std::empty(ids) ? 0 : ids.back();
}

void compress_image(const std::string &path) {
  auto image_file = klib::read_file(path, true);
  remove_file_or_dir(path);

  auto image_dir = std::filesystem::path(path).parent_path();
  auto new_file_name = std::filesystem::path(path).stem().string() + ".webp";

  klib::write_file(image_dir / new_file_name, true,
                   klib::image_to_webp(image_file));
}

}  // namespace

Epub::Epub() {
  font_ = std::string_view(font, font_size);
  style_ = std::string_view(style, style_size);
}

void Epub::set_novel(const Novel &novel) {
  novel_ = novel;
  ready_ = true;

  novel_.book_info_.name_ = make_book_name_legal(novel_.book_info_.name_);

  if (!std::empty(novel_.book_info_.cover_path_)) {
    novel_.book_info_.cover_path_ =
        std::filesystem::current_path() / novel_.book_info_.cover_path_;
  }
  novel_.book_info_.cover_file_name_ =
      std::filesystem::path(novel_.book_info_.cover_path_).stem().string() +
      ".webp";

  for (auto &path : novel_.image_paths_) {
    path = std::filesystem::current_path() / path;
  }
}

void Epub::flush_font(const std::string &book_dir) {
  klib::ChangeWorkingDir dir(book_dir);

  if (!std::filesystem::exists(Epub::font_woff2_path)) {
    klib::warn(
        "The epub was not generated by a compatible version of kepub, the "
        "font cannot be refreshed");
    return;
  }

  pugi::xml_document doc;
  doc.load_file(std::data(Epub::nav_xhtml_path),
                pugi::parse_default | pugi::parse_declaration);

  auto ol = doc.select_node("/html/body/nav/ol").node();
  Ensures(!ol.empty());

  for (const auto &li : ol.children("li")) {
    auto a = li.child("a");
    Ensures(!a.empty());
    font_words_.append(a.text().as_string());

    for (const auto &child_li : li.child("ol")) {
      auto child_a = child_li.child("a");
      Ensures(!child_a.empty());
      font_words_.append(child_a.text().as_string());
    }
  }

  generate_font();
}

void Epub::append() {
  if (!ready_) {
    klib::error("Call set_novel() first");
  }

  auto dir = std::make_unique<klib::ChangeWorkingDir>(novel_.book_info_.name_);

  auto [first_volume_id, first_chapter_id] = deal_with_package();
  deal_with_nav(first_volume_id, first_chapter_id);
  deal_with_volume(first_volume_id);
  deal_with_chapter(first_chapter_id);

  dir.reset();

  flush_font(novel_.book_info_.name_);

  klib::compress(novel_.book_info_.name_, klib::Format::Zip,
                 klib::Filter::Deflate, novel_.book_info_.name_ + ".epub",
                 false);

  ready_ = false;
}

void Epub::generate() {
  if (!ready_) {
    klib::error("Call set_novel() first");
  }

  if (std::empty(novel_.book_info_.name_)) {
    klib::error("book name is empty");
  }

  if (std::empty(uuid_)) {
    uuid_ = boost::to_upper_copy(klib::uuid());
  }
  if (std::empty(datetime_)) {
    datetime_ = get_datetime();
  }

  if (std::filesystem::exists(novel_.book_info_.name_)) {
    remove_file_or_dir(novel_.book_info_.name_);
  }

  std::filesystem::create_directory(novel_.book_info_.name_);
  auto dir = std::make_unique<klib::ChangeWorkingDir>(novel_.book_info_.name_);

  std::filesystem::create_directory(Epub::meta_inf_dir);
  std::filesystem::create_directory(Epub::epub_dir);
  std::filesystem::create_directory(Epub::style_dir);
  std::filesystem::create_directory(Epub::font_dir);
  if (!std::empty(novel_.book_info_.cover_path_) ||
      !std::empty(novel_.image_paths_)) {
    std::filesystem::create_directory(Epub::image_dir);
  }
  std::filesystem::create_directory(Epub::text_dir);

  generate_container();
  generate_style();
  generate_image();
  generate_volume();
  generate_chapter();
  generate_cover();
  generate_illustration();
  generate_introduction();
  generate_postscript();
  generate_nav();
  generate_package();
  generate_mimetype();
  generate_font();

  dir.reset();
  klib::compress(novel_.book_info_.name_, klib::Format::Zip,
                 klib::Filter::Deflate, novel_.book_info_.name_ + ".epub",
                 false);

  ready_ = false;
}

void Epub::generate_container() {
  auto doc = generate_declaration();

  auto container = doc.append_child("container");
  container.append_attribute("version") = "1.0";
  container.append_attribute("xmlns") =
      "urn:oasis:names:tc:opendocument:xmlns:container";

  auto rootfile = container.append_child("rootfiles").append_child("rootfile");
  rootfile.append_attribute("full-path") = std::data(Epub::package_opf_path);
  rootfile.append_attribute("media-type") = "application/oebps-package+xml";

  save_file(doc, Epub::container_xml_path);
}

void Epub::generate_style() const {
  Expects(!std::empty(style_));
  klib::write_file(Epub::style_css_path, false, style_);
}

void Epub::generate_image() const {
  for (const auto &item : novel_.image_paths_) {
    do_generate_image(item);
  }

  if (!std::empty(novel_.book_info_.cover_path_)) {
    do_generate_image(novel_.book_info_.cover_path_);
  }
}

void Epub::generate_volume() const { deal_with_volume(1); }

void Epub::generate_chapter() const { deal_with_chapter(1); }

void Epub::generate_cover() const {
  if (!std::empty(novel_.book_info_.cover_path_)) {
    font_words_.append("封面");

    auto doc = generate_xhtml_template("封面", "cover", false);

    auto body = doc.select_node("/html/body").node();
    Ensures(!body.empty());
    body.append_attribute("epub:type") = "coverpage";

    auto div = doc.select_node("/html/body/div").node();
    Ensures(!div.empty());

    auto img = div.append_child("img");
    img.append_attribute("alt") = "";
    img.append_attribute("src") =
        ("../image/" + novel_.book_info_.cover_file_name_).c_str();

    save_file(doc, Epub::cover_xhtml_path);
  }
}

void Epub::generate_illustration() const {
  for (std::int32_t i = 1; i <= novel_.illustration_num_; ++i) {
    auto title = "彩页 " + std::to_string(i);
    font_words_.append(title);

    auto doc = generate_xhtml_template(title, "center", false);
    auto file_name = num_to_illustration_name(i);

    auto div = doc.select_node("/html/body/div").node();
    Ensures(!div.empty());

    auto img = div.append_child("img");
    auto num_str = num_to_str(i);
    img.append_attribute("alt") = num_str.c_str();
    img.append_attribute("src") = ("../image/" + num_str + ".webp").c_str();

    auto path = std::filesystem::path(Epub::text_dir) / file_name;
    save_file(doc, path.c_str());
  }
}

void Epub::generate_introduction() const {
  if (!std::empty(novel_.book_info_.introduction_)) {
    font_words_.append("简介");

    auto doc = generate_xhtml_template("简介", "", true);

    auto body = doc.select_node("/html/body").node();
    Ensures(!body.empty());
    body.append_attribute("epub:type") = "introduction";

    append_texts(doc, novel_.book_info_.introduction_);
    save_file(doc, Epub::introduction_xhtml_path);
  }
}

void Epub::generate_postscript() const {
  if (!std::empty(novel_.postscript_)) {
    font_words_.append("后记");

    auto doc = generate_xhtml_template("后记", "", true);

    auto body = doc.select_node("/html/body").node();
    Ensures(!body.empty());
    body.append_attribute("epub:type") = "afterword";

    append_texts(doc, novel_.postscript_);
    save_file(doc, Epub::postscript_xhtml_path);
  }
}

void Epub::generate_nav() const {
  auto doc = generate_declaration();

  auto html = doc.append_child("html");
  html.append_attribute("xmlns") = "http://www.w3.org/1999/xhtml";
  html.append_attribute("xmlns:epub") = "http://www.idpf.org/2007/ops";
  html.append_attribute("xml:lang") = "zh-CN";

  auto head = html.append_child("head");
  head.append_child("title").text() = "目录";

  auto link = head.append_child("link");
  link.append_attribute("rel") = "stylesheet";
  link.append_attribute("href") = "css/style.css";

  auto body = html.append_child("body");
  auto nav = body.append_child("nav");
  nav.append_attribute("epub:type") = "toc";

  auto ol = nav.append_child("ol");

  if (!std::empty(novel_.book_info_.cover_path_)) {
    auto li = ol.append_child("li");
    auto a = li.append_child("a");
    a.append_attribute("href") = "text/cover.xhtml";
    a.text() = "封面";
  }

  if (!std::empty(novel_.book_info_.introduction_)) {
    auto li = ol.append_child("li");
    auto a = li.append_child("a");
    a.append_attribute("href") = "text/introduction.xhtml";
    a.text() = "简介";
  }

  for (std::int32_t i = 1; i <= novel_.illustration_num_; ++i) {
    auto li = ol.append_child("li");
    auto a = li.append_child("a");
    a.append_attribute("href") =
        ("text/" + num_to_illustration_name(i)).c_str();
    a.text() = ("彩页 " + std::to_string(i)).c_str();
  }

  do_deal_with_nav(ol, 1, 1);

  if (!std::empty(novel_.postscript_)) {
    auto li = ol.append_child("li");
    auto a = li.append_child("a");
    a.append_attribute("href") = "text/postscript.xhtml";
    a.text() = "后记";
  }

  save_file(doc, Epub::nav_xhtml_path);
}

void Epub::generate_package() const {
  auto doc = generate_declaration();

  auto package = doc.append_child("package");
  package.append_attribute("xmlns") = "http://www.idpf.org/2007/opf";
  package.append_attribute("version") = "3.0";
  package.append_attribute("xml:lang") = "zh-CN";
  package.append_attribute("unique-identifier") = "pub-id";

  auto metadata = package.append_child("metadata");
  metadata.append_attribute("xmlns:dc") = "http://purl.org/dc/elements/1.1/";

  auto dc_identifier = metadata.append_child("dc:identifier");
  dc_identifier.append_attribute("id") = "pub-id";
  dc_identifier.text() = ("urn:uuid:" + uuid_).c_str();

  auto meta = metadata.append_child("meta");
  meta.append_attribute("refines") = "#pub-id";
  meta.append_attribute("property") = "identifier-type";
  meta.append_attribute("scheme") = "xsd:string";
  meta.text() = "uuid";

  auto dc_title = metadata.append_child("dc:title");
  dc_title.append_attribute("id") = "title";
  dc_title.text() = novel_.book_info_.name_.c_str();

  meta = metadata.append_child("meta");
  meta.append_attribute("refines") = "#title";
  meta.append_attribute("property") = "title-type";
  meta.text() = "main";

  meta = metadata.append_child("meta");
  meta.append_attribute("refines") = "#title";
  meta.append_attribute("property") = "file-as";
  meta.text() = novel_.book_info_.name_.c_str();

  if (!std::empty(novel_.book_info_.author_)) {
    auto dc_creator = metadata.append_child("dc:creator");
    dc_creator.append_attribute("id") = "creator";
    dc_creator.text() = novel_.book_info_.author_.c_str();

    meta = metadata.append_child("meta");
    meta.append_attribute("refines") = "#creator";
    meta.append_attribute("property") = "role";
    meta.append_attribute("scheme") = "marc:relators";
    meta.text() = "aut";

    meta = metadata.append_child("meta");
    meta.append_attribute("refines") = "#creator";
    meta.append_attribute("property") = "file-as";
    meta.text() = novel_.book_info_.author_.c_str();
  }

  metadata.append_child("dc:language").text() = "zh-CN";

  if (!std::empty(rights_)) {
    metadata.append_child("dc:rights").text() = rights_.c_str();
  }

  if (!std::empty(novel_.book_info_.introduction_)) {
    metadata.append_child("dc:description").text() =
        boost::join(novel_.book_info_.introduction_, "\n").c_str();
  }

  meta = metadata.append_child("meta");
  meta.append_attribute("property") = "dcterms:modified";
  meta.text() = datetime_.c_str();

  if (!std::empty(novel_.book_info_.cover_path_)) {
    meta = metadata.append_child("meta");
    meta.append_attribute("name") = "cover";
    meta.append_attribute("content") =
        novel_.book_info_.cover_file_name_.c_str();
  }

  auto manifest = package.append_child("manifest");
  append_manifest_and_spine(manifest, "style.css", "css/style.css");
  append_manifest_and_spine(manifest, "SourceHanSansSC-Bold.woff2",
                            "font/SourceHanSansSC-Bold.woff2");

  auto image_size = std::size(novel_.image_paths_);
  for (std::size_t i = 1; i <= image_size; ++i) {
    append_manifest_and_spine(manifest, "x" + num_to_str(i) + ".webp",
                              "image/" + num_to_str(i) + ".webp");
  }

  if (!std::empty(novel_.book_info_.cover_path_)) {
    append_manifest_and_spine(manifest, novel_.book_info_.cover_file_name_,
                              "image/" + novel_.book_info_.cover_file_name_,
                              "cover-image");
    append_manifest_and_spine(manifest, "cover.xhtml", "text/cover.xhtml");
  }

  append_manifest(manifest, "nav.xhtml", "nav.xhtml", "nav");
  append_manifest_and_spine(manifest, "introduction.xhtml",
                            "text/introduction.xhtml");

  for (std::int32_t i = 1; i <= novel_.illustration_num_; ++i) {
    auto name = num_to_illustration_name(i);
    append_manifest_and_spine(manifest, name, "text/" + name);
  }

  do_deal_with_package(manifest, 1, 1);

  if (!std::empty(novel_.postscript_)) {
    append_manifest_and_spine(manifest, "postscript.xhtml",
                              "text/postscript.xhtml");
  }

  if (!std::empty(novel_.book_info_.cover_path_)) {
    auto guide = package.append_child("guide");
    auto reference = guide.append_child("reference");
    reference.append_attribute("type") = "cover";
    reference.append_attribute("title") = "封面";
    reference.append_attribute("href") = "text/cover.xhtml";
  }

  save_file(doc, Epub::package_opf_path);
}

void Epub::generate_mimetype() {
  std::string_view text = "application/epub+zip";
  klib::write_file(Epub::mimetype_path, false, text);
}

void Epub::generate_font() {
  Expects(!std::empty(font_));

  dbg(font_words_);
  auto ttf_font = klib::ttf_subset(font_, klib::utf8_to_utf32(font_words_));
  auto woff2_font = klib::ttf_to_woff2(ttf_font);
  klib::write_file(Epub::font_woff2_path, true, woff2_font);
}

void Epub::do_generate_image(const std::filesystem::path &path) const {
  const static std::filesystem::path image_dir(Epub::image_dir);

  Expects(std::filesystem::exists(path));

  auto file_name = path.filename();
  auto image_path = image_dir / file_name;
  std::filesystem::copy(path, image_path);

  compress_image(image_path);
}

void Epub::do_deal_with_nav(pugi::xml_node &ol, std::int32_t first_volume_id,
                            std::int32_t first_chapter_id) const {
  for (const auto &volume : novel_.volumes_) {
    pugi::xml_node node;

    if (!std::empty(volume.title_)) {
      font_words_.append(volume.title_);

      auto li = ol.append_child("li");
      auto a = li.append_child("a");
      a.append_attribute("href") =
          ("text/" + num_to_volume_name(first_volume_id++)).c_str();
      a.text() = volume.title_.c_str();
      node = li.append_child("ol");
    } else {
      auto last_li = ol.last_child();
      if (auto child = last_li.child("ol"); !child.empty()) {
        node = child;
      } else {
        node = ol;
      }
    }

    for (const auto &chapter : volume.chapters_) {
      font_words_.append(chapter.title_);

      auto chapter_li = node.append_child("li");
      auto chapter_a = chapter_li.append_child("a");
      chapter_a.append_attribute("href") =
          ("text/" + num_to_chapter_name(first_chapter_id++)).c_str();
      chapter_a.text() = chapter.title_.c_str();
    }
  }
}

void Epub::deal_with_nav(std::int32_t first_volume_id,
                         std::int32_t first_chapter_id) const {
  pugi::xml_document doc;
  doc.load_file(std::data(Epub::nav_xhtml_path),
                pugi::parse_default | pugi::parse_declaration);

  auto ol = doc.select_node("/html/body/nav/ol").node();
  Ensures(!ol.empty());

  do_deal_with_nav(ol, first_volume_id, first_chapter_id);

  save_file(doc, Epub::nav_xhtml_path);
}

void Epub::do_deal_with_package(pugi::xml_node &manifest,
                                std::int32_t first_volume_id,
                                std::int32_t first_chapter_id) const {
  for (const auto &volume : novel_.volumes_) {
    if (!std::empty(volume.title_)) {
      auto volume_file_name = num_to_volume_name(first_volume_id++);
      append_manifest_and_spine(manifest, volume_file_name,
                                "text/" + volume_file_name);
    }

    for (std::size_t i = 0; i < std::size(volume.chapters_); ++i) {
      auto chapter_file_name = num_to_chapter_name(first_chapter_id++);
      append_manifest_and_spine(manifest, chapter_file_name,
                                "text/" + chapter_file_name);
    }
  }
}

std::pair<std::int32_t, std::int32_t> Epub::deal_with_package() const {
  pugi::xml_document doc;
  doc.load_file(std::data(Epub::package_opf_path),
                pugi::parse_default | pugi::parse_declaration);

  auto manifest = doc.select_node("/package/manifest").node();
  auto first_volume_id = last_num(doc, "volume", ".xhtml") + 1;
  auto first_chapter_id = last_num(doc, "chapter", ".xhtml") + 1;
  do_deal_with_package(manifest, first_volume_id, first_chapter_id);

  if (!debug_) {
    auto datatime =
        doc.select_node("/package/metadata/meta[@property='dcterms:modified']")
            .node();
    Ensures(!datatime.empty());
    datatime.text() = get_datetime().c_str();
  }

  save_file(doc, Epub::package_opf_path);

  return {first_volume_id, first_chapter_id};
}

void Epub::deal_with_volume(std::int32_t first_volume_id) const {
  const static std::filesystem::path text_path(Epub::text_dir);

  for (const auto &volume : novel_.volumes_) {
    if (!std::empty(volume.title_)) {
      auto doc = generate_xhtml_template(volume.title_, "", true);
      auto path = text_path / num_to_volume_name(first_volume_id++);
      save_file(doc, path.c_str());
    }
  }
}

void Epub::deal_with_chapter(std::int32_t first_chapter_id) const {
  const static std::filesystem::path text_path(Epub::text_dir);

  for (const auto &volume : novel_.volumes_) {
    for (const auto &chapter : volume.chapters_) {
      auto doc = generate_xhtml_template(chapter.title_, "", true);
      append_texts(doc, chapter.texts_);

      auto path = text_path / num_to_chapter_name(first_chapter_id++);
      save_file(doc, path.c_str());
    }
  }
}

}  // namespace kepub
