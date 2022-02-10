#include <cstdint>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <klib/exception.h>
#include <klib/log.h>
#include <klib/util.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "aes.h"
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

const std::string token_path = "/tmp/ciweimao";

bool show_user_info(const std::string &account,
                    const std::string &login_token) {
  auto response =
      http_post("https://app.hbooker.com/reader/get_my_info",
                {{"account", account}, {"login_token", login_token}});
  UserInfo info(decrypt_no_iv(response.text()));

  if (info.login_expired()) {
    return false;
  } else {
    klib::info("Use existing login token, nick name: {}", info.nick_name());
    return true;
  }
}

std::optional<std::pair<std::string, std::string>> try_read_token() {
  if (!std::filesystem::exists(token_path)) {
    klib::warn("Login required to access this resource");
    return {};
  }

  auto json = klib::read_file(token_path, false);
  Token token(decrypt(json));

  auto account = token.account();
  auto login_token = token.login_token();

  if (show_user_info(account, login_token)) {
    return {{account, login_token}};
  } else {
    return {};
  }
}

void write_token(const std::string &account, const std::string &login_token) {
  klib::write_file(token_path, true, encrypt(serialize(account, login_token)));
}

std::pair<std::string, std::string> login(const std::string &login_name,
                                          const std::string &password) {
  auto response = http_post("https://app.hbooker.com/signup/login",
                            {{"login_name", login_name}, {"passwd", password}});
  LoginInfo info(decrypt_no_iv(response.text()));

  klib::info("Login successful, nick name: {}", info.nick_name());
  return {info.account(), info.login_token()};
}

std::tuple<std::string, std::string, std::vector<std::string>> get_book_info(
    const std::string &account, const std::string &login_token,
    const std::string &book_id) {
  auto response = http_post("https://app.hbooker.com/book/get_info_by_id",
                            {{"account", account},
                             {"login_token", login_token},
                             {"book_id", book_id}});
  BookInfo info(decrypt_no_iv(response.text()));

  klib::info("Book name: {}", info.book_name());
  klib::info("Author: {}", info.author());
  klib::info("Cover url: {}", info.cover_url());

  std::string cover_name = "cover.jpg";
  response = http_get_rss(info.cover_url());
  response.save_to_file(cover_name);
  klib::info("Cover downloaded successfully: {}", cover_name);

  return {info.book_name(), info.author(), info.intro()};
}

std::vector<std::pair<std::string, std::string>> get_book_volume(
    const std::string &account, const std::string &login_token,
    const std::string &book_id) {
  auto response = http_post("https://app.hbooker.com/book/get_division_list",
                            {{"account", account},
                             {"login_token", login_token},
                             {"book_id", book_id}});
  return Volumes(decrypt_no_iv(response.text())).volumes();
}

std::vector<std::tuple<std::string, std::string, std::string>> get_chapters(
    const std::string &account, const std::string &login_token,
    const std::string &volume_id) {
  auto response = http_post(
      "https://app.hbooker.com/chapter/get_updated_chapter_by_division_id",
      {{"account", account},
       {"login_token", login_token},
       {"division_id", volume_id}});
  return Chapters(decrypt_no_iv(response.text())).chapters();
}

std::string get_chapter_command(const std::string &account,
                                const std::string &login_token,
                                const std::string &chapter_id) {
  auto response = http_post("https://app.hbooker.com/chapter/get_chapter_cmd",
                            {{"account", account},
                             {"login_token", login_token},
                             {"chapter_id", chapter_id}});
  return ChaptersCommand(decrypt_no_iv(response.text())).command();
}

std::vector<std::string> get_content(const std::string &account,
                                     const std::string &login_token,
                                     const std::string &chapter_id) {
  auto chapter_command = get_chapter_command(account, login_token, chapter_id);
  auto response = http_post("https://app.hbooker.com/chapter/get_cpt_ifm",
                            {{"account", account},
                             {"login_token", login_token},
                             {"chapter_id", chapter_id},
                             {"chapter_command", chapter_command}});
  auto encrypt_content_str = Content(decrypt_no_iv(response.text())).content();
  auto content_str = decrypt_no_iv(encrypt_content_str, chapter_command);

  static std::int32_t image_count = 1;
  std::vector<std::string> content;
  for (auto &line : klib::split_str(content_str, "\n")) {
    line = kepub::trim(line);

    if (line.starts_with("<img src")) {
      pugi::xml_document doc;
      doc.load_string(line.c_str());
      std::string image_url = doc.child("img").attribute("src").as_string();

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

  std::string account, login_token;
  if (auto token = try_read_token(); token.has_value()) {
    std::tie(account, login_token) = *token;
  } else {
    auto login_name = kepub::get_login_name();
    auto password = kepub::get_password();
    std::tie(account, login_token) = login(login_name, password);
    klib::cleanse(password);
    write_token(account, login_token);
  }

  auto [book_name, author, description] =
      get_book_info(account, login_token, book_id);

  klib::info("Start getting chapter information");
  std::vector<
      std::pair<std::string,
                std::vector<std::tuple<std::string, std::string, std::string>>>>
      volume_chapter;
  std::int32_t chapter_count = 0;
  for (const auto &[volume_id, volume_name] :
       get_book_volume(account, login_token, book_id)) {
    auto chapters = get_chapters(account, login_token, volume_id);
    volume_chapter.emplace_back(volume_name, chapters);
    chapter_count += std::size(chapters);
  }

  klib::info("Start downloading novel content");
  kepub::ProgressBar bar(book_name, chapter_count);
  for (auto &[volume_name, chapters] : volume_chapter) {
    for (auto &[chapter_id, chapter_title, content] : chapters) {
      bar.set_postfix_text(chapter_title);
      content =
          boost::join(get_content(account, login_token, chapter_id), "\n");
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
