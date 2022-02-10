#pragma once

#include <string>

#include <klib/http.h>

klib::Response http_get(const std::string &url, const std::string &proxy);
