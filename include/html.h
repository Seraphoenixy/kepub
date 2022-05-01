#pragma once

#include <string>
#include <vector>

#include <klib/log.h>
#include <pugixml.hpp>

#include "kepub_export.h"

namespace kepub {

std::vector<std::string> KEPUB_EXPORT
get_node_texts(const pugi::xml_node &node, bool is_lightnovel = false);

pugi::xml_document KEPUB_EXPORT html_to_xml(const std::string &html);

#define CHECK_NODE(node)                   \
  do {                                     \
    if ((node).empty()) [[unlikely]] {     \
      klib::error("Failed to parse HTML"); \
    }                                      \
  } while (0)

}  // namespace kepub
