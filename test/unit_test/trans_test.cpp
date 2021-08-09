#include <catch2/catch.hpp>

#include "trans.h"

TEST_CASE("trans_str") {
  REQUIRE(kepub::trans_str("&amp;") == "&");
  REQUIRE(kepub::trans_str("安裝後?") == "安装后？");
}
