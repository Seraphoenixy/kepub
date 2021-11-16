#include <clocale>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include <klib/archive.h>
#include <klib/error.h>
#include <CLI/CLI.hpp>

#include "epub.h"
#include "util.h"
#include "version.h"

int main(int argc, const char *argv[]) try {
  std::setlocale(LC_ALL, "en_US.UTF-8");

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

  bool no_compress = false;
  app.add_flag("--no-compress", no_compress, "Do not compress(for testing)");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);

  auto book_name = std::filesystem::path(file_name).stem().string();
  auto epub_name = book_name + ".epub";
  auto zip_name = book_name + ".zip";
  kepub::check_file_exist(epub_name);

  klib::decompress(epub_name, book_name);

  if (!no_compress) {
    std::filesystem::rename(epub_name, book_name + "-back-up.epub");
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
    } else if (vec[i].starts_with(title_prefix)) {
      auto title = vec[i].substr(title_prefix_size);
      ++i;

      std::vector<std::string> text;
      for (; i < size && !(vec[i].starts_with(title_prefix) ||
                           vec[i].starts_with(volume_prefix));
           ++i) {
        kepub::push_back(text, vec[i], connect_chinese);
      }
      --i;

      contents.emplace_back(volume_name, title, text);
    }
  }

  kepub::Epub::append_chapter(book_name, contents);

  if (!no_compress) {
    klib::compress(book_name, klib::Algorithm::Zip, book_name + ".epub", false);
    std::filesystem::remove_all(book_name);
  }
} catch (const std::exception &err) {
  klib::error(KLIB_CURR_LOC, err.what());
} catch (...) {
  klib::error(KLIB_CURR_LOC, "Unknown exception");
}
