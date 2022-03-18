#include <cstdint>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <klib/exception.h>
#include <klib/log.h>
#include <klib/unicode.h>
#include <klib/url_parse.h>
#include <klib/util.h>
#include <oneapi/tbb.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "aes.h"
#include "http.h"
#include "json.h"
#include "novel.h"
#include "progress_bar.h"
#include "util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward.hpp>
backward::SignalHandling sh;
#endif

using namespace kepub::ciweimao;

namespace {

const std::string token_path = "/tmp/ciweimao";

bool show_user_info(const Token &token) {
  auto response = http_post(
      "https://app.hbooker.com/reader/get_my_info",
      {{"account", token.account_}, {"login_token", token.login_token_}});
  auto info = json_to_user_info(decrypt_no_iv(response.text()));

  if (info.login_expired_) {
    return false;
  } else {
    klib::info("Use existing login token, nick name: {}", info.nick_name_);
    return true;
  }
}

std::optional<Token> try_read_token() {
  if (!std::filesystem::exists(token_path)) {
    klib::warn("Login required to access this resource");
    return {};
  }

  auto json = klib::read_file(token_path, false);
  auto token = get_token(decrypt(json));

  if (show_user_info(token)) {
    return token;
  } else {
    return {};
  }
}

void write_token(const Token &token) {
  klib::write_file(token_path, true,
                   encrypt(serialize(token.account_, token.login_token_)));
}

LoginInfo login(const std::string &login_name, const std::string &password) {
  auto response = http_post("https://app.hbooker.com/signup/login",
                            {{"login_name", login_name}, {"passwd", password}});
  auto info = json_to_login_info(decrypt_no_iv(response.text()));

  klib::info("Login successful, nick name: {}", info.user_info_.nick_name_);
  return info;
}

kepub::BookInfo get_book_info(const Token &token, const std::string &book_id) {
  auto response = http_post("https://app.hbooker.com/book/get_info_by_id",
                            {{"account", token.account_},
                             {"login_token", token.login_token_},
                             {"book_id", book_id}});
  auto info = ::json_to_book_info(decrypt_no_iv(response.text()));

  klib::info("Book name: {}", info.name_);
  klib::info("Author: {}", info.author_);
  klib::info("Cover url: {}", info.cover_path_);

  std::string cover_name = "cover.jpg";
  response = http_get_rss(info.cover_path_);
  response.save_to_file(cover_name);
  klib::info("Cover downloaded successfully: {}", cover_name);

  return info;
}

std::vector<kepub::Volume> get_book_volume(const Token &token,
                                           const std::string &book_id) {
  auto response = http_post("https://app.hbooker.com/book/get_division_list",
                            {{"account", token.account_},
                             {"login_token", token.login_token_},
                             {"book_id", book_id}});

  return get_volume_info(decrypt_no_iv(response.text()));
}

std::vector<kepub::Chapter> get_chapters(const Token &token,
                                         std::uint64_t volume_id) {
  auto response = http_post(
      "https://app.hbooker.com/chapter/get_updated_chapter_by_division_id",
      {{"account", token.account_},
       {"login_token", token.login_token_},
       {"division_id", std::to_string(volume_id)}});

  return get_chapter_info(decrypt_no_iv(response.text()));
}

std::string get_chapter_command(const Token &token,
                                const std::string &chapter_id) {
  auto response = http_post("https://app.hbooker.com/chapter/get_chapter_cmd",
                            {{"account", token.account_},
                             {"login_token", token.login_token_},
                             {"chapter_id", chapter_id}});

  return ::get_chapter_command(decrypt_no_iv(response.text()));
}

std::vector<std::string> get_content(const Token &token,
                                     std::uint64_t chapter_id) {
  const auto id = std::to_string(chapter_id);
  auto chapter_command = get_chapter_command(token, id);
  auto response = http_post("https://app.hbooker.com/chapter/get_cpt_ifm",
                            {{"account", token.account_},
                             {"login_token", token.login_token_},
                             {"chapter_id", id},
                             {"chapter_command", chapter_command}});
  auto encrypt_content_str =
      json_to_chapter_text(decrypt_no_iv(response.text()));
  auto content_str = decrypt_no_iv(encrypt_content_str, chapter_command);

  std::vector<std::string> content;
  for (auto &line : klib::split_str(content_str, "\n")) {
    klib::trim(line);

    if (line.starts_with("<img src")) {
      pugi::xml_document doc;
      doc.load_string(line.c_str());
      std::string image_url = doc.child("img").attribute("src").as_string();

      klib::URL url(image_url);
      auto image_name = std::filesystem::path(url.path()).filename().string();

      klib::Response image;
      try {
        image = http_get_rss(image_url);
        image.save_to_file(image_name);
      } catch (const klib::RuntimeError &err) {
        klib::warn("{}: {}", err.what(), line);
        continue;
      }

      line = "[IMAGE] " + image_name;
    }

    kepub::push_back(content, line);
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

  auto hardware_concurrency = std::thread::hardware_concurrency();
  std::uint32_t max_concurrency = 0;
  app.add_option("-m,--multithreading", max_concurrency,
                 "Maximum number of concurrency to use when downloading")
      ->check(
          CLI::Range(1U, hardware_concurrency > 2 ? hardware_concurrency : 2))
      ->default_val(2);

  CLI11_PARSE(app, argc, argv)

  kepub::check_is_book_id(book_id);

  klib::info("Max concurrency: {}", max_concurrency);

  Token token;
  if (auto may_token = try_read_token(); may_token.has_value()) {
    token = *may_token;
  } else {
    auto login_name = kepub::get_login_name();
    auto password = kepub::get_password();
    token = login(login_name, password).token_;
    klib::cleanse(password);
    write_token(token);
  }

  auto book_info = get_book_info(token, book_id);

  klib::info("Start getting chapter information");
  auto volumes = get_book_volume(token, book_id);

  tbb::task_arena limited(max_concurrency);
  tbb::task_group tg;

  std::int32_t chapter_count = 0;

  limited.execute([&] {
    tg.run([&] {
      tbb::parallel_for_each(volumes, [&](kepub::Volume &volume) {
        auto chapters = get_chapters(token, volume.volume_id_);
        chapter_count += std::size(chapters);
        volume.chapters_ = std::move(chapters);
      });
    });
  });
  limited.execute([&] { tg.wait(); });

  klib::info("Start downloading novel content");
  kepub::ProgressBar bar(chapter_count, book_info.name_);

  for (auto &volume : volumes) {
    limited.execute([&] {
      tg.run([&] {
        tbb::parallel_for_each(volume.chapters_, [&](kepub::Chapter &chapter) {
          bar.set_postfix_text(chapter.title_);
          chapter.texts_ = get_content(token, chapter.chapter_id_);
          bar.tick();
        });
      });
    });
    limited.execute([&] { tg.wait(); });
  }

  kepub::generate_txt(book_info, volumes);
  klib::info("Novel '{}' download completed", book_info.name_);
} catch (const klib::Exception &err) {
  klib::error(err.what());
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
