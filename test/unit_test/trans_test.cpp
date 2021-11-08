#include <catch2/catch.hpp>

#include "trans.h"

TEST_CASE("trans_str", "[trans]") {
  REQUIRE(kepub::trans_str("“安装后”", false) == "“安装后”");
  REQUIRE(kepub::trans_str("&amp;", false) == "&");
  REQUIRE(kepub::trans_str("安裝後?", true) == "安装后？");

  REQUIRE(kepub::trans_str("，，，", false) == "，");
}
