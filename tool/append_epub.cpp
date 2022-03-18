#include <cstddef>
#include <exception>
#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

#include <klib/archive.h>
#include <klib/exception.h>
#include <klib/log.h>
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

  bool connect = false;
  app.add_flag("-c,--connect", connect, "Remove extra line breaks");

  bool remove = false;
  app.add_flag("-r,--remove", remove,
               "When the generation is successful, delete the TXT file and "
               "backup epub file");

  std::string datetime;
  app.add_option("-d,--datetime", datetime,
                 "Specify the datetime(for testing)");

  bool testing = false;
  app.add_flag("--testing", testing, "Do not compress(for testing)");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);

  auto book_name = std::filesystem::path(file_name).stem().string();
  auto epub_name = book_name + ".epub";
  auto backup_epub_name = book_name + "-back-up.epub";
  auto zip_name = book_name + ".zip";
  kepub::check_file_exist(epub_name);

  if (std::filesystem::exists(book_name)) {
    kepub::remove_file_or_dir(book_name);
  }
  if (testing) {
    book_name.append("-test");
    klib::decompress(epub_name, book_name);
  } else {
    klib::decompress(epub_name, book_name);
    std::filesystem::rename(epub_name, backup_epub_name);
  }

  kepub::Novel novel;
  novel.book_info_.name_ = book_name;

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
      novel.volumes_.emplace_back(volume_name);
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
        kepub::push_back(content, line, connect);
      }
      --i;

      if (std::empty(novel.volumes_)) {
        novel.volumes_.emplace_back();
      }
      novel.volumes_.back().chapters_.emplace_back(title, content);
    }
  }

  kepub::Epub epub;
  // For testing
  if (!std::empty(datetime)) {
    epub.set_datetime(datetime);
  }

  epub.set_novel(novel);
  epub.append();

  if (!testing) {
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
