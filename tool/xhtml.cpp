#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/error.h>
#include <CLI/CLI.hpp>
#include <pugixml.hpp>

#include "util.h"
#include "version.h"

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string file_name;
  app.add_option("file", file_name, "TXT file to be processed")->required();

  bool translation = false;
  app.add_flag("-t,--translation", translation,
               "Translate Traditional Chinese to Simplified Chinese");

  bool connect_chinese = false;
  app.add_flag("-c,--connect", connect_chinese,
               "Remove extra line breaks between Chinese");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);

  std::vector<std::string> result;
  for (const auto &item : kepub::read_file_to_vec(file_name, translation)) {
    kepub::push_back(result, item, connect_chinese);
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
