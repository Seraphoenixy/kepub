#include "aes.h"

#include <klib/base64.h>
#include <klib/crypto.h>
#include <klib/hash.h>

namespace {

const std::string default_key = "zG2nSeEfSHfvTCHy5LCcqtBbQehKNLXn";

}  // namespace

std::string encrypt(const std::string &str) {
  static const auto key = klib::sha256(default_key);
  return klib::fast_base64_encode(klib::aes_256_encrypt(str, key));
}

std::string decrypt(const std::string &str) {
  static const auto key = klib::sha256(default_key);
  return klib::aes_256_decrypt(klib::fast_base64_decode(str), key);
}

std::string decrypt_no_iv(const std::string &str) {
  static const auto key = klib::sha256(default_key);
  return klib::aes_256_decrypt_no_iv(klib::fast_base64_decode(str), key);
}

std::string decrypt_no_iv(const std::string &str, const std::string &key) {
  return klib::aes_256_decrypt_no_iv(klib::fast_base64_decode(str),
                                     klib::sha256(key));
}
