#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

std::string get_chapter_filename(const std::string &book_name,
                                 std::int32_t count);

void check_file_is_open(const std::ifstream &file, const std::string &filename);

void check_file_is_open(const std::ofstream &file, const std::string &filename);

std::string chapter_file_begin(const std::string &title);

std::string chapter_file_text(const std::string &text);

const char *chapter_file_end();

std::pair<std::vector<std::string>, bool> processing_cmd(std::int32_t argc,
                                                         char *argv[]);

void create_epub_directory(const std::string &book_name,
                           const std::vector<std::string> &description = {});

void generate_xhtml(const std::string &book_name,
                    const std::vector<std::string> &texts);

void generate_content_opf(const std::string &book_name,
                          const std::string &author, std::int32_t count);

void generate_toc_ncx(const std::string &book_name,
                      const std::vector<std::string> &titles);

std::pair<std::string, std::vector<std::string>> read_file(
    const std::string &filename);

void push_back(std::vector<std::string> &texts, const std::string &str);
