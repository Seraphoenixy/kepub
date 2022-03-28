#pragma once

#include <string>
#include <string_view>

#include "kepub_export.h"

namespace kepub {

std::string KEPUB_EXPORT trans_str(const std::string &str, bool translation);

std::string KEPUB_EXPORT trans_str(std::string_view str, bool translation);

std::string KEPUB_EXPORT trans_str(const char *str, bool translation);

}  // namespace kepub
