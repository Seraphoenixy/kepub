#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <klib/error.h>
#include <klib/http.h>
#include <klib/util.h>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

// TODO log
// #include <spdlog/spdlog.h>

#include "trans.h"
#include "util.h"

namespace {

std::string decrypt(const std::string &str) {
  const static auto key = klib::sha_256_raw("zG2nSeEfSHfvTCHy5LCcqtBbQehKNLXn");

  std::vector<std::uint8_t> iv;
  iv.resize(16, 0);

  return klib::aes_256_cbc_decrypt(klib::base64_decode(str), key, iv);
}

}  // namespace

// 100194379
int main(int argc, const char *argv[]) try {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);

  std::string user_name;
  std::cout << "user name: ";
  std::cin >> user_name;
  if (std::empty(user_name)) {
    klib::error("user name is empty");
  }

  // FIXME
  std::string password;
  std::cout << "password: ";
  std::cin >> password;
  if (std::empty(password)) {
    klib::error("password is empty");
  }

  klib::Request request;
  request.set_no_proxy();
  request.set_user_agent("Android com.kuangxiangciweimao.novel");
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get("https://app.hbooker.com/signup/login",
                              {{"app_version", "2.7.017"},
                               {"device_token", "ciweimao_client"},
                               {"login_name", user_name},
                               {"passwd", password}});
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("login fail: {}", response.status_code());
  }

  boost::json::error_code error_code;
  boost::json::monotonic_resource mr;
  auto jv = boost::json::parse(decrypt(response.text()), error_code, &mr);
  if (error_code) {
    klib::error("Json parse error: {}", error_code.message());
  }

  std::string login_token = jv.as_object()
                                .at("data")
                                .as_object()
                                .at("login_token")
                                .as_string()
                                .c_str();
  std::string account = jv.as_object()
                            .at("data")
                            .as_object()
                            .at("reader_info")
                            .as_object()
                            .at("account")
                            .as_string()
                            .c_str();

  response = request.get("https://app.hbooker.com/book/get_info_by_id",
                         {{"app_version", "2.7.017"},
                          {"device_token", "ciweimao_client"},
                          {"account", account},
                          {"login_token", login_token},
                          {"book_id", book_id}});
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("login fail: {}", response.status_code());
  }

  jv = boost::json::parse(decrypt(response.text()), error_code, &mr);
  if (error_code) {
    klib::error("Json parse error: {}", error_code.message());
  }

  std::string book_name = jv.as_object()
                              .at("data")
                              .as_object()
                              .at("book_info")
                              .as_object()
                              .at("book_name")
                              .as_string()
                              .c_str();
  std::string author = jv.as_object()
                           .at("data")
                           .as_object()
                           .at("book_info")
                           .as_object()
                           .at("author_name")
                           .as_string()
                           .c_str();

  std::string description = jv.as_object()
                                .at("data")
                                .as_object()
                                .at("book_info")
                                .as_object()
                                .at("description")
                                .as_string()
                                .c_str();
  std::vector<std::string> temp;
  boost::split(temp, description, boost::is_any_of("\n"),
               boost::token_compress_on);
  std::vector<std::string> descriptions;
  for (const auto &line : temp) {
    kepub::push_back(descriptions, kepub::trans_str(line),
                     options.connect_chinese_);
  }

  std::string cover_url = jv.as_object()
                              .at("data")
                              .as_object()
                              .at("book_info")
                              .as_object()
                              .at("cover")
                              .as_string()
                              .c_str();

  std::cout << book_name << '\n';
  std::cout << author << '\n';

  //  response = request.get(cover_url);
  //  if (response.status_code() != klib::Response::StatusCode::Ok) {
  //    klib::error("login fail: {}", response.status_code());
  //  }
  //  response.save_to_file("cover.jpg", true);

  for (const auto &line : descriptions) {
    std::cout << line << '\n';
  }

  response = request.get("https://app.hbooker.com/book/get_division_list",
                         {{"app_version", "2.7.017"},
                          {"device_token", "ciweimao_client"},
                          {"account", account},
                          {"login_token", login_token},
                          {"book_id", book_id}});
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("login fail: {}", response.status_code());
  }

  jv = boost::json::parse(decrypt(response.text()), error_code, &mr);
  if (error_code) {
    klib::error("Json parse error: {}", error_code.message());
  }

  auto division_list =
      jv.as_object().at("data").as_object().at("division_list").as_array();
  for (const auto &division : division_list) {
    std::string division_id =
        division.as_object().at("division_id").as_string().c_str();

    std::cout << division.as_object().at("division_name").as_string() << '\n';


  }
} catch (const std::exception &err) {
  klib::error(err.what());
} catch (...) {
  klib::error("Unknown exception");
}
