#pragma once

#include <string>

#include <unicode/translit.h>

class Trans {
 public:
  Trans();
  ~Trans();

  Trans(const Trans &) = delete;
  Trans(Trans &&) = delete;
  Trans &operator=(const Trans &) = delete;
  Trans &operator=(Trans &&) = delete;

  std::string trans_str(const std::string &str);

 private:
  icu::Transliterator *trans_;
};

void clean_up();
