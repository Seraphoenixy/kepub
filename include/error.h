#pragma once

#include <cstdlib>

#include <fmt/color.h>
#include <fmt/format.h>

namespace kepub {

template <typename... Args>
[[noreturn]] void error(std::string_view format_str, const Args &...args) {
  fmt::print(fmt::fg(fmt::color::red), "error: ");
  fmt::print(fmt::fg(fmt::color::red), format_str, args...);
  fmt::print("\n");

  std::exit(EXIT_FAILURE);
}

template <typename... Args>
void warning(std::string_view format_str, const Args &...args) {
  fmt::print(fmt::fg(fmt::color::yellow), "warning: ");
  fmt::print(fmt::fg(fmt::color::yellow), format_str, args...);
  fmt::print("\n");
}

}  // namespace kepub
