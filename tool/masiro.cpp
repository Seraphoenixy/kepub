#include <cstddef>
#include <filesystem>
#include <stdexcept>

#include <klib/epub.h>

#include "error.h"
#include "trans.h"
#include "util.h"

int main(int argc, const char *argv[]) try {
  auto [file_name, options] = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  // TODO Traditional Chinese translation error
  auto book_name = kepub::trans_str(std::filesystem::path(file_name).stem(),
                                    options.trans_hant_);

  klib::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name(book_name);
  epub.set_generate_cover(!options.no_cover_);
  epub.set_generate_postscript(!options.generate_postscript_);
  epub.set_illustration_num(options.illustration_num_);
  epub.set_image_num(options.image_num_);

  // author introduction
  // uuid date

  auto vec = kepub::read_file_to_vec(file_name, options.trans_hant_);
  auto size = std::size(vec);
  std::string start = "[WEB] ";
  auto start_size = std::size(start);

  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with(start)) {
      auto title = vec[i].substr(start_size);
      ++i;

      std::vector<std::string> text;
      for (; i < size && !vec[i].starts_with(start); ++i) {
        kepub::push_back(text, vec[i], options.connect_chinese_);
      }
      --i;

      epub.add_content(title, text);
    }
  }

  epub.generate();
} catch (const std::exception &err) {
  kepub::error(err.what());
} catch (...) {
  kepub::error("unknown exception");
}
