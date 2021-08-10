#include <catch2/catch.hpp>

#include "util.h"

TEST_CASE("str_size", "[util]") {
  REQUIRE(kepub::str_size("你好世界123abc,;.") == 4);
}

TEST_CASE("processing_cmd", "[util]") {
  const char *argv[] = {"kepub", "a.txt", "-c"};
  auto argc = std::size(argv);

  auto [file, options] = kepub::processing_cmd(argc, argv);

  REQUIRE(file == "a.txt");
  REQUIRE(options.connect_chinese_);
  REQUIRE_FALSE(options.no_compress_);
  REQUIRE_FALSE(options.generate_postscript_);
  REQUIRE_FALSE(options.no_cover_);

  REQUIRE(options.image_num_ == 0);
  REQUIRE(options.illustration_num_ == 0);
  REQUIRE(std::empty(options.date_));
  REQUIRE(std::empty(options.uuid_));
}
