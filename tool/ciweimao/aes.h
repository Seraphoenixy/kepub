#pragma once

#include <string>

std::string encrypt(const std::string &str);

std::string decrypt(const std::string &str);

std::string decrypt_no_iv(const std::string &str);

std::string decrypt_no_iv(const std::string &str, const std::string &key);
