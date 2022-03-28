#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

#include <pugixml.hpp>

#include "kepub_export.h"
#include "novel.h"

namespace kepub {

class KEPUB_EXPORT Epub {
 public:
  Epub();

  void set_rights(const std::string &rights) { rights_ = rights; }

  void set_uuid(const std::string &uuid) {
    uuid_ = uuid;
    debug_ = true;
  }
  void set_datetime(const std::string &date) {
    datetime_ = date;
    debug_ = true;
  }

  void set_novel(const Novel &novel);

  void flush_font(const std::string &book_dir);
  void append();

  void generate();

  constexpr static std::string_view meta_inf_dir = "META-INF";
  constexpr static std::string_view epub_dir = "EPUB";
  constexpr static std::string_view style_dir = "EPUB/css";
  constexpr static std::string_view font_dir = "EPUB/font";
  constexpr static std::string_view image_dir = "EPUB/image";
  constexpr static std::string_view text_dir = "EPUB/text";

  constexpr static std::string_view container_xml_path =
      "META-INF/container.xml";
  constexpr static std::string_view style_css_path = "EPUB/css/style.css";
  constexpr static std::string_view font_woff2_path =
      "EPUB/font/SourceHanSansSC-Bold.woff2";
  constexpr static std::string_view cover_xhtml_path = "EPUB/text/cover.xhtml";
  constexpr static std::string_view introduction_xhtml_path =
      "EPUB/text/introduction.xhtml";
  constexpr static std::string_view postscript_xhtml_path =
      "EPUB/text/postscript.xhtml";
  constexpr static std::string_view nav_xhtml_path = "EPUB/nav.xhtml";
  constexpr static std::string_view package_opf_path = "EPUB/package.opf";
  constexpr static std::string_view mimetype_path = "mimetype";

 private:
  static void generate_container();
  void generate_style() const;
  void generate_image() const;
  void generate_volume() const;
  void generate_chapter() const;
  void generate_cover() const;
  void generate_illustration() const;
  void generate_introduction() const;
  void generate_postscript() const;
  void generate_nav() const;
  void generate_package() const;
  static void generate_mimetype();
  void generate_font();

  void do_generate_image(const std::filesystem::path &path) const;
  void do_deal_with_nav(pugi::xml_node &ol, std::int32_t first_volume_id,
                        std::int32_t first_chapter_id) const;
  void deal_with_nav(std::int32_t first_volume_id,
                     std::int32_t first_chapter_id) const;
  void do_deal_with_package(pugi::xml_node &manifest,
                            std::int32_t first_chapter_id,
                            std::int32_t first_volume_id) const;
  [[nodiscard]] std::pair<std::int32_t, std::int32_t> deal_with_package() const;
  void deal_with_volume(std::int32_t first_volume_id) const;
  void deal_with_chapter(std::int32_t first_chapter_id) const;

  bool ready_ = false;

  std::string rights_;

  std::string uuid_;
  std::string datetime_;

  Novel novel_;

  std::string_view style_;
  std::string_view font_;

  mutable std::string font_words_;
  bool debug_ = false;
};

}  // namespace kepub
