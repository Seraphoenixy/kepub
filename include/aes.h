#pragma once

#include <string>

#include "config.h"

namespace kepub::ciweimao {

std::string KEPUB_PUBLIC encrypt(const std::string &str);

std::string KEPUB_PUBLIC decrypt(const std::string &str);

std::string KEPUB_PUBLIC decrypt_no_iv(const std::string &str);

std::string KEPUB_PUBLIC decrypt_no_iv(const std::string &str,
                                       const std::string &key);

}  // namespace kepub::ciweimao
