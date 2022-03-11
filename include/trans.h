#pragma once

#include <string>
#include <string_view>

namespace kepub {

std::string trans_str(const std::string &str, bool translation);

std::string trans_str(std::string_view str, bool translation);

std::string trans_str(const char *str, bool translation);

}  // namespace kepub
