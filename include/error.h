#pragma once

#include <cstdlib>

#include <fmt/color.h>
#include <fmt/format.h>

template <typename... Args>
[[noreturn]] void error(std::string_view format_str, const Args &...args) {
  fmt::print(fmt::fg(fmt::color::red), "error: ");
  fmt::print(fmt::fg(fmt::color::red), format_str, args...);
  fmt::print("\n");

  std::exit(EXIT_FAILURE);
}
