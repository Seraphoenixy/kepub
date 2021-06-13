#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace kepub {

inline bool connect_chinese = false;

inline bool no_cover = false;

inline bool download_cover = false;

inline bool postscript = false;

inline bool old_style = false;

inline std::int32_t illustration_num = 0;

inline std::int32_t max_chapter = 0;

inline std::string date;

void create_dir(const std::filesystem::path &path);

void check_is_txt_file(const std::string &file_name);

void check_file_exist(const std::string &file_name);

void check_is_url(const std::string &url);

std::string read_file_to_str(const std::string &file_name);

std::vector<std::string> read_file_to_vec(const std::string &file_name);

void check_and_write_file(std::ofstream &ofs, std::string_view str);

std::string num_to_str(std::int32_t i);

std::string num_to_chapter_name(std::int32_t i);

std::string num_to_illustration_name(std::int32_t i);

std::string processing_cmd(std::int32_t argc, char *argv[]);

void push_back(std::vector<std::string> &texts, const std::string &str);

std::int32_t str_size(const std::string &str);

}  // namespace kepub
