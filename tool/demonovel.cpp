#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/error.h>
#include <CLI/CLI.hpp>

#include "epub.h"
#include "trans.h"
#include "util.h"
#include "version.h"

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string file_name;
  app.add_option("file", file_name, "TXT file to be processed")->required();

  bool connect_chinese = false;
  app.add_flag("-c,--connect", connect_chinese,
               "Remove extra line breaks between Chinese");

  bool no_cover = false;
  app.add_flag("--no-cover", no_cover, "Do not generate cover");

  std::string uuid;
  app.add_option("--uuid", uuid, "Specify the uuid(for testing)");

  std::string date;
  app.add_option("--date", date, "Specify the date(for testing)");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);
  auto book_name =
      kepub::trans_str(std::filesystem::path(file_name).stem(), true);

  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name(book_name);
  epub.set_generate_cover(!no_cover);
  // For testing
  if (!std::empty(uuid)) {
    epub.set_uuid(uuid);
  }
  if (!std::empty(date)) {
    epub.set_date(date);
  }

  auto vec = kepub::read_file_to_vec(file_name, true);
  auto size = std::size(vec);
  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with("---------------BEGIN")) {
      i += 2;
      auto title = vec[i];
      i += 2;

      std::vector<std::string> content;
      for (; i < size && !vec[i].starts_with("---------------END"); ++i) {
        auto line = vec[i];
        kepub::str_check(line);
        kepub::push_back(content, line, connect_chinese);
      }

      epub.add_content(title, content);
    }
  }

  epub.generate();
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
