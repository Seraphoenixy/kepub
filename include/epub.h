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

  void set_creator(const std::string& creator);
  void set_book_name(const std::string& book_name);
  void set_author(const std::string& author);
  void set_introduction(const std::vector<std::string>& introduction);
  void set_postscript(const std::vector<std::string>& postscript);

  void set_generate_cover(bool generate_cover);
  void set_generate_postscript(bool generate_postscript);
  void set_illustration_num(std::int32_t illustration_num);
  void set_image_num(std::int32_t image_num);

  void set_uuid(const std::string& uuid);
  void set_date(const std::string& date);

  void add_content(const std::string& title,
                   const std::vector<std::string>& text);
  void add_content(const std::string& volume_name, const std::string& title,
                   const std::vector<std::string>& content);

  void flush_font(const std::string& book_name = "");

  void generate();

  void append_chapter(
      const std::string& book_name,
      const std::vector<std::tuple<std::string, std::string,
                                   std::vector<std::string>>>& content);

  constexpr static const char* meta_inf_dir = "META-INF";
  constexpr static const char* oebps_dir = "OEBPS";
  constexpr static const char* fonts_dir = "OEBPS/Fonts";
  constexpr static const char* images_dir = "OEBPS/Images";
  constexpr static const char* styles_dir = "OEBPS/Styles";
  constexpr static const char* text_dir = "OEBPS/Text";

  constexpr static const char* container_path = "META-INF/container.xml";
  constexpr static const char* font_path =
      "OEBPS/Fonts/SourceHanSansSC-Bold.woff2";
  constexpr static const char* temp_font_path =
      "/tmp/SourceHanSansSC-Bold.woff2";
  constexpr static const char* style_path = "OEBPS/Styles/style.css";
  constexpr static const char* cover_path = "OEBPS/Text/cover.xhtml";
  constexpr static const char* introduction_path =
      "OEBPS/Text/introduction.xhtml";
  constexpr static const char* message_path = "OEBPS/Text/message.xhtml";
  constexpr static const char* postscript_path = "OEBPS/Text/postscript.xhtml";
  constexpr static const char* content_path = "OEBPS/content.opf";
  constexpr static const char* toc_path = "OEBPS/toc.ncx";
  constexpr static const char* mimetype_path = "mimetype";

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
