#include <stdexcept>
#include <string>

#include <klib/archive.h>
#include <klib/exception.h>
#include <klib/log.h>
#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include "epub.h"
#include "util.h"
#include "version.h"

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string dir_name;
  app.add_option("dir", dir_name, "Epub directory to be compressed")
      ->required();

  bool remove = false;
  app.add_flag("-r,--remove", remove,
               "When the generation is successful, delete the TXT file");

  CLI11_PARSE(app, argc, argv)

  kepub::check_dir_exist(dir_name);

  kepub::Epub epub;
  epub.flush_font(dir_name);

  spdlog::info("Start to compress and generate epub files");
  klib::compress(dir_name, klib::Algorithm::Zip, dir_name + ".epub", false);

  if (remove) {
    kepub::remove_file_or_dir(dir_name);
  }
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
