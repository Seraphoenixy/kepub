#include "util.h"

#include <cassert>
#include <cctype>
#include <clocale>
#include <cstddef>
#include <cstdlib>
#include <cuchar>
#include <filesystem>
#include <fstream>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include "error.h"
#include "version.h"

namespace {

std::u32string utf8_to_utf32(const std::string &str) {
  setlocale(LC_ALL, "en_US.utf8");

  std::u32string result;

  std::size_t rc;
  char32_t out;
  auto begin = str.c_str();
  mbstate_t state = {};

  while ((rc = mbrtoc32(&out, begin, std::size(str), &state))) {
    assert(rc != static_cast<std::size_t>(-3));

    if (rc == static_cast<std::size_t>(-2)) {
      kepub::error("Incomplete byte composition Incomplete byte composition");
    } else if (rc == static_cast<std::size_t>(-1)) {
      kepub::error("Character encoding error");
    }

    begin += rc;
    result.push_back(out);
  }

  return result;
}

// https://stackoverflow.com/questions/62531882/is-there-a-way-to-detect-chinese-characters-in-c-using-boost
template <char32_t a, char32_t b>
class UnicodeRange {
  static_assert(a <= b, "proper range");

 public:
  constexpr bool operator()(char32_t x) const noexcept {
    return x >= a && x <= b;
  }
};

using UnifiedIdeographs = UnicodeRange<0x4E00, 0x9FFF>;
using UnifiedIdeographsA = UnicodeRange<0x3400, 0x4DBF>;
using UnifiedIdeographsB = UnicodeRange<0x20000, 0x2A6DF>;
using UnifiedIdeographsC = UnicodeRange<0x2A700, 0x2B73F>;
using UnifiedIdeographsD = UnicodeRange<0x2B740, 0x2B81F>;
using UnifiedIdeographsE = UnicodeRange<0x2B820, 0x2CEAF>;
using CompatibilityIdeographs = UnicodeRange<0xF900, 0xFAFF>;
using CompatibilityIdeographsSupplement = UnicodeRange<0x2F800, 0x2FA1F>;

constexpr bool is_chinese(char32_t x) noexcept {
  return UnifiedIdeographs{}(x) || UnifiedIdeographsA{}(x) ||
         UnifiedIdeographsB{}(x) || UnifiedIdeographsC{}(x) ||
         UnifiedIdeographsD{}(x) || UnifiedIdeographsE{}(x) ||
         CompatibilityIdeographs{}(x) || CompatibilityIdeographsSupplement{}(x);
}

std::string num_to_str(std::int32_t i) {
  if (i <= 0 || i >= 1000) {
    kepub::error("too many xhtml files need to be generated");
  }

  if (i < 10) {
    return "00" + std::to_string(i);
  } else if (i < 100) {
    return "0" + std::to_string(i);
  } else {
    return std::to_string(i);
  }
}

}  // namespace

namespace kepub {

bool start_with_chinese(const std::string &str) {
  return is_chinese(utf8_to_utf32(str).front());
}

bool end_with_chinese(const std::string &str) {
  return is_chinese(utf8_to_utf32(str).back());
}

void create_dir(const std::filesystem::path &path) {
  if (!std::filesystem::create_directory(path)) {
    kepub::error("can not create directory: {}", path.string());
  }
}

std::string read_file_to_str(const std::string &file_name) {
  std::ifstream ifs(file_name, std::ifstream::binary);
  std::string data;

  data.resize(static_cast<std::string::size_type>(
      ifs.seekg(0, std::ifstream::end).tellg()));
  ifs.seekg(0, std::ifstream::beg)
      .read(data.data(), static_cast<std::streamsize>(std::size(data)));

  return data;
}

std::vector<std::string> read_file_to_vec(const std::string &file_name) {
  auto data = read_file_to_str(file_name);
  std::vector<std::string> result;
  boost::split(result, data, boost::is_any_of("\n"), boost::token_compress_on);
  return result;
}

void check_and_write_file(std::ofstream &ofs, std::string_view str) {
  if (!ofs) {
    error("file is not open");
  }

  ofs << str << std::flush;
}

void push_back(std::vector<std::string> &texts, const std::string &str) {
  if (std::empty(str)) {
    return;
  }

  if (std::empty(texts)) {
    texts.push_back(str);
    return;
  }

  if (texts.back().ends_with("，") || str.starts_with("，")) {
    texts.back().append(str);
  } else if (std::isalpha(texts.back().back()) && std::isalpha(str.front())) {
    texts.back().append(" " + str);
  } else {
    if (connect_chinese && end_with_chinese(texts.back()) &&
        start_with_chinese(str)) {
      texts.back().append(str);
    } else {
      texts.push_back(str);
    }
  }
}

std::string num_to_chapter_name(std::int32_t i) {
  return "chapter" + num_to_str(i) + ".xhtml";
}

std::string processing_cmd(std::int32_t argc, char *argv[]) {
  std::vector<std::string> input_file;

  boost::program_options::options_description generic("Generic options");
  generic.add_options()("version,v", "print version string")(
      "help,h", "produce help message");

  boost::program_options::options_description config("Configuration");
  config.add_options()("connect,c", "connect chinese");

  boost::program_options::options_description hidden("Hidden options");
  hidden.add_options()(
      "input-file",
      boost::program_options::value<std::vector<std::string>>(&input_file));

  boost::program_options::options_description cmdline_options;
  cmdline_options.add(generic).add(config).add(hidden);

  boost::program_options::options_description visible("Allowed options");
  visible.add(generic).add(config);

  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  boost::program_options::variables_map vm;
  store(boost::program_options::command_line_parser(argc, argv)
            .options(cmdline_options)
            .positional(p)
            .run(),
        vm);
  notify(vm);

  if (vm.contains("help")) {
    fmt::print("Usage: {} [options] file...\n\n{}\n", argv[0], visible);
    std::exit(EXIT_SUCCESS);
  }

  if (vm.contains("version")) {
    fmt::print("{} version: {}.{}.{}\n", argv[0], KEPUB_VER_MAJOR,
               KEPUB_VER_MINOR, KEPUB_VER_PATCH);
    std::exit(EXIT_SUCCESS);
  }

  if (vm.contains("connect")) {
    connect_chinese = true;
  }

  if (!vm.contains("input-file")) {
    error("need a text file name");
  }

  if (std::size(input_file) != 1) {
    error("does not support multiple files");
  }

  return input_file.front();
}

std::string chapter_line(const std::string &line) {
  return "<p>" + line + "</p>";
}

void check_is_txt_file(const std::string &file_name) {
  if (std::filesystem::path(file_name).extension() != ".txt") {
    kepub::error("need a txt file: {}", file_name);
  }
}

}  // namespace kepub
