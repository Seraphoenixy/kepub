#pragma once

#include <string>

namespace kepub {

std::string get_page(const std::string& url);

void get_file(const std::string& url, const std::string& file_name);

}  // namespace kepub
