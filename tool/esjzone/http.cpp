#include "http.h"

#include <klib/log.h>

namespace {

klib::Request request;

}  // namespace

klib::Response http_get(const std::string &url, const std::string &proxy) {
  request.set_browser_user_agent();
  if (!std::empty(proxy)) {
    request.set_proxy(proxy);
  } else {
    request.set_no_proxy();
  }
#ifndef NDEBUG
  request.verbose(true);
#endif

  auto response = request.get(url);

  auto status = response.status();
  if (status != klib::HttpStatus::HTTP_STATUS_OK) {
    klib::error("HTTP GET failed, code: {}, reason: {}, url: {}",
                static_cast<std::int32_t>(status),
                klib::http_status_str(status), url);
  }

  return response;
}
