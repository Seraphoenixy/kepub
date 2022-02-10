#include "http.h"

#include <ctime>

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/hash.h>
#include <klib/util.h>
#include <boost/algorithm/string.hpp>

namespace {

const std::string authorization = "Basic YXBpdXNlcjozcyMxLXl0NmUqQWN2QHFlcg==";
const std::string device_token = "1F6EF324-A916-4995-971D-3AA71813072B";
const std::string user_agent =
    "boluobao/4.8.24(iOS;15.3)/appStore/" + device_token;
const std::string user_agent_rss =
    "SFReader/4.8.24 (iPhone; iOS 15.3; Scale/3.00)";

klib::Request request;

std::string sf_security() {
  std::string uuid = klib::uuid();
  auto timestamp = std::time(nullptr);
  std::string sign = boost::to_upper_copy(klib::md5_hex(
      uuid + std::to_string(timestamp) + device_token + "FMLxgOdsfxmN!Dt4"));

  return fmt::format(
      FMT_COMPILE("nonce={}&timestamp={}&devicetoken={}&sign={}"), uuid,
      timestamp, device_token, sign);
}

}  // namespace

klib::Response http_get(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &params) {
  request.set_no_proxy();
  request.set_user_agent(user_agent);
  request.set_accept_encoding("gzip, deflate, br");
#ifndef NDEBUG
  request.verbose(true);
#endif

  return request.get(url, params,
                     {{"Connection", "keep-alive"},
                      {"Accept", "application/vnd.sfacg.api+json;version=1"},
                      {"SFSecurity", sf_security()},
                      {"Accept-Language", "zh-Hans-CN;q=1"},
                      {"Authorization", authorization}});
}

klib::Response http_get_rss(const std::string &url) {
  request.set_no_proxy();
  request.set_user_agent(user_agent_rss);
  request.set_accept_encoding("gzip, deflate, br");
#ifndef NDEBUG
  request.verbose(true);
#endif

  return request.get(url, {},
                     {{"Connection", "keep-alive"},
                      {"Accept", "image/*,*/*;q=0.8"},
                      {"Accept-Language", "zh-CN,zh-Hans;q=0.9"}});
}

klib::Response http_post(const std::string &url, const std::string &json) {
  request.set_no_proxy();
  request.set_user_agent(user_agent);
  request.set_accept_encoding("gzip, deflate, br");
#ifndef NDEBUG
  request.verbose(true);
#endif

  return request.post(url, json,
                      {{"Connection", "keep-alive"},
                       {"Accept", "application/vnd.sfacg.api+json;version=1"},
                       {"SFSecurity", sf_security()},
                       {"Accept-Language", "zh-Hans-CN;q=1"},
                       {"Authorization", authorization}});
}
