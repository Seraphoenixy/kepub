#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "config.h"
#include "novel.h"

namespace kepub {

void KEPUB_PUBLIC check_file_exist(const std::string &file_name);

void KEPUB_PUBLIC check_dir_exist(const std::string &dir_name);

void KEPUB_PUBLIC check_is_txt_file(const std::string &file_name);

void KEPUB_PUBLIC check_is_epub_file(const std::string &file_name);

void KEPUB_PUBLIC remove_file_or_dir(const std::string &path);

void KEPUB_PUBLIC check_is_book_id(const std::string &book_id);

std::vector<std::string> KEPUB_PUBLIC
read_file_to_vec(const std::string &file_name, bool translation);

void KEPUB_PUBLIC str_check(const std::string &str);

std::int32_t KEPUB_PUBLIC str_size(const std::string &str);

void KEPUB_PUBLIC volume_name_check(const std::string &volume_name);

void KEPUB_PUBLIC title_check(const std::string &title);

void KEPUB_PUBLIC push_back(std::vector<std::string> &texts,
                            const std::string &str, bool connect);

void KEPUB_PUBLIC push_back(std::vector<std::string> &texts,
                            const std::string &str);

std::string KEPUB_PUBLIC get_login_name();

std::string KEPUB_PUBLIC get_password();

std::string KEPUB_PUBLIC num_to_str(std::int32_t i);

std::string KEPUB_PUBLIC make_book_name_legal(const std::string &file_name);

void KEPUB_PUBLIC generate_txt(const BookInfo &book_info,
                               const std::vector<Chapter> &chapters);

void KEPUB_PUBLIC generate_txt(const BookInfo &book_info,
                               const std::vector<Volume> &volumes);

}  // namespace kepub
