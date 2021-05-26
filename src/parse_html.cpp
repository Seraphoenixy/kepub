#include "parse_html.h"

#include <sstream>

#include <tidy.h>
#include <tidybuffio.h>

#include "error.h"

namespace kepub {

XHTML::XHTML(const std::string& xhtml) {
  pugi::xml_parse_result err = root_.load_string(xhtml.c_str());
  if (!err) {
    error("parse xml error: {}", err.description());
  }

  node_ = root_.document_element();
}

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

void XHTML::previous() { node_ = node_.parent(); }

std::vector<pugi::xml_node> XHTML::children() const {
  std::vector<pugi::xml_node> result;

  for (const auto& node : node_.children()) {
    result.push_back(node);
  }

  return result;
}

void XHTML::save(std::string_view file) const { root_.save_file(file.data()); }

void XHTML::push_back(const Node& node) {
  auto child = node_.append_child(node.name_.c_str());
  insert(child, node);
}

void XHTML::insert_before(const Node& node, std::string_view attr_name,
                          std::string_view attr_value) {
  auto hint =
      node_.find_child_by_attribute(attr_name.data(), attr_value.data());
  auto child = node_.insert_child_before(node.name_.c_str(), hint);

  insert(child, node);
}

void XHTML::insert_after(const Node& node, std::string_view attr_name,
                         std::string_view attr_value) {
  auto hint =
      node_.find_child_by_attribute(attr_name.data(), attr_value.data());
  auto child = node_.insert_child_after(node.name_.c_str(), hint);

  insert(child, node);
}

void XHTML::move_by_name(std::string_view name) {
  auto child = node_.children(name.data());
  auto begin = child.begin();
  auto end = child.end();

  std::size_t size = 0;
  for (auto iter = begin; iter != end; ++iter) {
    ++size;
  }

  if (size == 0) {
    error("can not find this node: {}", name);
  } else if (size > 1) {
    error("multiple nodes with the same name : {}", name);
  }

  node_ = *begin;
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

void XHTML::insert(pugi::xml_node child, const Node& node) {
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

void XHTML::move_by_attr_class(std::string_view name,
                               std::string_view attr_value) {
  move_by_attr(name, "class", attr_value);
}

std::string XHTML::to_string() const {
  std::ostringstream stream;
  root_.save(stream);
  return stream.str();
}

pugi::xml_node XHTML::last_child() const { return node_.last_child(); }

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
    kepub::error("error");
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

}  // namespace kepub
