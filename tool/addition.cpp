#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <klib/archive.h>
#include <klib/epub.h>
#include <klib/error.h>

#include "util.h"

int main(int argc, const char *argv[]) try {
  auto [file_name, options] = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  auto book_name = std::filesystem::path(file_name).stem().string();
  auto epub_name = book_name + ".epub";
  auto zip_name = book_name + ".zip";
  kepub::check_file_exist(epub_name);

  klib::decompress(epub_name, book_name);
  std::filesystem::rename(epub_name, book_name + "-back-up.epub");

  std::vector<std::pair<std::string, std::vector<std::string>>> contents;

  auto vec = kepub::read_file_to_vec(file_name);
  auto size = std::size(vec);
  std::string title_prefix = "[WEB] ";
  auto title_prefix_size = std::size(title_prefix);

  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with(title_prefix)) {
      auto title = vec[i].substr(title_prefix_size);
      ++i;

      std::vector<std::string> text;
      for (; i < size && !vec[i].starts_with(title_prefix); ++i) {
        kepub::push_back(text, vec[i], options.connect_chinese_);
      }
      --i;

      contents.emplace_back(title, text);
    }
  }

  klib::Epub::append_chapter(book_name, contents);
  klib::compress(book_name, klib::Algorithm::Zip, book_name + ".epub", false);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
