#include "html.h"

#include <klib/html.h>

namespace kepub::esjzone {

namespace {

void do_get_node_texts(const pugi::xml_node &node, std::string &str) {
  if (node.children().begin() == node.children().end()) {
    str += node.text().as_string();
  } else {
    for (const auto &child : node.children()) {
      if (node.name() == std::string("p") || node.name() == std::string("br")) {
        str += "\n";
      }
      do_get_node_texts(child, str);
    }
  }
}

}  // namespace

std::vector<std::string> get_node_texts(const pugi::xml_node &node) {
  std::vector<std::string> result;

  for (const auto &child : node.children()) {
    std::string str;
    do_get_node_texts(child, str);
    result.push_back(str);
  }

  return result;
}

pugi::xml_document html_to_xml(const std::string &html) {
  auto xml = klib::html_tidy(html, true);
  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  return doc;
}

}  // namespace kepub::esjzone
