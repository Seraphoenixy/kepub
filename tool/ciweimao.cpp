#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <klib/crypto.h>
#include <klib/error.h>
#include <klib/hash_lib.h>
#include <klib/http.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <pugixml.hpp>

#include "progress_bar.h"
#include "trans.h"
#include "util.h"
#include "version.h"

namespace {

constexpr std::int32_t ok = 100000;
constexpr std::int32_t login_expired = 200100;

const std::string app_version = "2.9.100";
const std::string device_token = "ciweimao_client";
const std::string user_agent = "Android com.kuangxiangciweimao.novel";

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
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url, params);
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("HTTP GET fail: {}", response.status_code());
  }

  return response;
}

auto parse_json(const std::string &json, bool check = true) {
  static boost::json::error_code error_code;
  static boost::json::monotonic_resource mr;
  auto jv = boost::json::parse(json, error_code, &mr);
  if (error_code) {
    klib::error("Json parse error: {}", error_code.message());
  }

  if (check) {
    if (std::string code = jv.at("code").as_string().c_str();
        std::stoi(code) != ok) {
      klib::error(jv.at("tip").as_string().c_str());
    }
  }

  return jv;
}

bool show_user_info(const std::string &account,
                    const std::string &login_token) {
  auto response = http_get("https://app.hbooker.com/reader/get_my_info",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token}});
  auto jv = parse_json(decrypt(response.text()), false);

  if (std::string code = jv.at("code").as_string().c_str();
      std::stoi(code) == login_expired) {
    klib::warn(jv.at("tip").as_string().c_str());
    return false;
  } else if (std::stoi(code) != ok) {
    klib::error(jv.at("tip").as_string().c_str());
  } else {
    std::string reader_name =
        jv.at("data").at("reader_info").at("reader_name").as_string().c_str();
    spdlog::info("Use existing login token, reader name: {}", reader_name);

    return true;
  }
}

std::optional<std::pair<std::string, std::string>> try_read_token() {
  if (!std::filesystem::exists(token_path)) {
    return {};
  }

  auto json = klib::read_file(token_path, false);
  auto obj = parse_json(decrypt(json), false).as_object();

  std::string account = obj.at("account").as_string().c_str();
  std::string login_token = obj.at("login_token").as_string().c_str();

  if (show_user_info(account, login_token)) {
    return {{account, login_token}};
  } else {
    return {};
  }
}

void write_token(const std::string &account, const std::string &login_token) {
  boost::json::object obj;
  obj["account"] = account;
  obj["login_token"] = login_token;

  klib::write_file(token_path, true, encrypt(boost::json::serialize(obj)));
}

std::pair<std::string, std::string> login(const std::string &login_name,
                                          const std::string &password) {
  auto response = http_get("https://app.hbooker.com/signup/login",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"login_name", login_name},
                            {"passwd", password}});
  auto jv = parse_json(decrypt(response.text()));

  auto data = jv.at("data").as_object();
  std::string account =
      data.at("reader_info").at("account").as_string().c_str();
  std::string login_token = data.at("login_token").as_string().c_str();

  std::string reader_name =
      data.at("reader_info").at("reader_name").as_string().c_str();
  spdlog::info("Login successful, reader name: {}", reader_name);

  return {account, login_token};
}

std::tuple<std::string, std::string, std::vector<std::string>> get_book_info(
    const std::string &account, const std::string &login_token,
    const std::string &book_id) {
  auto response = http_get("https://app.hbooker.com/book/get_info_by_id",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token},
                            {"book_id", book_id}});
  auto jv = parse_json(decrypt(response.text()));

  auto book_info = jv.at("data").at("book_info");
  std::string book_name =
      kepub::trans_str(book_info.at("book_name").as_string().c_str(), false);
  std::string author =
      kepub::trans_str(book_info.at("author_name").as_string().c_str(), false);
  std::string description_str = book_info.at("description").as_string().c_str();
  std::string cover_url = book_info.at("cover").as_string().c_str();

  std::vector<std::string> description;
  for (const auto &line : klib::split_str(description_str, "\n")) {
    kepub::push_back(description, kepub::trans_str(line, false), false);
  }

  spdlog::info("Book name: {}", book_name);
  spdlog::info("Author: {}", author);
  spdlog::info("Cover url: {}", cover_url);

  std::string cover_name = "cover.jpg";
  response = http_get(cover_url);
  response.save_to_file(cover_name, true);
  spdlog::info("Cover downloaded successfully: {}", cover_name);

  return {book_name, author, description};
}

std::vector<std::pair<std::string, std::string>> get_book_volume(
    const std::string &account, const std::string &login_token,
    const std::string &book_id) {
  auto response = http_get("https://app.hbooker.com/book/get_division_list",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token},
                            {"book_id", book_id}});
  auto jv = parse_json(decrypt(response.text()));

  std::vector<std::pair<std::string, std::string>> result;

  auto volume_list = jv.at("data").at("division_list").as_array();
  for (const auto &volume : volume_list) {
    std::string volume_id = volume.at("division_id").as_string().c_str();
    std::string volume_name =
        kepub::trans_str(volume.at("division_name").as_string().c_str(), false);

    result.emplace_back(volume_id, volume_name);
  }

  return result;
}

std::vector<std::tuple<std::string, std::string, std::string>> get_chapters(
    const std::string &account, const std::string &login_token,
    const std::string &volume_id, bool download_unpurchased) {
  auto response = http_get(
      "https://app.hbooker.com/chapter/get_updated_chapter_by_division_id",
      {{"app_version", app_version},
       {"device_token", device_token},
       {"account", account},
       {"login_token", login_token},
       {"division_id", volume_id}});
  auto jv = parse_json(decrypt(response.text()));

  std::vector<std::tuple<std::string, std::string, std::string>> result;

  auto chapter_list = jv.at("data").at("chapter_list").as_array();
  for (const auto &chapter : chapter_list) {
    std::string chapter_id = chapter.at("chapter_id").as_string().c_str();
    std::string chapter_title = kepub::trans_str(
        chapter.at("chapter_title").as_string().c_str(), false);
    std::string is_valid = chapter.at("is_valid").as_string().c_str();
    std::string auth_access = chapter.at("auth_access").as_string().c_str();

    if (is_valid != "1") {
      klib::warn("The chapter is not valid, id: {}, title: {}", chapter_id,
                 chapter_title);
      continue;
    }

    if (auth_access != "1") {
      klib::warn("No authorized access, id: {}, title: {}", chapter_id,
                 chapter_title);
    }

    if (auth_access == "1" || download_unpurchased) {
      result.emplace_back(chapter_id, chapter_title, "");
    }
  }

  return result;
}

std::string get_chapter_command(const std::string &account,
                                const std::string &login_token,
                                const std::string &chapter_id) {
  auto response = http_get("https://app.hbooker.com/chapter/get_chapter_cmd",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token},
                            {"chapter_id", chapter_id}});
  auto jv = parse_json(decrypt(response.text()));

  return jv.at("data").at("command").as_string().c_str();
}

std::vector<std::string> get_content(const std::string &account,
                                     const std::string &login_token,
                                     const std::string &chapter_id) {
  auto chapter_command = get_chapter_command(account, login_token, chapter_id);
  auto response = http_get("https://app.hbooker.com/chapter/get_cpt_ifm",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"account", account},
                            {"login_token", login_token},
                            {"chapter_id", chapter_id},
                            {"chapter_command", chapter_command}});
  auto jv = parse_json(decrypt(response.text()));
  auto chapter_info = jv.at("data").at("chapter_info");

  std::string encrypt_content_str =
      chapter_info.at("txt_content").as_string().c_str();
  auto content_str = decrypt(encrypt_content_str, chapter_command);

  std::vector<std::string> content;
  for (const auto &line : klib::split_str(content_str, "\n")) {
    kepub::push_back(content, kepub::trans_str(line, false), false);
  }

  static std::int32_t image_count = 1;
  for (auto &line : content) {
    if (line.starts_with("<img src")) {
      pugi::xml_document doc;
      doc.load_string(line.c_str());

      std::string image_url = doc.child("img").attribute("src").as_string();
      boost::replace_all(image_url, "：", ":");

      auto image = http_get(image_url);
      auto image_name = kepub::num_to_str(image_count++);
      image.save_to_file(image_name + ".jpg", true);

      line = "[IMAGE] " + image_name;
    }
  }

  return content;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  CLI::App app;
  app.set_version_flag("-v,--version", kepub::version_str(argv[0]));

  std::string book_id;
  app.add_option("book-id", book_id, "The book id of the book to be downloaded")
      ->required();

  bool download_unpurchased = false;
  app.add_flag("-d,--download-unpurchased", download_unpurchased,
               "Download the beginning of the unpurchased chapter");

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

  std::vector<
      std::pair<std::string,
                std::vector<std::tuple<std::string, std::string, std::string>>>>
      volume_chapter;
  std::int32_t chapter_count = 0;
  for (const auto &[volume_id, volume_name] :
       get_book_volume(account, login_token, book_id)) {
    auto chapters =
        get_chapters(account, login_token, volume_id, download_unpurchased);
    chapter_count += std::size(chapters);

    volume_chapter.emplace_back(volume_name, chapters);
  }

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
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
