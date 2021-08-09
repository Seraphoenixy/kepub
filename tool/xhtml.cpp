#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/error.h>

#include "util.h"

int main(int argc, const char *argv[]) try {
  auto [file_name, options] = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  std::vector<std::string> result;
  for (const auto &item : kepub::read_file_to_vec(file_name)) {
    kepub::push_back(result, item, options.connect_chinese_);
  }

  std::string out = std::filesystem::path(file_name).stem().string() + ".xhtml";
  std::ofstream ofs(out);
  if (!ofs) {
    klib::error("can not open: '{}'", out);
  }

  std::int32_t count = 0;
  for (const auto &item : result) {
    kepub::str_check(item);
    count += kepub::str_size(item);
    ofs << "<p>" << item << "</p>" << '\n';
  }

  std::cout << "总字数：" << count << '\n';
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
