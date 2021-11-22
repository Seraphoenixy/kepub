#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <klib/crypto.h>
#include <klib/error.h>
#include <klib/hash.h>
#include <klib/http.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include "json.h"
#include "progress_bar.h"
#include "trans.h"
#include "util.h"
#include "version.h"

namespace {

const std::string app_version = "2.9.100";
const std::string device_token = "iPhone-ADDACF06-A9DD-482B-ADF5-ADE5B97438EE";
const std::string user_agent = "HappyBook/2.9.1 (iPhone; iOS 15.1; Scale/3.00)";

const std::string default_key = "zG2nSeEfSHfvTCHy5LCcqtBbQehKNLXn";

const std::string token_path = std::string(std::getenv("HOME")) + "/.ciweimao";

std::string encrypt(const std::string &str) {
  static const auto key = klib::sha_256(default_key);
  return klib::aes_256_encrypt_base64(str, key, false);
}

std::string decrypt(const std::string &str) {
  static const auto key = klib::sha_256(default_key);
  return klib::aes_256_decrypt_base64(str, key, false);
}

std::string decrypt(const std::string &str, const std::string &key) {
  return klib::aes_256_decrypt_base64(str, klib::sha_256(key), false);
}

klib::Response http_get(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &params = {}) {
  static klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
  request.set_accept_encoding("gzip, deflate");
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(
      url, params,
      {{"Connection", "keep-alive"}, {"Accept-Language", "zh-Hans-CN;q=1"}});
  if (!response.ok()) {
    klib::error(KLIB_CURR_LOC, "HTTP GET fail: {}", response.status_code());
  }

  return response;
}

klib::Response http_post(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &data) {
  static klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
  request.set_accept_encoding("gzip, deflate");
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.post(
      url, data,
      {{"Connection", "keep-alive"}, {"Accept-Language", "zh-Hans-CN;q=1"}});
  if (!response.ok()) {
    klib::error(KLIB_CURR_LOC, "HTTP POST fail: {}", response.status_code());
  }

  return response;
}

bool show_user_info(const std::string &account,
                    const std::string &login_token) {
  auto response = http_post("https://app.hbooker.com/reader/get_my_info",
                            {{"app_version", app_version},
                             {"device_token", device_token},
                             {"account", account},
                             {"login_token", login_token}});
  UserInfo info(decrypt(response.text()));

  if (info.login_expired()) {
    return false;
  } else {
    spdlog::info("Use existing login token, nick name: {}", info.nick_name());
    return true;
  }
}

std::optional<std::pair<std::string, std::string>> try_read_token() {
  if (!std::filesystem::exists(token_path)) {
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
                            {{"app_version", app_version},
                             {"device_token", device_token},
                             {"login_name", login_name},
                             {"passwd", password}});
  LoginInfo info(decrypt(response.text()));

  spdlog::info("Login successful, nick name: {}", info.nick_name());
  return {info.account(), info.login_token()};
}

std::tuple<std::string, std::string, std::vector<std::string>> get_book_info(
    const std::string &account, const std::string &login_token,
    const std::string &book_id) {
  auto response = http_post("https://app.hbooker.com/book/get_info_by_id",
                            {{"app_version", app_version},
                             {"device_token", device_token},
                             {"account", account},
                             {"login_token", login_token},
                             {"book_id", book_id}});
  BookInfo info(decrypt(response.text()));

  spdlog::info("Book name: {}", info.book_name());
  spdlog::info("Author: {}", info.author());
  spdlog::info("Cover url: {}", info.cover_url());

  std::string cover_name = "cover.jpg";
  response = http_get(info.cover_url());
  response.save_to_file(cover_name, true);
  spdlog::info("Cover downloaded successfully: {}", cover_name);

  return {info.book_name(), info.author(), info.intro()};
}

std::vector<std::pair<std::string, std::string>> get_book_volume(
    const std::string &account, const std::string &login_token,
    const std::string &book_id) {
  auto response = http_post("https://app.hbooker.com/book/get_division_list",
                            {{"app_version", app_version},
                             {"device_token", device_token},
                             {"account", account},
                             {"login_token", login_token},
                             {"book_id", book_id}});
  return Volumes(decrypt(response.text())).volumes();
}

std::vector<std::tuple<std::string, std::string, std::string>> get_chapters(
    const std::string &account, const std::string &login_token,
    const std::string &volume_id) {
  auto response = http_post(
      "https://app.hbooker.com/chapter/get_updated_chapter_by_division_id",
      {{"app_version", app_version},
       {"device_token", device_token},
       {"account", account},
       {"login_token", login_token},
       {"division_id", volume_id}});
  return Chapters(decrypt(response.text())).chapters();
}

std::string get_chapter_command(const std::string &account,
                                const std::string &login_token,
                                const std::string &chapter_id) {
  auto response = http_post("https://app.hbooker.com/chapter/get_chapter_cmd",
                            {{"app_version", app_version},
                             {"device_token", device_token},
                             {"account", account},
                             {"login_token", login_token},
                             {"chapter_id", chapter_id}});
  return ChaptersCommand(decrypt(response.text())).command();
}

std::vector<std::string> get_content(const std::string &account,
                                     const std::string &login_token,
                                     const std::string &chapter_id) {
  auto chapter_command = get_chapter_command(account, login_token, chapter_id);
  auto response = http_post("https://app.hbooker.com/chapter/get_cpt_ifm",
                            {{"app_version", app_version},
                             {"device_token", device_token},
                             {"account", account},
                             {"login_token", login_token},
                             {"chapter_id", chapter_id},
                             {"chapter_command", chapter_command}});
  auto encrypt_content_str = Content(decrypt(response.text())).content();
  auto content_str = decrypt(encrypt_content_str, chapter_command);

  static std::int32_t image_count = 1;
  std::vector<std::string> content;
  for (auto &line : klib::split_str(content_str, "\n")) {
    line = kepub::trans_str(line, false);

    if (line.starts_with("<img src")) {
      pugi::xml_document doc;
      doc.load_string(line.c_str());

      std::string image_url = doc.child("img").attribute("src").as_string();
      boost::replace_all(image_url, "：", ":");

      auto image = http_get(image_url);
      auto image_name = kepub::num_to_str(image_count++);
      image.save_to_file(image_name + ".jpg", true);

      line = "[IMAGE] " + image_name;

      content.push_back(line);
    } else {
      kepub::push_back(content, line, false);
    }
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

  spdlog::info("Get chapter information");
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

  spdlog::info("Start download");
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
  spdlog::info("{} download complete", book_name);
} catch (const std::exception &err) {
  klib::error(KLIB_CURR_LOC, err.what());
} catch (...) {
  klib::error(KLIB_CURR_LOC, "Unknown exception");
}
