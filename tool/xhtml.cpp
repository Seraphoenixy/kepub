#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/error.h>
#include <pugixml.hpp>

#include "util.h"

int main(int argc, const char *argv[]) try {
  auto [file_name, options] = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  std::vector<std::string> result;
  for (const auto &item : kepub::read_file_to_vec(file_name)) {
    kepub::push_back(result, item, options.connect_chinese_);
  }

  pugi::xml_document doc;
  std::int32_t count = 0;
  for (const auto &item : result) {
    kepub::str_check(item);
    count += kepub::str_size(item);
    doc.append_child("p").text() = item.c_str();
  }

  std::cout << "总字数：" << count << '\n';

  std::string out = std::filesystem::path(file_name).stem().string() + ".xhtml";
  doc.save_file(out.c_str(), "    ",
                pugi::format_default | pugi::format_no_declaration);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
