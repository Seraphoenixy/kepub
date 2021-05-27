#pragma once

#include <string>

#include <unicode/translit.h>

namespace kepub {

class Trans {
 public:
  ~Trans();

  Trans(const Trans &) = delete;
  Trans(Trans &&) = delete;
  Trans &operator=(const Trans &) = delete;
  Trans &operator=(Trans &&) = delete;

  static const Trans &get();
  [[nodiscard]] std::string trans_str(const std::string &str) const;

 private:
  Trans();

  icu::Transliterator *hant_hans_;
  icu::Transliterator *halfwidth_fullwidth_;
};

std::string trans_str(const std::string &str);

}  // namespace kepub
