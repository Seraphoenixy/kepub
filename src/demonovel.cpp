#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "common.h"

std::pair<std::string, std::vector<std::string>>
read_file(const std::string &filename) {
  if (std::filesystem::path{filename}.filename().extension().string() !=
      ".txt") {
    error("must be a txt file: {}", filename);
  }

  std::ifstream ifs{filename};
  check_file_is_open(ifs, filename);

  std::vector<std::string> texts;
  std::string line;

  while (std::getline(ifs, line)) {
    auto str{trans_str(line)};
    if (!std::empty(str)) {
      texts.push_back(str);
    }
  }

  auto book_name{std::filesystem::path{filename}.filename().stem().string()};
  book_name = trans_str(book_name);

  return {book_name, texts};
}

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
        if (texts[index].starts_with("－－－－－－－－－－－－－－－BEGIN")) {
          index += 2;
          auto title{texts[index]};
          titles.push_back(title);
          index += 2;

          auto filename{get_chapter_filename(book_name, count)};
          ++count;

          std::ofstream ofs{filename};
          check_file_is_open(ofs, filename);

          ofs << chapter_file_begin(title);

          for (; index < size &&
                 !texts[index].starts_with("－－－－－－－－－－－－－－－END");
               ++index) {
            ofs << chapter_file_text(texts[index]);
          }

          ofs << chapter_file_end() << std::flush;
        }
      }

      generate_content_opf(book_name, texts[1], count);
      generate_toc_ncx(book_name, titles);
    }
  }
}
