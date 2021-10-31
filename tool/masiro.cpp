#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/archive.h>
#include <klib/error.h>
#include <CLI/CLI.hpp>

#include "epub.h"
#include "trans.h"
#include "util.h"
#include "version.h"

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str(argv[0]));

  std::string file_name;
  app.add_option("file", file_name, "TXT file to be processed")->required();

  bool translation = false;
  app.add_flag("-t,--translation", translation,
               "Translate Traditional Chinese to Simplified Chinese");

  bool connect_chinese = false;
  app.add_flag("-c,--connect", connect_chinese,
               "Remove extra line breaks between Chinese");

  bool no_cover = false;
  app.add_flag("--no-cover", no_cover, "Do not generate cover");

  std::int32_t illustration_num = 0;
  app.add_option("-i,--illustration", illustration_num,
                 "Generate illustration");

  std::int32_t image_num = 0;
  app.add_option("--image", image_num,
                 "Specify the number of generated images");

  bool generate_postscript = false;
  app.add_flag("-p,--postscript", generate_postscript, "Generate postscript");

  std::string uuid;
  app.add_option("--uuid", uuid, "Specify the uuid(for testing)");

  std::string date;
  app.add_option("--date", date, "Specify the date(for testing)");

  bool do_not_remove_dir = false;
  app.add_flag("--do-not-remove-dir", do_not_remove_dir, "Do not remove dir");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);
  auto book_name =
      kepub::trans_str(std::filesystem::path(file_name).stem(), translation);

  kepub::Epub epub;
  epub.set_creator("kaiser");
  epub.set_book_name(book_name);
  epub.set_generate_cover(!no_cover);
  epub.set_illustration_num(illustration_num);
  epub.set_image_num(image_num);
  epub.set_generate_postscript(generate_postscript);
  // For testing
  if (!std::empty(uuid)) {
    epub.set_uuid(uuid);
  }
  if (!std::empty(date)) {
    epub.set_date(date);
  }

  auto vec = kepub::read_file_to_vec(file_name, translation);
  auto size = std::size(vec);

  std::string title_prefix = "[WEB] ";
  auto title_prefix_size = std::size(title_prefix);

  std::string volume_name;
  std::string volume_prefix = "[VOLUME] ";
  auto volume_prefix_size = std::size(volume_prefix);

  std::string author;
  std::string author_prefix = "[AUTHOR]";

  std::vector<std::string> introduction;
  std::string introduction_prefix = "[INTRO]";

  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with(author_prefix)) {
      ++i;

      author = vec[i];
    } else if (vec[i].starts_with(introduction_prefix)) {
      ++i;

      for (; i < size && !(vec[i].starts_with(author_prefix) ||
                           vec[i].starts_with(introduction_prefix) ||
                           vec[i].starts_with(title_prefix) ||
                           vec[i].starts_with(volume_prefix));
           ++i) {
        kepub::push_back(introduction, vec[i], connect_chinese);
      }
      --i;
    } else if (vec[i].starts_with(volume_prefix)) {
      volume_name = vec[i].substr(volume_prefix_size);
    } else if (vec[i].starts_with(title_prefix)) {
      auto title = vec[i].substr(title_prefix_size);
      ++i;

      std::vector<std::string> text;
      for (; i < size && !(vec[i].starts_with(author_prefix) ||
                           vec[i].starts_with(introduction_prefix) ||
                           vec[i].starts_with(title_prefix) ||
                           vec[i].starts_with(volume_prefix));
           ++i) {
        kepub::push_back(text, vec[i], connect_chinese);
      }
      --i;

      epub.add_content(volume_name, title, text);
    }
  }

  if (!std::empty(author)) {
    epub.set_author(author);
  }
  if (!std::empty(introduction)) {
    epub.set_introduction(introduction);
  }
  epub.generate();

  bool cover_done = true;
  if (!no_cover) {
    std::string cover_name = "cover.jpg";

    if (!std::filesystem::exists(cover_name)) {
      klib::warn("No cover");
      cover_done = false;
    } else {
      std::filesystem::copy(cover_name, std::filesystem::path(book_name) /
                                            kepub::Epub::images_dir /
                                            cover_name);
    }
  }

  bool image_done = true;
  if (image_num != 0) {
    for (std::int32_t i = 1; i <= image_num; ++i) {
      auto jpg_name = kepub::num_to_str(i) + ".jpg";

      if (!std::filesystem::exists(jpg_name)) {
        klib::warn("Incorrect number of image");
        image_done = false;
        break;
      }

      std::filesystem::copy(jpg_name, std::filesystem::path(book_name) /
                                          kepub::Epub::images_dir / jpg_name);
    }
  }

  bool compress = !std::empty(author) && !std::empty(introduction) &&
                  cover_done && image_done;

  if (!do_not_remove_dir && compress) {
    klib::compress(book_name, klib::Algorithm::Zip, book_name + ".epub", false);
    std::filesystem::remove_all(book_name);
  }
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
