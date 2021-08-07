#pragma once

#include <cstdlib>
#include <string_view>
#include <utility>

#include <fmt/color.h>
#include <fmt/core.h>

namespace kepub {

template <typename... Args>
void warn(std::string_view fmt, Args &&...args) {
  fmt::print(fmt::fg(fmt::color::yellow), "kepub warning:\n");
  fmt::print(fmt::fg(fmt::color::yellow), fmt, std::forward<Args>(args)...);
  fmt::print("\n");
}

template <typename... Args>
[[noreturn]] void error(std::string_view fmt, Args &&...args) {
  fmt::print(fmt::fg(fmt::color::red), "kepub error:\n");
  fmt::print(fmt::fg(fmt::color::red), fmt, std::forward<Args>(args)...);
  fmt::print("\n");

  std::exit(EXIT_FAILURE);
}

}  // namespace kepub
