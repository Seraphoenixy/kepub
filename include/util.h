#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include <unicode/utypes.h>

namespace kepub {

struct Options {
  bool no_cover_ = false;
  bool generate_postscript_ = false;
  std::int32_t illustration_num_ = 0;
  std::int32_t image_num_ = 0;

  bool old_style_ = false;
  bool connect_chinese_ = false;

  // for test
  std::string uuid_;
  std::string date_;
};

void create_dir(const std::filesystem::path &path);

void check_is_txt_file(const std::string &file_name);

void check_file_exist(const std::string &file_name);

std::string read_file_to_str(const std::string &file_name);

std::vector<std::string> read_file_to_vec(const std::string &file_name);

void check_and_write_file(std::ofstream &ofs, std::string_view str);

std::string num_to_str(std::int32_t i);

std::string num_to_chapter_name(std::int32_t i);

std::string num_to_illustration_name(std::int32_t i);

std::pair<std::string, Options> processing_cmd(std::int32_t argc,
                                               const char *argv[]);

void push_back(std::vector<std::string> &texts, const std::string &str,
               bool connect_chinese);

std::int32_t str_size(const std::string &str);

void str_check(const std::string &str);

void check_icu(UErrorCode status);

}  // namespace kepub
