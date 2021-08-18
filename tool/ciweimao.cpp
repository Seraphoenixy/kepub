#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <klib/error.h>
#include <klib/http.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

#include "trans.h"
#include "util.h"

namespace {

const std::string app_version = "2.8.101";
const std::string device_token = "ciweimao_client";
const std::string default_key = "zG2nSeEfSHfvTCHy5LCcqtBbQehKNLXn";
const std::string user_agent = "Android com.kuangxiangciweimao.novel";
constexpr std::int32_t ok = 100000;

std::string decrypt(const std::string &str) {
  static const auto decrypt_key = klib::sha_256_raw(default_key);
  static const std::vector<std::uint8_t> iv = {0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0};

  return klib::aes_256_cbc_decrypt(klib::base64_decode(str), decrypt_key, iv);
}

std::string decrypt(const std::string &str, const std::string &key) {
  static const std::vector<std::uint8_t> iv = {0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0};

  return klib::aes_256_cbc_decrypt(klib::base64_decode(str),
                                   klib::sha_256_raw(key), iv);
}

std::string get_login_name() {
  std::string login_name;

  std::cout << "login name: ";
  std::cin >> login_name;
  if (std::empty(login_name)) {
    klib::error("login name is empty");
  }

  return login_name;
}

std::string get_password() {
  std::string password = getpass("password: ");

  if (std::empty(password)) {
    klib::error("password is empty");
  }

  return password;
}

klib::Response http_get(const std::string &url,
                        const std::map<std::string, std::string> &params) {
  static klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url, params);
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("HTTP get fail: {}", response.status_code());
  }

  return response;
}

auto parse_json(const std::string &json) {
  static boost::json::error_code error_code;
  static boost::json::monotonic_resource mr;
  auto jv = boost::json::parse(json, error_code, &mr);
  if (error_code) {
    klib::error("Json parse error: {}", error_code.message());
  }

  std::string code = jv.at("code").as_string().c_str();
  if (std::stoi(code) != ok) {
    klib::error(jv.at("tip").as_string().c_str());
  }

  return jv;
}

std::pair<std::string, std::string> login(const std::string &login_name,
                                          const std::string &password) {
  auto response = http_get("https://app.hbooker.com/signup/login",
                           {{"app_version", app_version},
                            {"device_token", device_token},
                            {"login_name", login_name},
                            {"passwd", password}});
  auto jv = parse_json(decrypt(response.text()));

  std::string account =
      jv.at("data").at("reader_info").at("account").as_string().c_str();
  std::string login_token = jv.at("data").at("login_token").as_string().c_str();

  spdlog::info("登陆成功, 帐户: {}", account);

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
      kepub::trans_str(book_info.at("book_name").as_string().c_str());
  std::string author =
      kepub::trans_str(book_info.at("author_name").as_string().c_str());
  std::string description_str = book_info.at("description").as_string().c_str();

  std::vector<std::string> temp;
  boost::split(temp, description_str, boost::is_any_of("\n"),
               boost::token_compress_on);
  std::vector<std::string> description;
  for (const auto &line : temp) {
    kepub::push_back(description, kepub::trans_str(line), false);
  }

  spdlog::info("书名: {}", book_name);
  spdlog::info("作者: {}", author);

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
  auto division_list = jv.at("data").at("division_list").as_array();
  for (const auto &division : division_list) {
    result.emplace_back(division.at("division_id").as_string().c_str(),
                        division.at("division_name").as_string().c_str());
  }

  return result;
}

std::vector<std::pair<std::string, std::string>> get_chapters(
    const std::string &account, const std::string &login_token,
    const std::string &division_id) {
  auto response = http_get(
      "https://app.hbooker.com/chapter/get_updated_chapter_by_division_id",
      {{"app_version", app_version},
       {"device_token", device_token},
       {"account", account},
       {"login_token", login_token},
       {"division_id", division_id}});
  auto jv = parse_json(decrypt(response.text()));

  std::vector<std::pair<std::string, std::string>> result;
  auto chapter_list = jv.at("data").at("chapter_list").as_array();
  for (const auto &chapter : chapter_list) {
    std::string chapter_id = chapter.at("chapter_id").as_string().c_str();
    std::string chapter_title = chapter.at("chapter_title").as_string().c_str();
    std::string is_valid = chapter.at("is_valid").as_string().c_str();
    std::string auth_access = chapter.at("auth_access").as_string().c_str();

    if (is_valid != "1") {
      klib::warn("The chapter is not valid, id: {}, title: {}", chapter_id,
                 chapter_title);
    } else if (auth_access != "1") {
      klib::warn("No authorized access, id: {}, title: {}", chapter_id,
                 chapter_title);
    } else {
      result.emplace_back(chapter_id, chapter_title);
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
  std::string chapter_title =
      chapter_info.at("chapter_title").as_string().c_str();
  std::string auth_access = chapter_info.at("auth_access").as_string().c_str();

  if (auth_access != "1") {
    klib::error("No authorized access, id: {}, title: {}", chapter_id,
                chapter_title);
  }

  std::string encrypt_content_str =
      jv.at("data").at("chapter_info").at("txt_content").as_string().c_str();
  auto content_str = decrypt(encrypt_content_str, chapter_command);

  std::vector<std::string> temp;
  boost::split(temp, content_str, boost::is_any_of("\n"),
               boost::token_compress_on);
  std::vector<std::string> content;
  for (const auto &line : temp) {
    kepub::push_back(content, kepub::trans_str(line), false);
  }

  return content;
}

}  // namespace

int main(int argc, const char *argv[]) try {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);

  auto login_name = get_login_name();
  auto password = get_password();

  auto [account, login_token] = login(login_name, password);
  auto [book_name, author, description] =
      get_book_info(account, login_token, book_id);

  auto p = std::make_unique<klib::ChangeWorkingDir>("temp");

  std::vector<std::pair<std::pair<std::string, std::string>,
                        std::vector<std::pair<std::string, std::string>>>>
      volume_chapter;
  for (const auto &[volume_id, volume_name] :
       get_book_volume(account, login_token, book_id)) {
    auto chapters = get_chapters(account, login_token, volume_id);
    spdlog::info("{} get chapters ok", volume_name);

    volume_chapter.emplace_back(std::make_pair(volume_id, volume_name),
                                chapters);
  }

  for (const auto &[volume, chapters] : volume_chapter) {
    std::filesystem::create_directory(volume.second);
    klib::ChangeWorkingDir change_working_dir(volume.second);

    for (const auto &[chapter_id, chapter_title] : chapters) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(50ms);

      auto pid = fork();
      if (pid < 0) {
        klib::error("Fork error");
      } else if (pid == 0) {
        klib::write_file(
            chapter_title + ".txt", false,
            boost::join(get_content(account, login_token, chapter_id), "\n"));
        spdlog::info("{} ok", chapter_title);
        std::exit(EXIT_SUCCESS);
      }
    }
  }

  klib::wait_for_child_process();
  p.reset();

  std::ofstream book_ofs(book_name);
  book_ofs << author << "\n\n";
  for (const auto &line : description) {
    book_ofs << line << "\n";
  }
  book_ofs << "\n";

  p = std::make_unique<klib::ChangeWorkingDir>("temp");
  for (const auto &[volume, chapters] : volume_chapter) {
    klib::ChangeWorkingDir change_working_dir(volume.second);
    book_ofs << "[VOLUME] " << volume.second << "\n\n";

    for (const auto &[chapter_id, chapter_title] : chapters) {
      book_ofs << "[WEB] " << chapter_title << "\n\n";
      book_ofs << klib::read_file(chapter_title + ".txt", false) << "\n\n";
    }
  }

  p.reset();
  std::filesystem::remove_all("temp");
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
