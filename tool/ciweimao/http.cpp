#include "http.h"

#include <cstdint>

#include <klib/log.h>

namespace {

const std::string app_version = "2.9.702";
const std::string device_token = "iPhone-ADDACF06-A9DD-482B-ADF5-ADE5B97438EE";
const std::string user_agent = "HappyBook/2.9.7 (iPhone; iOS 15.3; Scale/3.00)";

klib::Request request;

void report_http_error(klib::HttpStatus status, const std::string &url) {
  klib::error("HTTP GET failed, code: {}, reason: {}, url: {}",
              static_cast<std::int32_t>(status), klib::http_status_str(status),
              url);
}

}  // namespace

klib::Response http_get_rss(const std::string &url) {
  const static std::string user_agent_rss =
      request.url_encode("刺猬猫阅读") +
      "/2.9.702 CFNetwork/1329 Darwin/21.3.0";

  request.set_no_proxy();
  request.set_user_agent(user_agent_rss);
  request.set_accept_encoding("gzip, deflate, br");
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(
      url, {}, {{"Connection", "keep-alive"}, {"Accept-Language", "zh-cn"}});

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response;
}

klib::Response http_post(const std::string &url,
                         std::unordered_map<std::string, std::string> data) {
  request.set_no_proxy();
  request.set_user_agent(user_agent);
  request.set_accept_encoding("gzip, deflate, br");
#ifndef NDEBUG
  request.verbose(true);
#endif

  data.emplace("app_version", app_version);
  data.emplace("device_token", device_token);

  auto response = request.post(
      url, data,
      {{"Connection", "keep-alive"}, {"Accept-Language", "zh-Hans-CN;q=1"}});

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    report_http_error(status, url);
  }

  return response;
}
