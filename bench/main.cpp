#include <filesystem>
#include <string>

#include <catch2/catch.hpp>

#include "util.h"

TEST_CASE("read_file_to_vec", "[bench]") {
  const std::string file_name = "100012892.txt";
  REQUIRE(std::filesystem::exists(file_name));

  BENCHMARK("read_file_to_vec") {
    return kepub::read_file_to_vec(file_name, false);
  };
}
