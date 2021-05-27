#pragma once

#include <string>

namespace kepub {

void compress(const std::string &dir);

void decompress(const std::string &file_name, const std::string &dir = "");

}  // namespace kepub
