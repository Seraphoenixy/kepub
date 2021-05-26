#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <pugixml.hpp>

namespace kepub {

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
  explicit XHTML(const std::string &xhtml);

  void move_by_name(std::string_view name);
  void move_by_attr(std::string_view name, std::string_view attr_name,
                    std::string_view attr_value);

  void move_by_attr_class(std::string_view name, std::string_view attr_value);

  void save(std::string_view file) const;
  std::string to_string() const;

  [[nodiscard]] std::string get_text() const;
  void previous();
  [[nodiscard]] std::vector<pugi::xml_node> children() const;

  void push_back(const Node &node);

  void insert_before(const Node &node, std::string_view attr_name,
                     std::string_view attr_value);

  void insert_after(const Node &node, std::string_view attr_name,
                    std::string_view attr_value);

  static void get_text(pugi::xml_node node, std::string &str);

 private:
  void insert(pugi::xml_node child, const Node &node);

  pugi::xml_document root_;
  pugi::xml_node node_;
};

std::string html_tidy(const std::string &html);

}  // namespace kepub
