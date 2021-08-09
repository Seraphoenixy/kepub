#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "parse_xml.h"

namespace kepub {

class Content {
 public:
  explicit Content(const std::string &title,
                   const std::vector<std::string> &lines = {});

  void push_line(const std::string &line, bool connect_chinese);
  void push_lines(const std::vector<std::string> &lines, bool connect_chinese);

  [[nodiscard]] const std::string &get_title() const;
  [[nodiscard]] const std::vector<std::string> &get_lines() const;

 private:
  std::string title_;
  std::vector<std::string> lines_;
};

class Epub {
 public:
  Epub() = default;

  void set_generate_cover(bool generate_cover);
  void set_generate_postscript(bool generate_postscript);
  void set_old_style(bool old_style);
  void set_illustration_num(int32_t illustration_num);
  void set_image_num(int32_t image_num);

  void set_creator(const std::string &creator);
  void set_book_name(const std::string &book_name);
  void set_author(const std::string &author);
  void set_description(const Content &content);
  void set_cover_url(const std::string &url);

  void add_content(const Content &content);

  void generate();
  void generate_for_web(
      const std::vector<std::string> &titles,
      const std::vector<std::string> &urls,
      const std::function<std::vector<std::string>(const std::string &get_text)>
          &get_text);

  [[nodiscard]] static std::string generate_chapter(const Content &content);

  static void add_file_in_content_opf(XHTML &content_opf, const std::string &id,
                                      const std::string &href,
                                      const std::string &media_type);
  static void add_nav_point(XHTML &toc_ncx, const std::string &title,
                            const std::string &src);

 private:
  void common_generate();

  void generate_image();
  void generate_cover();
  void generate_message();
  void generate_introduction();
  void generate_illustration();
  void generate_chapter();
  void generate_postscript();

  bool generate_cover_ = false;
  bool generate_postscript_ = false;
  bool old_style_ = false;
  std::int32_t illustration_num_ = 0;
  std::int32_t image_num_ = 0;

  std::string creator_ = "TODO";
  std::string book_name_ = "TODO";
  std::string author_ = "TODO";
  std::vector<std::string> description_ = {"TODO"};
  std::string cover_url_;

  std::vector<Content> contents_;

  std::filesystem::path root_;

  XHTML content_opf_;
  XHTML toc_ncx_;
};

}  // namespace kepub
