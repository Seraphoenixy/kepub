#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
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

std::string decrypt(const std::string &str) {
  const static auto key = klib::sha_256_raw(default_key);

  static std::vector<std::uint8_t> iv;
  if (std::empty(iv)) {
    iv.resize(16, 0);
  }

  return klib::aes_256_cbc_decrypt(klib::base64_decode(str), key, iv);
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

  spdlog::info("Login successful, account: {}", account);

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

  spdlog::info("Book name: {}", book_name);
  spdlog::info("Author: {}", author);

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

}  // namespace

// 100194379
int main(int argc, const char *argv[]) try {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);

  auto login_name = get_login_name();
  auto password = get_password();

  auto [account, login_token] = login(login_name, password);
  auto [book_name, author, description] =
      get_book_info(account, login_token, book_id);
  auto volume = get_book_volume(account, login_token, book_id);
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
