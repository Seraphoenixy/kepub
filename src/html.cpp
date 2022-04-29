#include "html.h"

#include <klib/html.h>

namespace kepub {

namespace {

void do_get_node_texts(const pugi::xml_node &node, std::string &str,
                       bool &connect) {
  if (node.children().begin() == node.children().end()) {
    if (node.name() == std::string("img")) {
      std::string image_url = node.attribute("src").as_string();
      str += ("[IMAGE] " + image_url);
    } else {
      str += node.text().as_string();
    }
  } else {
    if (node.name() == std::string("ruby")) {
      connect = true;
    }

    for (const auto &child : node.children()) {
      if (node.name() == std::string("p") || node.name() == std::string("br")) {
        str += "\n";
      }
      do_get_node_texts(child, str, connect);
    }
  }
}

}  // namespace

std::vector<std::string> get_node_texts(const pugi::xml_node &node) {
  std::vector<std::string> result;

  std::int32_t count = 0;

  for (const auto &child : node.children()) {
    std::string str;
    bool connect = false;
    do_get_node_texts(child, str, connect);

    if (connect) {
      count = 2;
    }

    if (count > 0) {
      result.back().append(str);
      --count;
    } else {
      result.push_back(str);
    }
  }

  return result;
}

pugi::xml_document html_to_xml(const std::string &html) {
  auto xml = klib::html_tidy(html, true);
  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  return doc;
}

}  // namespace kepub
