#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/epub.h>
#include <klib/error.h>

#include "trans.h"
#include "util.h"

int main(int argc, const char *argv[]) try {
  auto [file_name, options] = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);
  auto book_name = kepub::trans_str(std::filesystem::path(file_name).stem());

  klib::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name(book_name);
  epub.set_generate_cover(!options.no_cover_);
  epub.set_generate_postscript(options.generate_postscript_);
  epub.set_illustration_num(options.illustration_num_);
  epub.set_image_num(options.image_num_);
  // For testing
  if (!std::empty(options.uuid_)) {
    epub.set_uuid(options.uuid_);
  }
  if (!std::empty(options.date_)) {
    epub.set_date(options.date_);
  }

  auto vec = kepub::read_file_to_vec(file_name);
  auto size = std::size(vec);
  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with("---------------BEGIN")) {
      i += 2;
      auto title = vec[i];
      i += 2;

      std::vector<std::string> text;
      for (; i < size && !vec[i].starts_with("---------------END"); ++i) {
        kepub::push_back(text, vec[i], options.connect_chinese_);
      }

      epub.add_content(title, text);
    }
  }

  epub.generate(false);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
