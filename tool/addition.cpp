#include <filesystem>

#include "compress.h"
#include "util.h"

int main(int argc, char *argv[]) {
  auto file_name = kepub::processing_cmd(argc, argv);
  kepub::check_is_txt_file(file_name);

  auto book_name = std::filesystem::path(file_name).stem().string();
  auto epub_name = book_name + ".epub";
  auto zip_name = book_name + ".zip";
  kepub::check_file_exist(epub_name);

  std::filesystem::rename(epub_name, zip_name);

  kepub::decompress(zip_name, book_name);
  kepub::remove_file(zip_name);
}
