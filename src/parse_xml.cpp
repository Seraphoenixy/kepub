#include "parse_xml.h"

#include <cstddef>
#include <sstream>

#include <tidy.h>
#include <tidybuffio.h>

#include "error.h"

namespace kepub {

std::string html_tidy(const std::string& html) {
  TidyDoc doc = tidyCreate();
  tidyOptSetBool(doc, TidyXhtmlOut, yes);
  tidyOptSetBool(doc, TidyShowWarnings, no);
  tidyOptSetInt(doc, TidyWrapLen, 0);
  tidyOptSetBool(doc, TidyHideComments, yes);

  auto rc = tidyParseString(doc, html.c_str());

  if (rc >= 0) {
    rc = tidyCleanAndRepair(doc);
  }

  TidyBuffer output = {};
  if (rc >= 0) {
    rc = tidySaveBuffer(doc, &output);
  }

  std::string xhtml;
  if (rc >= 0) {
    xhtml.assign(reinterpret_cast<const char*>(output.bp), output.size);
  } else {
    error("error");
  }

  tidyBufFree(&output);
  tidyRelease(doc);

  return xhtml;
}

Node::Node(const std::string& name) : name_(name) {}

void Node::add_attr(const std::string& attr_name,
                    const std::string& attr_value) {
  attrs_.emplace_back(attr_name, attr_value);
}

void Node::set_text(const std::string& text) { text_ = text; }

void Node::add_child(const Node& node) { children_.push_back(node); }

XHTML::XHTML(const std::string& xhtml) {
  pugi::xml_parse_result err = root_.load_string(xhtml.c_str());
  if (!err) {
    error("parse xml error: {}", err.description());
  }

  node_ = root_.document_element();
}

void XHTML::save(std::string_view file) const {
  root_.save_file(file.data(), "\t",
                  pugi::format_default | pugi::format_no_escapes);
}

std::string XHTML::to_string() const {
  std::ostringstream stream;
  root_.save(stream, "\t", pugi::format_default | pugi::format_no_escapes);
  return stream.str();
}

void XHTML::previous() { node_ = node_.parent(); }

void XHTML::reset() { node_ = root_.document_element(); }

void XHTML::move_by_name(std::string_view name) {
  node_ = get_child_by_name(name);
}

void XHTML::move_by_attr(std::string_view name, std::string_view attr_name,
                         std::string_view attr_value) {
  auto child = node_.find_child_by_attribute(name.data(), attr_name.data(),
                                             attr_value.data());

  if (child.empty()) {
    error("can not find this node: {} {} {}, curr node name: {} {}", name,
          attr_name, attr_value, node_.name(),
          node_.attribute("class").value());
  }

  node_ = child;
}

void XHTML::move_by_attr_class(std::string_view name,
                               std::string_view attr_value) {
  move_by_attr(name, "class", attr_value);
}

bool XHTML::has_child() const {
  auto children = node_.children();
  return children.begin() != children.end();
}

std::string XHTML::first_child_attr(std::string_view attr_name) const {
  auto attr = node_.first_child().attribute(attr_name.data());
  if (attr.empty()) {
    error("there is no attribute: {}", attr_name);
  }

  return attr.value();
}

std::string XHTML::last_child_attr(std::string_view attr_name) const {
  auto attr = node_.last_child().attribute(attr_name.data());
  if (attr.empty()) {
    error("there is no attribute: {}", attr_name);
  }

  return attr.value();
}

std::vector<std::string> XHTML::get_children_attr(
    std::string_view name, std::string_view attr_name) const {
  std::vector<std::string> result;

  for (const auto& child : node_.children(name.data())) {
    auto attr = child.attribute(attr_name.data());
    if (attr.empty()) {
      error("there is no {} in {}", attr_name, name);
    }

    result.emplace_back(attr.value());
  }

  if (std::empty(result)) {
    error("get_children_attr no data");
  }

  return result;
}

std::vector<std::string> XHTML::get_children_text(std::string_view name) const {
  std::vector<std::string> result;

  if (std::empty(name)) {
    for (const auto& child : node_.children()) {
      std::string str;
      kepub::XHTML::get_text(child, str);
      result.push_back(str);
    }
  } else {
    for (const auto& child : node_.children(name.data())) {
      std::string str;
      kepub::XHTML::get_text(child, str);
      result.push_back(str);
    }
  }

  if (std::empty(result)) {
    error("get_children_text no data");
  }

  return result;
}

void XHTML::set_child_text(std::string_view name, const std::string& text) {
  auto child = get_child_by_name(name);
  child.remove_children();
  child.append_child(pugi::node_pcdata).set_value(text.c_str());
}

void XHTML::set_child_attr(std::string_view name, std::string_view attr_name,
                           const std::string& attr_value) {
  auto child = get_child_by_name(name);
  auto attr = child.attribute(attr_name.data());

  if (attr.empty()) {
    error("there is no attr {} in {}", attr_name, name);
  }

  attr.set_value(attr_value.data());
}

void XHTML::push_back(const Node& node) {
  auto child = node_.append_child(node.name_.c_str());

  if (child.empty()) {
    error("can not append child");
  }

  for (const auto& [name, value] : node.attrs_) {
    child.append_attribute(name.c_str()).set_value(value.c_str());
  }

  if (!std::empty(node.text_)) {
    child.append_child(pugi::node_pcdata).set_value(node.text_.c_str());
  }

  auto backup = node_;
  node_ = child;
  for (const auto& item : node.children_) {
    push_back(item);
  }
  node_ = backup;
}

void XHTML::push_back(
    std::string_view name,
    const std::vector<std::pair<std::string, std::string>>& attrs,
    const std::string& text) {
  Node node(name.data());

  for (const auto& [attr_name, attr_value] : attrs) {
    node.add_attr(attr_name, attr_value);
  }

  node.set_text(text);

  push_back(node);
}

void XHTML::push_back(const std::string& text) { push_back("p", {}, text); }

std::string XHTML::get_text() const {
  std::string str;
  XHTML::get_text(node_, str);
  return str;
}

void XHTML::get_text(pugi::xml_node node, std::string& str) {
  if (node.children().begin() == node.children().end()) {
    str += node.text().as_string();
  } else {
    for (const auto& child : node.children()) {
      if (node.name() == std::string("p") || node.name() == std::string("br")) {
        str += "\n";
      }
      get_text(child, str);
    }
  }
}

pugi::xml_node XHTML::get_child_by_name(std::string_view name) {
  auto children = node_.children(name.data());
  auto begin = children.begin();
  auto end = children.end();

  std::size_t size = 0;
  for (auto iter = begin; iter != end; ++iter) {
    ++size;
  }

  if (size == 0) {
    error("can not find this node: {}", name);
  } else if (size > 1) {
    error("multiple nodes with the same name : {}", name);
  }

  return *begin;
}

}  // namespace kepub
