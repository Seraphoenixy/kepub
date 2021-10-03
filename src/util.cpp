#include "util.h"

#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <klib/error.h>
#include <klib/util.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include "encoding.h"
#include "trans.h"
#include "version.h"

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
  return u_ispunct(c) || c == to_unicode("～") || c == to_unicode("ー");
}

}  // namespace

namespace kepub {

void check_file_exist(const std::string &file_name) {
  if (!std::filesystem::is_regular_file(file_name)) {
    klib::error("The file not exist: '{}'", file_name);
  }
}

void check_is_txt_file(const std::string &file_name) {
  check_file_exist(file_name);

  if (std::filesystem::path(file_name).extension() != ".txt") {
    klib::error("Need a txt file: {}", file_name);
  }
}

void check_is_book_id(const std::string &book_id) {
  if (!std::all_of(std::begin(book_id), std::end(book_id),
                   [](char c) { return std::isdigit(c); })) {
    klib::error("Need a book id: {}", book_id);
  }
}

std::vector<std::string> read_file_to_vec(const std::string &file_name) {
  auto data = klib::read_file(file_name, false);

  if (auto encoding = detect_encoding(data); encoding != "UTF-8") {
    klib::error("file '{}' encoding is not UTF-8 ({})", file_name, encoding);
  }

  std::vector<std::string> result;
  boost::split(result, data, boost::is_any_of("\n"), boost::token_compress_on);

  for (auto &item : result) {
    item = trans_str(item);
  }

  std::erase_if(result,
                [](const std::string &line) { return std::empty(line); });

  return result;
}

std::int32_t str_size(const std::string &str) {
  std::int32_t result = 0;

  for (auto c : klib::utf8_to_utf32(str)) {
    if (klib::is_chinese(c)) {
      ++result;
    }
  }

  return result;
}

void str_check(const std::string &str) {
  auto copy = str;
  std::erase_if(copy, [](auto c) { return std::isalnum(c); });

  for (auto c : klib::utf8_to_utf32(copy)) {
    if (!u_isblank(c) && !klib::is_chinese(c) && !is_punct(c)) {
      std::string temp;
      UChar32 ch = c;
      klib::warn("Unknown character: {} in {}",
                 icu::UnicodeString::fromUTF32(&ch, 1).toUTF8String(temp), str);
    }
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
      str.starts_with("。") || str.starts_with("”") || str.starts_with("、") ||
      str.starts_with("』") || str.starts_with("》") || str.starts_with("】") ||
      str.starts_with("）")) {
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

std::pair<std::string, Options> processing_cmd(std::int32_t argc,
                                               const char *argv[]) {
  std::string input_file;
  Options options;

  boost::program_options::options_description generic("Generic options");
  generic.add_options()("version,v", "print version string");
  generic.add_options()("help,h", "produce help message");

  boost::program_options::options_description config("Configuration");
  config.add_options()("connect,c", "connect chinese");
  config.add_options()("no-cover", "do not generate cover");
  config.add_options()("postscript,p", "generate postscript");
  config.add_options()("download-without-authorization,d",
                       "download without authorization");
  config.add_options()(
      "illustration,i",
      boost::program_options::value<std::int32_t>(&options.illustration_num_)
          ->default_value(0),
      "generate illustration");
  config.add_options()(
      "image",
      boost::program_options::value<std::int32_t>(&options.image_num_)
          ->default_value(0),
      "generate image");

  config.add_options()("no-compress", "do not compress(for testing)");
  config.add_options()(
      "date",
      boost::program_options::value<std::string>(&options.date_)
          ->default_value(""),
      "specify the date(for testing)");
  config.add_options()(
      "uuid",
      boost::program_options::value<std::string>(&options.uuid_)
          ->default_value(""),
      "specify the uuid(for testing)");

  boost::program_options::options_description hidden("Hidden options");
  hidden.add_options()("input-file",
                       boost::program_options::value<std::string>(&input_file));

  boost::program_options::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(hidden);

  boost::program_options::options_description visible("Allowed options");
  visible.add(generic).add(config);

  boost::program_options::positional_options_description p;
  p.add("input-file", 1);

  boost::program_options::variables_map vm;
  store(boost::program_options::command_line_parser(argc, argv)
            .options(cmdline_options)
            .positional(p)
            .run(),
        vm);
  notify(vm);

  if (vm.contains("help")) {
    fmt::print("{}", visible);
    std::exit(EXIT_SUCCESS);
  }

  if (vm.contains("version")) {
    fmt::print("{} version: {}", argv[0], kepub_version());
    std::exit(EXIT_SUCCESS);
  }

  if (!vm.contains("input-file")) {
    klib::error("need a text file name or a url");
  }

  if (vm.contains("connect")) {
    options.connect_chinese_ = true;
  }
  if (vm.contains("no-cover")) {
    options.no_cover_ = true;
  }
  if (vm.contains("no-compress")) {
    options.no_compress_ = true;
  }
  if (vm.contains("postscript")) {
    options.generate_postscript_ = true;
  }
  if (vm.contains("download-without-authorization")) {
    options.download_without_authorization_ = true;
  }

  return {input_file, options};
}

void check_icu(UErrorCode status) {
  if (U_FAILURE(status)) {
    klib::error("{}", u_errorName(status));
  }
}

std::string get_login_name() {
  std::string login_name;

  std::cout << "login name: ";
  std::cin >> login_name;
  if (std::empty(login_name)) {
    klib::error("login name is empty");
  }

  return login_name;
}

std::string get_password() {
  std::string password = getpass("password: ");

  if (std::empty(password)) {
    klib::error("password is empty");
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

  oss << author << "\n\n";
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

  std::ofstream book_ofs(book_name + ".txt");
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

  oss << author << "\n\n";
  for (const auto &line : description) {
    oss << line << "\n";
  }
  oss << "\n";

  for (const auto &[volume_name, chapters] : volume_chapter) {
    oss << "[VOLUME] " << volume_name << "\n\n";

    for (const auto &[chapter_id, chapter_title, content] : chapters) {
      oss << "[WEB] " << chapter_title << "\n\n";
      oss << content << "\n\n";
    }
  }

  std::string str = oss.str();
  // '\n'
  str.pop_back();

  std::ofstream book_ofs(book_name + ".txt");
  book_ofs << str << std::flush;
}

}  // namespace kepub
