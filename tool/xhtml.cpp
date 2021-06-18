#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "error.h"
#include "util.h"

namespace {

std::string chapter_line(const std::string &str) {
  if (kepub::old_style) {
    return "<p class=\"calibre2\">" + str + "</p>";
  } else {
    return "<p>" + str + "</p>";
  }
}

}  // namespace

int main(int argc, char *argv[]) try {
  auto file_name = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  std::vector<std::string> result;

  for (const auto &item : kepub::read_file_to_vec(file_name)) {
    kepub::push_back(result, item);
  }

  std::string out = std::filesystem::path(file_name).stem().string() + ".xhtml";
  std::ofstream ofs(out);
  if (!ofs) {
    kepub::error("can not open: {}", out);
  }

  std::int32_t count = 0;
  for (const auto &item : result) {
    kepub::str_check(item);
    count += kepub::str_size(item);
    ofs << chapter_line(item) << '\n';
  }
  std::cout << "总字数：" << count << '\n';
} catch (const std::exception &err) {
  kepub::error(err.what());
} catch (...) {
  kepub::error("unknown exception");
}
