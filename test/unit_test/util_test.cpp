#include <catch2/catch.hpp>

#include "util.h"

TEST_CASE("check_is_book_id", "[util]") {
  REQUIRE_NOTHROW(kepub::check_is_book_id("12345"));
}

TEST_CASE("title_check", "[util]") {
  REQUIRE_NOTHROW(kepub::title_check("第一章 被俘虏的开始"));
  REQUIRE_NOTHROW(kepub::title_check("第一百三十二章 标标标标标标标标标"));
  REQUIRE_NOTHROW(kepub::title_check("第123话 标题标标标标"));

  REQUIRE_NOTHROW(kepub::title_check("第一章 "));
  REQUIRE_NOTHROW(kepub::title_check("第1二3话"));
  REQUIRE_NOTHROW(kepub::title_check("第123话标题"));
  REQUIRE_NOTHROW(kepub::title_check("123话 标题"));
}
