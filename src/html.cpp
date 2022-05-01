#include "html.h"

#include <dbg.h>
#include <klib/html.h>

namespace kepub {

namespace {

// FIXME connect: lightnovel workaround
void do_get_node_texts(const pugi::xml_node &node, std::string &str,
                       bool &connect) {
  dbg(node.name());

  if (node.children().begin() == node.children().end()) {
    if (node.name() == std::string("img")) {
      std::string image_url = node.attribute("src").as_string();
      str += ("\n[IMAGE] " + image_url + "\n");
    } else {
      str += node.text().as_string();
    }
  } else {
    if (node.name() == std::string("ruby")) {
      connect = true;
    }

    for (const auto &child : node.children()) {
      dbg(child.name());

      if (child.name() == std::string("p") ||
          child.name() == std::string("br") ||
          child.name() == std::string("div")) {
        str += "\n";
      }
      do_get_node_texts(child, str, connect);
    }
  }
}

}  // namespace

std::vector<std::string> get_node_texts(const pugi::xml_node &node,
                                        bool is_lightnovel) {
  std::vector<std::string> result;

  std::int32_t count = 0;

  for (const auto &child : node.children()) {
    std::string str;
    bool connect = false;
    do_get_node_texts(child, str, connect);
    if (!is_lightnovel) {
      connect = false;
    }

    if (connect) {
      count = 2;
    }

    if (count > 0) {
      if (std::empty(result)) {
        result.emplace_back();
      }

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
