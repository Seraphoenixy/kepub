#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace kepub {

inline bool connect_chinese = false;

bool start_with_chinese(const std::string &str);

bool end_with_chinese(const std::string &str);

void create_dir(const std::filesystem::path &path);

std::string read_file_to_str(const std::string &file_name);

std::vector<std::string> read_file_to_vec(const std::string &file_name);

void check_and_write_file(std::ofstream &ofs, std::string_view str);

void push_back(std::vector<std::string> &texts, const std::string &str);

std::string num_to_chapter_name(std::int32_t i);

std::string processing_cmd(std::int32_t argc, char *argv[]);

std::string chapter_line(const std::string &line);

void check_is_txt_file(const std::string &file_name);

void check_file_exist(const std::string &file_name);

}  // namespace kepub
