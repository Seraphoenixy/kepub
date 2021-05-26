#include "download.h"

#include <cstddef>

#include <curl/curl.h>

#include "error.h"

namespace {

std::size_t callback_func_std_string(void* contents, std::size_t size,
                                     std::size_t nmemb, std::string* s) {
  s->append(static_cast<char*>(contents), size * nmemb);
  return size * nmemb;
}

}  // namespace

namespace kepub {

std::string get_page(const std::string& url) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  auto http_handle = curl_easy_init();
  if (!http_handle) {
    error("curl_easy_init() error");
  }

#ifndef NDEBUG
  curl_easy_setopt(http_handle, CURLOPT_VERBOSE, 1);
#endif

  curl_easy_setopt(http_handle, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, 1);
  curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYHOST, 2);
  curl_easy_setopt(http_handle, CURLOPT_CAPATH, "/etc/ssl/certs");
  curl_easy_setopt(http_handle, CURLOPT_CAINFO,
                   "/etc/ssl/certs/ca-certificates.crt");
  curl_easy_setopt(
      http_handle, CURLOPT_USERAGENT,
      "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
      "Chrome/91.0.4456.0 Safari/537.36 Edg/91.0.845.2");

  curl_easy_setopt(http_handle, CURLOPT_NOPROXY, "*");

  std::string result;
  curl_easy_setopt(http_handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, &result);
  curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION,
                   callback_func_std_string);

  if (curl_easy_perform(http_handle) != CURLE_OK) {
    error("curl_easy_perform() error");
  }

  curl_easy_cleanup(http_handle);
  curl_global_cleanup();

  if (std::empty(result)) {
    error("get page error");
  }

  return result;
}

}  // namespace kepub
