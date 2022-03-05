#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace kepub {

class Epub {
 public:
  Epub();

  void set_creator(const std::string &creator);
  void set_book_name(const std::string &book_name);
  void set_author(const std::string &author);
  void set_introduction(const std::vector<std::string> &introduction);
  void set_postscript(const std::vector<std::string> &postscript);

  void set_generate_cover(bool generate_cover);
  void set_generate_postscript(bool generate_postscript);
  void set_illustration_num(std::int32_t illustration_num);
  void set_image_num(std::int32_t image_num);

  void set_uuid(const std::string &uuid);
  void set_date(const std::string &date);

  void add_content(const std::string &title,
                   const std::vector<std::string> &text);
  void add_content(const std::string &volume_name, const std::string &title,
                   const std::vector<std::string> &content);

  void flush_font(const std::string &book_name = "");

  void generate();

  void append_chapter(
      const std::string &book_name,
      const std::vector<std::tuple<std::string, std::string,
                                   std::vector<std::string>>> &content);

  constexpr static std::string_view meta_inf_dir = "META-INF";
  constexpr static std::string_view oebps_dir = "OEBPS";
  constexpr static std::string_view fonts_dir = "OEBPS/Fonts";
  constexpr static std::string_view images_dir = "OEBPS/Images";
  constexpr static std::string_view styles_dir = "OEBPS/Styles";
  constexpr static std::string_view text_dir = "OEBPS/Text";

  constexpr static std::string_view container_path = "META-INF/container.xml";
  constexpr static std::string_view font_path =
      "OEBPS/Fonts/SourceHanSansSC-Bold.woff2";
  constexpr static std::string_view temp_font_path =
      "/tmp/SourceHanSansSC-Bold.ttf";
  constexpr static std::string_view style_path = "OEBPS/Styles/style.css";
  constexpr static std::string_view cover_path = "OEBPS/Text/cover.xhtml";
  constexpr static std::string_view introduction_path =
      "OEBPS/Text/introduction.xhtml";
  constexpr static std::string_view message_path = "OEBPS/Text/message.xhtml";
  constexpr static std::string_view postscript_path =
      "OEBPS/Text/postscript.xhtml";
  constexpr static std::string_view content_path = "OEBPS/content.opf";
  constexpr static std::string_view toc_path = "OEBPS/toc.ncx";
  constexpr static std::string_view mimetype_path = "mimetype";

 private:
  void generate_container() const;
  void generate_font() const;
  void generate_style() const;
  void generate_chapter() const;
  void generate_cover() const;
  void generate_illustration() const;
  void generate_introduction() const;
  void generate_message() const;
  void generate_postscript() const;
  void generate_content() const;
  void generate_toc();
  void generate_mimetype() const;

  std::string creator_ = "TODO";
  std::string book_name_ = "TODO";
  std::string author_ = "TODO";
  std::vector<std::string> introduction_ = {"TODO"};
  std::vector<std::string> postscript_ = {"TODO"};

  bool generate_cover_ = false;
  bool generate_postscript_ = false;
  std::int32_t illustration_num_ = 0;
  std::int32_t image_num_ = 0;

  std::string uuid_;
  std::string date_;

  std::string_view style_;
  std::string_view font_;
  std::vector<std::tuple<std::string, std::string, std::vector<std::string>>>
      content_;

  std::string font_words_ = "封面彩页简介制作信息后记0123456789";
};

}  // namespace kepub
