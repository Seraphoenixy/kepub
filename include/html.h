#pragma once

#include <string>
#include <vector>

#include <pugixml.hpp>

#include "kepub_export.h"

namespace kepub {

std::vector<std::string> KEPUB_EXPORT
get_node_texts(const pugi::xml_node &node);

pugi::xml_document KEPUB_EXPORT html_to_xml(const std::string &html);

}  // namespace kepub
