#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <unicode/utypes.h>

namespace kepub {

void check_file_exist(const std::string &file_name);

void check_is_txt_file(const std::string &file_name);

void check_is_book_id(const std::string &book_id);

std::vector<std::string> read_file_to_vec(const std::string &file_name,
                                          bool translation);

std::int32_t str_size(const std::string &str);

void str_check(const std::string &str);

void push_back(std::vector<std::string> &texts, const std::string &str,
               bool connect_chinese);

void check_icu(UErrorCode status);

std::string get_login_name();

std::string get_password();

std::string num_to_str(std::int32_t i);

void generate_txt(
    const std::string &book_name, const std::string &author,
    const std::vector<std::string> &description,
    const std::vector<std::pair<std::string, std::string>> &chapters);

void generate_txt(
    const std::string &book_name, const std::string &author,
    const std::vector<std::string> &description,
    const std::vector<std::pair<
        std::string,
        std::vector<std::tuple<std::string, std::string, std::string>>>>
        &volume_chapter);

}  // namespace kepub
