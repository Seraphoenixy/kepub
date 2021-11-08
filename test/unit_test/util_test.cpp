#include <catch2/catch.hpp>

#include "util.h"

TEST_CASE("check_is_book_id", "[util]") {
  REQUIRE_NOTHROW(kepub::check_is_book_id("12345"));
}
