#pragma once

#include <string>
#include <vector>

#include <pugixml.hpp>

#include "config.h"

namespace kepub::esjzone {

std::vector<std::string> KEPUB_PUBLIC
get_node_texts(const pugi::xml_node &node);

pugi::xml_document KEPUB_PUBLIC html_to_xml(const std::string &html);

}  // namespace kepub::esjzone
