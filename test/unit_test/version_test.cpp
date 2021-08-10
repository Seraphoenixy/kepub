#include <catch2/catch.hpp>

#include "version.h"

TEST_CASE("kepub_version", "[version]") {
  REQUIRE(kepub::kepub_version() == "v0.5.2");
}
