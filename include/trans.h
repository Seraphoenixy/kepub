#pragma once

#include <string>
#include <string_view>

namespace kepub {

class Converter {
 public:
  Converter();

  [[nodiscard]] std::string convert(const std::string &str) const;
};

std::string trans_str(const char *str, bool translation);

std::string trans_str(std::string_view str, bool translation);

std::string trans_str(const std::string &str, bool translation);

}  // namespace kepub
