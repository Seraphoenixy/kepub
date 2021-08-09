#include <cstdint>
#include <initializer_list>
#include <string>

#include <catch2/catch.hpp>

#include "encoding.h"

namespace {

std::string to_str(std::initializer_list<std::uint8_t> il) {
  std::string str;
  for (auto i : il) {
    str.push_back(static_cast<char>(i));
  }

  return str;
}

}  // namespace

TEST_CASE("detect_encoding") {
  REQUIRE(kepub::detect_encoding("abc") == "UTF-8");
  REQUIRE(kepub::detect_encoding("你好世界") == "UTF-8");

  // https://zh.wikipedia.org/wiki/UTF-16
  std::string utf_16 = to_str(
      {0XFF, 0XFE, 0X31, 0X67, 0X2C, 0X00, 0X7F, 0X80, 0X69, 0XD8, 0XA5, 0XDE});
  REQUIRE(kepub::detect_encoding(utf_16) == "UTF-16LE");
}
