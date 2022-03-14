#include <cstddef>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>

#include <klib/exception.h>
#include <klib/log.h>
#include <CLI/CLI.hpp>

#include "epub.h"
#include "trans.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string file_name;
  app.add_option("file", file_name, "TXT file to be processed")->required();

  bool only_check = false;
  app.add_flag("-o,--only-check", only_check,
               "Only check the content and title, do not generate epub");

  bool translation = false;
  app.add_flag("-t,--translation", translation,
               "Translate Traditional Chinese to Simplified Chinese");

  bool connect = false;
  app.add_flag("-c,--connect", connect, "Remove extra line breaks");

  bool no_compress_images = false;
  app.add_flag("--no-compress-images", no_compress_images,
               "Do not compress images");

  std::int32_t illustration_num = 0;
  app.add_option("-i,--illustration", illustration_num,
                 "Generate illustration");

  bool remove = false;
  app.add_flag(
      "-r,--remove", remove,
      "When the generation is successful, delete the TXT file and picture");

  std::string uuid;
  app.add_option("-u,--uuid", uuid, "Specify the uuid(for testing)");

  std::string datetime;
  app.add_option("-d,--datetime", datetime,
                 "Specify the datetime(for testing)");

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_txt_file(file_name);
  auto book_name =
      kepub::trans_str(std::filesystem::path(file_name).stem(), translation);
  klib::info("Book name: {}", book_name);

  kepub::Novel novel;
  novel.book_info_.name_ = book_name;
  novel.illustration_num_ = illustration_num;
  if (std::filesystem::exists("cover.jpg")) {
    novel.book_info_.cover_path_ = "cover.jpg";
  }
  for (std::int32_t i = 1;; ++i) {
    auto name = kepub::num_to_str(i) + ".jpg";
    if (std::filesystem::exists(name)) {
      novel.image_paths_.push_back(name);
    } else {
      break;
    }
  }

  kepub::Epub epub;
  epub.set_rights("Kaiser");
  epub.set_compress_images(!no_compress_images);
  // For testing
  if (!std::empty(uuid)) {
    epub.set_uuid(uuid);
  }
  if (!std::empty(datetime)) {
    epub.set_datetime(datetime);
  }

  auto vec = kepub::read_file_to_vec(file_name, translation);
  auto size = std::size(vec);

  std::string title_prefix = "[WEB] ";
  auto title_prefix_size = std::size(title_prefix);

  std::string volume_prefix = "[VOLUME] ";
  auto volume_prefix_size = std::size(volume_prefix);

  std::string author_prefix = "[AUTHOR]";
  std::string introduction_prefix = "[INTRO]";
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

      novel.book_info_.author_ = vec[i];
      klib::info("Author: {}", novel.book_info_.author_);
    } else if (vec[i].starts_with(introduction_prefix)) {
      ++i;

      for (; i < size && !is_prefix(vec[i]); ++i) {
        auto line = vec[i];
        kepub::str_check(line);

        word_count += kepub::str_size(line);
        kepub::push_back(novel.book_info_.introduction_, line, connect);
      }
      --i;
    } else if (vec[i].starts_with(postscript_prefix)) {
      ++i;

      for (; i < size && !is_prefix(vec[i]); ++i) {
        auto line = vec[i];
        kepub::str_check(line);

        word_count += kepub::str_size(line);
        kepub::push_back(novel.postscript_, line, connect);
      }
      --i;
    } else if (vec[i].starts_with(volume_prefix)) {
      auto volume_name = vec[i].substr(volume_prefix_size);
      kepub::volume_name_check(volume_name);
      novel.volumes_.push_back({"", volume_name, {}});
    } else if (vec[i].starts_with(title_prefix)) {
      auto title = vec[i].substr(title_prefix_size);
      kepub::title_check(title);
      ++i;

      std::vector<std::string> content;
      for (; i < size && !is_prefix(vec[i]); ++i) {
        auto line = vec[i];
        kepub::str_check(line);

        word_count += kepub::str_size(line);
        kepub::push_back(content, line, connect);
      }
      --i;

      if (std::empty(novel.volumes_)) {
        novel.volumes_.push_back({"", "", {}});
      }
      novel.volumes_.back().chapters_.push_back({"", title, content});
    }
  }

  klib::info("Total words: {}", word_count);

  if (only_check) {
    klib::info("Novel '{}' check operation completed", book_name);
    return EXIT_SUCCESS;
  }

  epub.set_novel(novel);
  klib::info("Start to generate epub files");
  epub.generate();

  if (remove) {
    kepub::remove_file_or_dir(file_name);

    if (!std::empty(novel.book_info_.cover_path_)) {
      kepub::remove_file_or_dir(novel.book_info_.cover_path_);
    }

    for (const auto &path : novel.image_paths_) {
      kepub::remove_file_or_dir(path);
    }
  }

  if (std::empty(uuid) && std::empty(datetime)) {
    kepub::remove_file_or_dir(book_name);
  }

  klib::info("The epub of novel '{}' was successfully generated", book_name);
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
