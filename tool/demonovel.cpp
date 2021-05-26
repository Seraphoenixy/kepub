#include <cstddef>
#include <filesystem>

#include "epub.h"
#include "util.h"

int main(int argc, char *argv[]) {
  auto file_name = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name(std::filesystem::path(file_name).stem());

  auto vec = kepub::read_file_to_vec(file_name);
  auto size = std::size(vec);
  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with("－－－－－－－－－－－－－－－BEGIN")) {
      i += 2;
      kepub::Content content(vec[i]);
      i += 2;

      for (;
           i < size && !vec[i].starts_with("－－－－－－－－－－－－－－－END");
           ++i) {
        content.push_line(vec[i]);
      }

      epub.add_content(content);
    }
  }

  epub.generate();
}
