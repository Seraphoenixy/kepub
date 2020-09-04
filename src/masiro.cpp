#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "common.h"

int main(int argc, char *argv[]) {
  init_trans();

  auto [input_file, xhtml]{processing_cmd(argc, argv)};

  for (const auto &item : input_file) {
    auto [book_name, texts]{read_file(item)};

    if (xhtml) {
      generate_xhtml(book_name, texts);
    } else {
      create_epub_directory(book_name);

      std::vector<std::string> titles;
      auto size{std::size(texts)};
      std::int32_t count{1};
      for (std::size_t index{}; index < size; ++index) {
        if (texts[index].starts_with("[WEB] ")) {
          auto title{texts[index].substr(6)};
          titles.push_back(title);
          ++index;

          auto filename{get_chapter_filename(book_name, count)};
          ++count;

          std::ofstream ofs{filename};
          check_file_is_open(ofs, filename);

          ofs << chapter_file_begin(title);

          for (; index < size && !texts[index].starts_with("[WEB] "); ++index) {
            ofs << chapter_file_text(texts[index]);
          }
          --index;

          ofs << chapter_file_end() << std::flush;
        }
      }

      generate_content_opf(book_name, "TODO", count);
      generate_toc_ncx(book_name, titles);
    }
  }
}
