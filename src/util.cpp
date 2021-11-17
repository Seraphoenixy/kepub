#include "util.h"

#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

#include <klib/error.h>
#include <klib/unicode.h>
#include <klib/util.h>
#include <simdjson.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <boost/algorithm/string.hpp>

#include "trans.h"

namespace {

char32_t to_unicode(const std::string &str) {
  auto utf32 = klib::utf8_to_utf32(str);
  assert(std::size(utf32) == 1);

  return utf32.front();
}

bool start_with_chinese(const std::string &str) {
  return klib::is_chinese(klib::utf8_to_utf32(str).front());
}

bool end_with_chinese(const std::string &str) {
  return klib::is_chinese(klib::utf8_to_utf32(str).back());
}

bool is_punct(char32_t c) {
  return u_ispunct(c) || c == to_unicode("～") || c == to_unicode("ー") ||
         c == to_unicode("♂") || c == to_unicode("♀") || c == to_unicode("◇") ||
         c == to_unicode("￮") || c == to_unicode("+") || c == to_unicode("=");
}

std::string make_book_name_legal(const std::string &file_name) {
  auto new_file_name = klib::make_file_or_dir_name_legal(file_name);
  if (new_file_name != file_name) {
    klib::warn(
        "The title of the book is illegal, and the title of the book has been "
        "changed from '{}' to '{}'",
        file_name, new_file_name);
  }

  return new_file_name;
}

}  // namespace

namespace kepub {

void check_file_exist(const std::string &file_name) {
  if (!std::filesystem::is_regular_file(file_name)) {
    klib::error(KLIB_CURR_LOC, "The file not exist: '{}'", file_name);
  }
}

void check_is_txt_file(const std::string &file_name) {
  check_file_exist(file_name);

  if (std::filesystem::path(file_name).extension() != ".txt") {
    klib::error(KLIB_CURR_LOC, "Need a txt file: {}", file_name);
  }
}

void check_is_book_id(const std::string &book_id) {
  if (!std::all_of(std::begin(book_id), std::end(book_id),
                   [](char c) { return std::isdigit(c); })) {
    klib::error(KLIB_CURR_LOC, "Need a book id: {}", book_id);
  }
}

std::vector<std::string> read_file_to_vec(const std::string &file_name,
                                          bool translation) {
  auto data = klib::read_file(file_name, false);

  if (!simdjson::validate_utf8(data)) {
    klib::error(KLIB_CURR_LOC, "file '{}' encoding is not UTF-8", file_name);
  }

  std::vector<std::string> result;
  boost::split(result, data, boost::is_any_of("\n"), boost::token_compress_on);

  for (auto &item : result) {
    item = trans_str(item, translation);
  }

  std::erase_if(result,
                [](const std::string &line) { return std::empty(line); });

  return result;
}

void str_check(const std::string &str) {
  auto copy = str;
  std::erase_if(copy, [](char c) { return std::isalnum(c) || c == ' '; });

  for (auto c : klib::utf8_to_utf32(copy)) {
    if (!klib::is_chinese(c) && !is_punct(c)) {
      std::string temp;
      UChar32 ch = c;
      klib::warn("Unknown character: {} in {}",
                 icu::UnicodeString::fromUTF32(&ch, 1).toUTF8String(temp), str);
    }
  }
}

void title_check(const std::string &title) {
  if (std::count_if(std::begin(title), std::end(title),
                    [](char c) { return c == ' '; }) != 1) {
    klib::warn("Irregular title format: {}", title);
  }
}

void push_back(std::vector<std::string> &texts, const std::string &str,
               bool connect_chinese) {
  if (std::empty(str)) {
    return;
  }

  if (std::empty(texts)) {
    texts.push_back(str);
    return;
  }

  if (texts.back().ends_with("，") || str.starts_with("，") ||
      str.starts_with("。") || str.starts_with("”") || str.starts_with("｣") ||
      str.starts_with("、") || str.starts_with("』") || str.starts_with("》") ||
      str.starts_with("】") || str.starts_with("）")) {
    texts.back().append(str);
  } else if (std::isalpha(texts.back().back()) && std::isalpha(str.front())) {
    texts.back().append(" " + str);
  } else if (end_with_chinese(texts.back()) && std::isalpha(str.front())) {
    texts.back().append(" " + str);
  } else if (std::isalpha(texts.back().back()) && start_with_chinese(str)) {
    texts.back().append(" " + str);
  } else if ((std::isalpha(texts.back().back()) ||
              end_with_chinese(texts.back())) &&
             (str.starts_with("！") || str.starts_with("？"))) {
    texts.back().append(str);
  } else if (connect_chinese && end_with_chinese(texts.back()) &&
             start_with_chinese(str)) {
    texts.back().append(str);
  } else {
    texts.push_back(str);
  }
}

void check_icu(UErrorCode status) {
  if (U_FAILURE(status)) {
    klib::error(KLIB_CURR_LOC, "{}", u_errorName(status));
  }
}

std::string get_login_name() {
  std::string login_name;

  std::cout << "login name: ";
  std::cin >> login_name;
  if (std::empty(login_name)) {
    klib::error(KLIB_CURR_LOC, "login name is empty");
  }

  return login_name;
}

std::string get_password() {
  std::string password = getpass("password: ");

  if (std::empty(password)) {
    klib::error(KLIB_CURR_LOC, "password is empty");
  }

  return password;
}

std::string num_to_str(std::int32_t i) {
  assert(i > 0);

  auto str = std::to_string(i);
  if (i < 10) {
    return "00" + str;
  } else if (i < 100) {
    return "0" + str;
  } else {
    return str;
  }
}

void generate_txt(
    const std::string &book_name, const std::string &author,
    const std::vector<std::string> &description,
    const std::vector<std::pair<std::string, std::string>> &chapters) {
  std::ostringstream oss;

  oss << "[AUTHOR]" << '\n';
  oss << author << "\n\n";

  oss << "[INTRO]" << '\n';
  for (const auto &line : description) {
    oss << line << "\n";
  }
  oss << "\n";

  for (const auto &[chapter_title, content] : chapters) {
    oss << "[WEB] " << chapter_title << "\n\n";
    oss << content << "\n\n";
  }

  std::string str = oss.str();
  // '\n'
  str.pop_back();

  std::ofstream book_ofs(make_book_name_legal(book_name) + ".txt");
  book_ofs << str << std::flush;
}

void generate_txt(
    const std::string &book_name, const std::string &author,
    const std::vector<std::string> &description,
    const std::vector<std::pair<
        std::string,
        std::vector<std::tuple<std::string, std::string, std::string>>>>
        &volume_chapter) {
  std::ostringstream oss;

  oss << "[AUTHOR]" << '\n';
  oss << author << "\n\n";

  oss << "[INTRO]" << '\n';
  for (const auto &line : description) {
    oss << line << "\n";
  }
  oss << "\n";

  for (const auto &[volume_name, chapters] : volume_chapter) {
    if (std::empty(chapters)) {
      continue;
    }

    oss << "[VOLUME] " << volume_name << "\n\n";

    for (const auto &[chapter_id, chapter_title, content] : chapters) {
      oss << "[WEB] " << chapter_title << "\n\n";
      oss << content << "\n\n";
    }
  }

  std::string str = oss.str();
  // '\n'
  str.pop_back();

  std::ofstream book_ofs(make_book_name_legal(book_name) + ".txt");
  book_ofs << str << std::flush;
}

}  // namespace kepub
