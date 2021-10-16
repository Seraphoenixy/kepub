#include <catch2/catch.hpp>

#include "util.h"

TEST_CASE("check_is_book_id", "[util]") {
  REQUIRE_NOTHROW(kepub::check_is_book_id("12345"));
}

TEST_CASE("str_size", "[util]") {
  REQUIRE(kepub::str_size("你好世界123abc,;.") == 4);
}
