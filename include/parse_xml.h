#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <pugixml.hpp>

namespace kepub {

std::string html_tidy(const std::string &html);

class XHTML;

class Node {
  friend class XHTML;

 public:
  explicit Node(const std::string &name);

  void add_attr(const std::string &attr_name, const std::string &attr_value);
  void set_text(const std::string &text);

  void add_child(const Node &node);

 private:
  std::string name_;
  std::vector<std::pair<std::string, std::string>> attrs_;
  std::string text_;
  std::vector<Node> children_;
};

class XHTML {
 public:
  XHTML() = default;
  explicit XHTML(const std::string &xhtml);

  void save(std::string_view file) const;
  [[nodiscard]] std::string to_string() const;

  void previous();
  void reset();

  void move_by_name(std::string_view name);
  void move_by_attr(std::string_view name, std::string_view attr_name,
                    std::string_view attr_value);
  void move_by_attr_class(std::string_view name, std::string_view attr_value);

  [[nodiscard]] bool has_child() const;
  [[nodiscard]] std::string first_child_attr(std::string_view attr_name) const;
  [[nodiscard]] std::string last_child_attr(std::string_view attr_name) const;

  [[nodiscard]] std::vector<std::string> get_children_attr(
      std::string_view name, std::string_view attr_name) const;
  [[nodiscard]] std::vector<std::string> get_children_text(
      std::string_view name = "") const;

  void set_child_text(std::string_view name, const std::string &text);
  void set_child_attr(std::string_view name, std::string_view attr_name,
                      const std::string &attr_value);

  void push_back(const Node &node);
  void push_back(std::string_view name,
                 const std::vector<std::pair<std::string, std::string>> &attrs,
                 const std::string &text = "");
  void push_back(const std::string &text);

  [[nodiscard]] std::string get_text() const;

 private:
  static void get_text(pugi::xml_node node, std::string &str);
  pugi::xml_node get_child_by_name(std::string_view name);

  pugi::xml_document root_;
  pugi::xml_node node_;
};

}  // namespace kepub
