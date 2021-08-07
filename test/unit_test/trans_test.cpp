#include <catch2/catch.hpp>

#include "trans.h"

TEST_CASE("trans_str") {
  REQUIRE(kepub::trans_str("&amp;", true) == "&amp;");
  REQUIRE(kepub::trans_str("安裝後?", true) == "安装后？");
  REQUIRE(kepub::trans_str("安裝後?", false) == "安裝後？");
}
