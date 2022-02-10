#pragma once

#include <string>
#include <unordered_map>

#include <klib/http.h>

klib::Response http_get(
    const std::string &url,
    const std::unordered_map<std::string, std::string> &params = {});

klib::Response http_get_rss(const std::string &url);

klib::Response http_post(const std::string &url, const std::string &json);
