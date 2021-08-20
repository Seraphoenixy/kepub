#include <ctime>
#include <string>

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/error.h>
#include <klib/http.h>
#include <klib/util.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

#include "util.h"

namespace {

const std::string authorization =
    "Basic YW5kcm9pZHVzZXI6MWEjJDUxLXl0Njk7KkFjdkBxeHE=";
const std::string user_agent = "boluobao/4.7.54(android;30)/HomePage";
const std::string device_token = "AAC3B586-D131-32DE-942C-F5CCED55B45E";

std::string sf_security() {
  std::string uuid = boost::to_upper_copy(klib::uuid());
  auto timestamp = std::time(nullptr);
  std::string sign = boost::to_upper_copy(
      klib::md5(uuid + std::to_string(timestamp) + device_token + "xw3#a12-x"));

  return fmt::format(
      FMT_COMPILE("nonce={}&timestamp={}&devicetoken={}&sign={}"), uuid,
      timestamp, device_token, sign);
}

void login(const std::string &login_name, const std::string &password) {
  klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
#ifndef NDEBUG
  request.verbose(true);
#endif

  boost::json::object obj;
  obj["username"] = login_name;
  obj["password"] = password;

  auto response = request.post("https://api.sfacg.com/sessions",
                               boost::json::serialize(obj),
                               {{"Content-Type", "application/json"},
                                {"Authorization", authorization},
                                {"SFSecurity", sf_security()}});
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("Login fail: {}", response.status_code());
  }

  spdlog::info("登陆成功");
}

std::tuple<std::string, std::string, std::vector<std::string>> get_book_info(
    const std::string &book_id) {
  klib::Request request;
  request.set_no_proxy();
  request.set_user_agent(user_agent);
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(
      "https://api.sfacg.com/novels/" + book_id, {},
      {{"Authorization", authorization}, {"SFSecurity", sf_security()}});
  if (response.status_code() != klib::Response::StatusCode::Ok) {
    klib::error("Login fail: {}", response.status_code());
  }

  return {};
}

}  // namespace

// 231436
int main(int argc, const char *argv[]) {
  auto [book_id, options] = kepub::processing_cmd(argc, argv);

  auto login_name = kepub::get_login_name();
  auto password = kepub::get_password();

  login(login_name, password);
}
