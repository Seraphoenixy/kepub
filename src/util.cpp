#include "util.h"

#include <cassert>
#include <cctype>
#include <clocale>
#include <cstddef>
#include <cstdlib>
#include <cuchar>
#include <regex>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>

#include "error.h"
#include "trans.h"
#include "version.h"

namespace {

std::u32string utf8_to_utf32(const std::string &str) {
  std::setlocale(LC_ALL, "en_US.utf8");

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

bool start_with_chinese(const std::string &str) {
  return is_chinese(utf8_to_utf32(str).front());
}

bool end_with_chinese(const std::string &str) {
  return is_chinese(utf8_to_utf32(str).back());
}

}  // namespace

namespace kepub {

void create_dir(const std::filesystem::path &path) {
  if (!std::filesystem::create_directory(path)) {
    error("can not create directory: {}", path.string());
  }
}

void check_is_txt_file(const std::string &file_name) {
  check_file_exist(file_name);

  if (std::filesystem::path(file_name).extension() != ".txt") {
    error("need a txt file: {}", file_name);
  }
}

void check_file_exist(const std::string &file_name) {
  if (!(std::filesystem::exists(file_name) &&
        std::filesystem::is_regular_file(file_name))) {
    error("the file not exist: {}", file_name);
  }
}

// https://stackoverflow.com/questions/38608116/how-to-check-a-specified-string-is-a-valid-url-or-not-using-c-code/38608262
void check_is_url(const std::string &url) {
  if (!std::regex_match(
          url,
          std::regex(
              R"(^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$)"))) {
    error("not a url");
  }
}

std::string read_file_to_str(const std::string &file_name) {
  std::ifstream ifs(file_name, std::ifstream::binary);
  std::string data;

  data.resize(ifs.seekg(0, std::ifstream::end).tellg());
  ifs.seekg(0, std::ifstream::beg).read(data.data(), std::size(data));

  return data;
}

std::vector<std::string> read_file_to_vec(const std::string &file_name) {
  auto data = read_file_to_str(file_name);
  std::vector<std::string> result;
  boost::split(result, data, boost::is_any_of("\n"), boost::token_compress_on);

  for (auto &item : result) {
    item = trans_str(item);
  }

  return result;
}

void check_and_write_file(std::ofstream &ofs, std::string_view str) {
  if (!ofs) {
    error("file is not open");
  }

  ofs << str << std::flush;
}

std::string num_to_str(std::int32_t i) {
  if (i <= 0 || i >= 1000) {
    error("too many xhtml files need to be generated");
  }

  if (i < 10) {
    return "00" + std::to_string(i);
  } else if (i < 100) {
    return "0" + std::to_string(i);
  } else {
    return std::to_string(i);
  }
}

std::string num_to_chapter_name(std::int32_t i) {
  return "chapter" + num_to_str(i) + ".xhtml";
}

std::string num_to_illustration_name(std::int32_t i) {
  return "illustration" + num_to_str(i) + ".xhtml";
}

std::string processing_cmd(std::int32_t argc, char *argv[]) {
  std::string input_file;

  boost::program_options::options_description generic("Generic options");
  generic.add_options()("version,v", "print version string");
  generic.add_options()("help,h", "produce help message");

  boost::program_options::options_description config("Configuration");
  config.add_options()("connect,c", "connect chinese");
  config.add_options()("no-cover", "do not generate cover");
  config.add_options()("postscript,p", "generate postscript");
  config.add_options()("illustration,i", "generate illustration");

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
    fmt::print("Usage: {} [options] file...\n\n{}\n", argv[0], visible);
    std::exit(EXIT_SUCCESS);
  }

  if (vm.contains("version")) {
    fmt::print("{} version: {}.{}.{}\n", argv[0], KEPUB_VER_MAJOR,
               KEPUB_VER_MINOR, KEPUB_VER_PATCH);
    fmt::print("Build time: {} {}", __DATE__, __TIME__);
    std::exit(EXIT_SUCCESS);
  }

  if (!vm.contains("input-file")) {
    error("need a text file name or a url");
  }

  if (vm.contains("connect")) {
    connect_chinese = true;
  }
  if (vm.contains("no-cover")) {
    no_cover = true;
  }
  if (vm.contains("postscript")) {
    postscript = true;
  }
  if (vm.contains("illustration")) {
    illustration_num = vm["illustration"].as<std::int32_t>();
  }

  return input_file;
}

void push_back(std::vector<std::string> &texts, const std::string &str) {
  if (std::empty(str)) {
    return;
  }

  if (std::empty(texts)) {
    texts.push_back(str);
    return;
  }

  if (texts.back().ends_with("，") || str.starts_with("，") ||
      str.starts_with("。") || str.starts_with("！") || str.starts_with("？") ||
      str.starts_with("”") || str.starts_with("、") || str.starts_with("』") ||
      str.starts_with("》") || str.starts_with("】") || str.starts_with("）")) {
    texts.back().append(str);
  } else if (std::isalpha(texts.back().back()) && std::isalpha(str.front())) {
    texts.back().append(" " + str);
  } else if (end_with_chinese(texts.back()) && std::isalpha(str.front())) {
    texts.back().append(" " + str);
  } else if (std::isalpha(texts.back().back()) && start_with_chinese(str)) {
    texts.back().append(" " + str);
  } else if (connect_chinese && end_with_chinese(texts.back()) &&
             start_with_chinese(str)) {
    texts.back().append(str);
  } else {
    texts.push_back(str);
  }
}

}  // namespace kepub
