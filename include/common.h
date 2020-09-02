#pragma once

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include <fmt/color.h>
#include <fmt/format.h>
#include <unicode/unistr.h>

template <typename... Args>
[[noreturn]] void error(std::string_view format_str, const Args &... args) {
  fmt::print(fmt::fg(fmt::terminal_color::red), "error: ");
  fmt::print(fmt::fg(fmt::terminal_color::red), format_str, args...);
  fmt::print("\n");

  std::exit(EXIT_FAILURE);
}

std::string num_to_str(std::int32_t i);

void do_create_directory(const std::string &dir);

void check_file_is_open(const std::ifstream &file, const std::string &filename);

void check_file_is_open(const std::ofstream &file, const std::string &filename);

void create_directory(const std::string &book_name);

void custom_trans(icu::UnicodeString &str);

std::pair<std::string, std::vector<std::string>>
read_file(const std::string &filename);

void generate_content_opf(const std::string &book_name,
                          const std::string &author, std::int32_t count);

void generate_toc_ncx(const std::string &book_name,
                      const std::vector<std::string> &titles);

std::pair<std::vector<std::string>, bool> processing_cmd(std::int32_t argc,
                                                         char *argv[]);

std::string chapter_file_string(const std::string &title);

std::string chapter_file_string_text(const std::string &text);

const char *chapter_file_string_end();

void generate_xhtml(const std::string &book_name,
                    const std::vector<std::string> &texts);

std::string get_chapter_filename(const std::string &book_name,
                                 std::int32_t count);
