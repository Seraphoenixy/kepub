#pragma once

#include <string>
#include <unordered_map>

#include <klib/http.h>

klib::Response http_get_rss(const std::string &url);

klib::Response http_post(const std::string &url,
                         std::unordered_map<std::string, std::string> data);
