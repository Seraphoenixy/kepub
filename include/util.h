#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <unicode/utypes.h>

namespace kepub {

struct Options {
  bool no_cover_ = false;
  bool generate_postscript_ = false;
  std::int32_t illustration_num_ = 0;
  std::int32_t image_num_ = 0;

  bool connect_chinese_ = false;

  // for testing
  std::string uuid_;
  std::string date_;
  bool no_compress_ = false;
};

void check_file_exist(const std::string &file_name);

void check_is_txt_file(const std::string &file_name);

std::vector<std::string> read_file_to_vec(const std::string &file_name);

std::int32_t str_size(const std::string &str);

void str_check(const std::string &str);

void push_back(std::vector<std::string> &texts, const std::string &str,
               bool connect_chinese);

std::pair<std::string, Options> processing_cmd(std::int32_t argc,
                                               const char *argv[]);

void check_icu(UErrorCode status);

}  // namespace kepub
