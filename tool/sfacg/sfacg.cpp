#include <exception>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/exception.h>
#include <klib/log.h>
#include <klib/util.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/core/ignore_unused.hpp>

#include "http.h"
#include "json.h"
#include "progress_bar.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

namespace {

bool show_user_info() {
  auto response = http_get("https://api.sfacg.com/user");
  UserInfo info(response.text());

  if (info.login_expired()) {
    return false;
  } else {
    klib::info("Use existing cookies, nick name: {}", info.nick_name());
    return true;
  }
}

void login(const std::string &login_name, const std::string &password) {
  auto response = http_post("https://api.sfacg.com/sessions",
                            serialize(login_name, password));
  JsonBase json_base(response.text());
  boost::ignore_unused(json_base);

  response = http_get("https://api.sfacg.com/user");
  LoginInfo info(response.text());
  klib::info("Login successful, nick name: {}", info.nick_name());
}

std::tuple<std::string, std::string, std::vector<std::string>> get_book_info(
    const std::string &book_id) {
  auto response = http_get("https://api.sfacg.com/novels/" + book_id,
                           {{"expand", "intro"}});
  BookInfo info(response.text());

  klib::info("Book name: {}", info.book_name());
  klib::info("Author: {}", info.author());
  klib::info("Point: {}", info.point());
  klib::info("Cover url: {}", info.cover_url());

  std::string cover_name = "cover.jpg";
  response = http_get_rss(info.cover_url());
  response.save_to_file(cover_name);
  klib::info("Cover downloaded successfully: {}", cover_name);

  return {info.book_name(), info.author(), info.intro()};
}

std::vector<
    std::pair<std::string,
              std::vector<std::tuple<std::string, std::string, std::string>>>>
get_volume_chapter(const std::string &book_id) {
  auto response = http_get(fmt::format(
      FMT_COMPILE("https://api.sfacg.com/novels/{}/dirs"), book_id));
  return VolumeChapter(response.text()).get_volume_chapter();
}

std::vector<std::string> get_content(const std::string &chapter_id) {
  auto response = http_get("https://api.sfacg.com/Chaps/" + chapter_id,
                           {{"chapsId", chapter_id}, {"expand", "content"}});

  auto content_str = Content(response.text()).content();

  static std::int32_t image_count = 1;
  std::vector<std::string> content;
  for (auto &line : klib::split_str(content_str, "\n")) {
    line = kepub::trim(line);

    if (line.starts_with("[img")) {
      auto begin = line.find("https");
      if (begin == std::string::npos) {
        klib::warn("Invalid image URL: {}", line);
        continue;
      }

      auto end = line.find("[/img]");
      if (end == std::string::npos) {
        klib::warn("Invalid image URL: {}", line);
        continue;
      }

      auto image_url = line.substr(begin, end - begin);

      klib::Response image;
      try {
        image = http_get_rss(image_url);
      } catch (const klib::RuntimeError &err) {
        klib::warn("{}: {}", err.what(), line);
        continue;
      }

      auto image_name = kepub::num_to_str(image_count++);
      image.save_to_file(image_name + ".jpg");

      line = "[IMAGE] " + image_name;
    }

    kepub::push_back_no_connect(content, line);
  }

  return content;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str());

  std::string book_id;
  app.add_option("book-id", book_id, "The book id of the book to be downloaded")
      ->required();

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);

  if (!show_user_info()) {
    auto login_name = kepub::get_login_name();
    auto password = kepub::get_password();
    login(login_name, password);
    klib::cleanse(password);
  }

  auto [book_name, author, description] = get_book_info(book_id);

  klib::info("Start getting chapter information");
  auto volume_chapter = get_volume_chapter(book_id);

  std::int32_t chapter_count = 0;
  for (const auto &[volume_name, chapters] : volume_chapter) {
    chapter_count += std::size(chapters);
  }

  klib::info("Start downloading novel content");
  kepub::ProgressBar bar(book_name, chapter_count);
  for (auto &[volume_name, chapters] : volume_chapter) {
    for (auto &[chapter_id, chapter_title, content] : chapters) {
      bar.set_postfix_text(chapter_title);
      content = boost::join(get_content(chapter_id), "\n");
      bar.tick();
    }
  }

  kepub::generate_txt(book_name, author, description, volume_chapter);
  klib::info("Novel '{}' download completed", book_name);
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
