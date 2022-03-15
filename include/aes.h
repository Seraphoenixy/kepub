#pragma once

#include <string>

namespace kepub::ciweimao {

std::string encrypt(const std::string &str);

std::string decrypt(const std::string &str);

std::string decrypt_no_iv(const std::string &str);

std::string decrypt_no_iv(const std::string &str, const std::string &key);

}  // namespace kepub::ciweimao
