#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/archive.h>
#include <klib/exception.h>
#include <klib/log.h>
#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include "epub.h"
#include "trans.h"
#include "util.h"
#include "version.h"

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string file_name;
  app.add_option("file", file_name, "TXT file to be processed")->required();

  bool only_check = false;
  app.add_flag("--only-check", only_check,
               "Only check the content and title, do not generate epub");

  bool no_check = false;
  app.add_flag("--no-check", no_check, "Do not check the content and title");

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

  bool remove = false;
  app.add_flag(
      "-r,--remove", remove,
      "When the generation is successful, delete the TXT file and picture");

  std::string uuid;
  app.add_option("--uuid", uuid, "Specify the uuid(for testing)");

  std::string date;
  app.add_option("--date", date, "Specify the date(for testing)");

  bool no_compress = false;
  app.add_flag("--no-compress", no_compress, "Do not compress(for testing)");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);
  auto book_name =
      kepub::trans_str(std::filesystem::path(file_name).stem(), translation);
  spdlog::info("Book name: {}", book_name);

  kepub::Epub epub;
  epub.set_creator("Kaiser");
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

  std::vector<std::string> postscript;
  std::string postscript_prefix = "[POST]";

  std::int32_t word_count = 0;

  auto is_prefix = [&](const std::string &line) {
    return line.starts_with(author_prefix) ||
           line.starts_with(introduction_prefix) ||
           line.starts_with(postscript_prefix) ||
           line.starts_with(title_prefix) || line.starts_with(volume_prefix);
  };

  for (std::size_t i = 0; i < size; ++i) {
    if (vec[i].starts_with(author_prefix)) {
      ++i;

      if (!std::empty(author)) {
        klib::warn("Author has been defined");
      }

      author = vec[i];
      spdlog::info("Author: {}", author);
    } else if (vec[i].starts_with(introduction_prefix)) {
      ++i;

      if (!std::empty(introduction)) {
        klib::warn("Introduction has been defined");
        introduction.clear();
      }

      for (; i < size && !is_prefix(vec[i]); ++i) {
        auto line = vec[i];

        if (!no_check) {
          kepub::str_check(line);
        }

        word_count += kepub::str_size(line);
        kepub::push_back(introduction, line, connect_chinese);
      }
      --i;
    } else if (vec[i].starts_with(postscript_prefix)) {
      ++i;

      if (!std::empty(postscript)) {
        klib::warn("Postscript has been defined");
        postscript.clear();
      }

      for (; i < size && !is_prefix(vec[i]); ++i) {
        auto line = vec[i];

        if (!no_check) {
          kepub::str_check(line);
        }

        word_count += kepub::str_size(line);
        kepub::push_back(postscript, line, connect_chinese);
      }
      --i;
    } else if (vec[i].starts_with(volume_prefix)) {
      volume_name = vec[i].substr(volume_prefix_size);

      if (!no_check) {
        kepub::volume_name_check(volume_name);
      }
    } else if (vec[i].starts_with(title_prefix)) {
      auto title = vec[i].substr(title_prefix_size);
      if (!no_check) {
        kepub::title_check(title);
      }
      ++i;

      std::vector<std::string> content;
      for (; i < size && !is_prefix(vec[i]); ++i) {
        auto line = vec[i];

        if (!no_check) {
          kepub::str_check(line);
        }

        word_count += kepub::str_size(line);
        kepub::push_back(content, line, connect_chinese);
      }
      --i;

      epub.add_content(volume_name, title, content);
    }
  }

  spdlog::info("Total words: {}", word_count);

  if (only_check) {
    spdlog::info("Novel '{}' check operation completed", book_name);
    return EXIT_SUCCESS;
  }

  if (!std::empty(author)) {
    epub.set_author(author);
  } else {
    klib::warn("No author information");
  }

  if (!std::empty(introduction)) {
    epub.set_introduction(introduction);
  } else {
    klib::warn("No introduction information");
  }

  if (generate_postscript) {
    if (!std::empty(postscript)) {
      epub.set_postscript(postscript);
    } else {
      klib::warn("No postscript information");
    }
  }

  bool postscript_done =
      !generate_postscript || (generate_postscript && !std::empty(postscript));

  epub.generate();

  bool cover_done = true;
  std::string cover_name = "cover.jpg";
  if (!no_cover) {
    if (!std::filesystem::is_regular_file(cover_name)) {
      klib::warn("Can't find cover image: {}", cover_name);
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

      if (!std::filesystem::is_regular_file(jpg_name)) {
        klib::warn("Can't find image: {}", jpg_name);
        image_done = false;
        break;
      }

      std::filesystem::copy(jpg_name, std::filesystem::path(book_name) /
                                          kepub::Epub::images_dir / jpg_name);
    }
  }

  bool book_done = !std::empty(author) && !std::empty(introduction) &&
                   postscript_done && cover_done && image_done;

  if (book_done) {
    if (remove) {
      kepub::remove_file_or_dir(file_name);
      kepub::remove_file_or_dir(cover_name);
      for (std::int32_t i = 1; i <= image_num; ++i) {
        auto jpg_name = kepub::num_to_str(i) + ".jpg";
        kepub::remove_file_or_dir(jpg_name);
      }
    }

    if (!no_compress) {
      spdlog::info("Start to compress and generate epub files");
      klib::compress(book_name, klib::Algorithm::Zip, book_name + ".epub",
                     false);
      kepub::remove_file_or_dir(book_name);
      spdlog::info("The epub of novel '{}' was successfully generated",
                   book_name);
    }
  } else {
    klib::warn("Some kind of error occurred, epub generation failed");
  }
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
