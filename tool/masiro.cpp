#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <klib/error.h>
#include <CLI/CLI.hpp>

#include "epub.h"
#include "trans.h"
#include "util.h"
#include "version.h"

int main(int argc, const char *argv[]) try {
  CLI::App app("masiro, ");
  app.set_version_flag("-v,--version", fmt::format("{} version: {}", argv[0],
                                                   kepub::kepub_version()));

  bool no_cover = false;
  app.add_flag("--no-cover", no_cover, "do not generate cover");

  bool generate_postscript = false;
  app.add_flag("-p,--postscript", generate_postscript, "generate postscript");

  bool connect_chinese = false;
  app.add_flag("-c,--connect", connect_chinese, "connect chinese");

  std::int32_t illustration_num = 0;
  app.add_option("-i,--illustration", illustration_num,
                 "generate illustration");

  std::int32_t image_num = 0;
  app.add_option("--image", image_num, "generate image");

  std::string uuid;
  app.add_option("--uuid", uuid, "specify the uuid(for testing)");

  std::string date;
  app.add_option("--date", date, "specify the date(for testing)");

  std::string file_name;
  app.add_option("file", file_name, "txt file")->required();

  CLI11_PARSE(app, argc, argv);

  kepub::check_is_txt_file(file_name);
  auto book_name = kepub::trans_str(std::filesystem::path(file_name).stem());

  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name(book_name);
  epub.set_generate_cover(!no_cover);
  epub.set_generate_postscript(generate_postscript);
  epub.set_illustration_num(illustration_num);
  epub.set_image_num(image_num);
  // For testing
  if (!std::empty(uuid)) {
    epub.set_uuid(uuid);
  }
  if (!std::empty(date)) {
    epub.set_date(date);
  }

  auto vec = kepub::read_file_to_vec(file_name);
  auto size = std::size(vec);

  std::string title_prefix = "[WEB] ";
  auto title_prefix_size = std::size(title_prefix);

  std::string volume_name;
  std::string volume_prefix = "[VOLUME] ";
  auto volume_prefix_size = std::size(volume_prefix);

  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with(volume_prefix)) {
      volume_name = vec[i].substr(volume_prefix_size);
      ++i;
    }

    if (vec[i].starts_with(title_prefix)) {
      auto title = vec[i].substr(title_prefix_size);
      ++i;

      std::vector<std::string> text;
      for (; i < size && !(vec[i].starts_with(title_prefix) ||
                           vec[i].starts_with(volume_prefix));
           ++i) {
        kepub::push_back(text, vec[i], connect_chinese);
      }
      --i;

      epub.add_content(volume_name, title, text);
    }
  }

  epub.generate(false);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
