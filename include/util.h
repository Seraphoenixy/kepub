#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "novel.h"

namespace kepub {

void check_file_exist(const std::string &file_name);

void check_dir_exist(const std::string &dir_name);

void check_is_txt_file(const std::string &file_name);

void remove_file_or_dir(const std::string &path);

void check_is_book_id(const std::string &book_id);

std::vector<std::string> read_file_to_vec(const std::string &file_name,
                                          bool translation);

void str_check(const std::string &str);

std::int32_t str_size(const std::string &str);

void volume_name_check(const std::string &volume_name);

void title_check(const std::string &title);

void push_back(std::vector<std::string> &texts, const std::string &str,
               bool connect);

void push_back(std::vector<std::string> &texts, const std::string &str);

std::string get_login_name();

std::string get_password();

std::string num_to_str(std::int32_t i);

std::string make_book_name_legal(const std::string &file_name);

void generate_txt(const BookInfo &book_info,
                  const std::vector<Chapter> &chapters);

void generate_txt(const BookInfo &book_info,
                  const std::vector<Volume> &volumes);

}  // namespace kepub
