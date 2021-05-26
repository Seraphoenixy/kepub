#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace kepub {

class Content {
 public:
  explicit Content(const std::string &title,
                   const std::vector<std::string> &lines = {});

  void push_vec(const std::vector<std::string> &lines);
  void push_line(const std::string &line);

  [[nodiscard]] const std::string &get_title() const;
  [[nodiscard]] const std::vector<std::string> &get_lines() const;

 private:
  std::string title_;
  std::vector<std::string> lines_;
};

class Epub {
 public:
  Epub() = default;

  void set_creator(const std::string &creator);

  void set_book_name(const std::string &book_name);
  void set_author(const std::string &author);
  void set_description(const std::vector<std::string> &description);

  void add_content(const std::string &title,
                   const std::vector<std::string> &text);
  void add_content(const Content &content);

  void generate() const;

  void generate_for_web(
      const std::vector<std::string> &titles,
      const std::vector<std::string> &urls,
      std::function<std::vector<std::string>(const std::string &get_text)>
          get_text) const;

 private:
  void do_generate() const;

  [[nodiscard]] std::string generate_content_opf(std::size_t size) const;
  [[nodiscard]] std::string generate_toc_ncx(
      const std::vector<std::string> &titles) const;
  [[nodiscard]] std::string generate_introduction() const;
  [[nodiscard]] std::string generate_chapter(const Content &content) const;

  std::string creator_ = "TODO";

  std::string book_name_ = "TODO";
  std::string author_ = "TODO";
  std::vector<std::string> description_ = {"TODO"};

  std::vector<Content> contents_;
};

}  // namespace kepub
