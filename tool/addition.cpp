#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include <klib/archive.h>
#include <klib/exception.h>
#include <klib/log.h>
#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include "epub.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

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

  bool remove = false;
  app.add_flag("-r,--remove", remove,
               "When the generation is successful, delete the TXT file and "
               "backup epub file");

  bool no_compress = false;
  app.add_flag("--no-compress", no_compress, "Do not compress(for testing)");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);

  auto book_name = std::filesystem::path(file_name).stem().string();
  auto epub_name = book_name + ".epub";
  auto backup_epub_name = book_name + "-back-up.epub";
  auto zip_name = book_name + ".zip";
  kepub::check_file_exist(epub_name);

  klib::decompress(epub_name, book_name);

  if (!no_compress) {
    std::filesystem::rename(epub_name, backup_epub_name);
  }

  std::vector<std::tuple<std::string, std::string, std::vector<std::string>>>
      contents;

  auto vec = kepub::read_file_to_vec(file_name, translation);
  auto size = std::size(vec);

  std::string title_prefix = "[WEB] ";
  auto title_prefix_size = std::size(title_prefix);

  std::string volume_name;
  std::string volume_prefix = "[VOLUME] ";
  auto volume_prefix_size = std::size(volume_prefix);

  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with(volume_prefix)) {
      volume_name = vec[i].substr(volume_prefix_size);
      kepub::volume_name_check(volume_name);
    } else if (vec[i].starts_with(title_prefix)) {
      auto title = vec[i].substr(title_prefix_size);
      kepub::title_check(title);
      ++i;

      std::vector<std::string> content;
      for (; i < size && !(vec[i].starts_with(title_prefix) ||
                           vec[i].starts_with(volume_prefix));
           ++i) {
        auto line = vec[i];
        kepub::str_check(line);
        kepub::push_back(content, line, connect_chinese);
      }
      --i;

      contents.emplace_back(volume_name, title, content);
    }
  }

  kepub::Epub epub;
  epub.append_chapter(book_name, contents);

  if (!no_compress) {
    spdlog::info("Start to compress and generate epub files");
    klib::compress(book_name, klib::Algorithm::Zip, book_name + ".epub", false);
    kepub::remove_file_or_dir(book_name);
  }

  if (remove) {
    kepub::remove_file_or_dir(file_name);
    kepub::remove_file_or_dir(backup_epub_name);
  }
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
