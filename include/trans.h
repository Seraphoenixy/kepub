#pragma once

#include <string>

#include <unicode/translit.h>
#include <unicode/umachine.h>

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

// https://stackoverflow.com/questions/62531882/is-there-a-way-to-detect-chinese-characters-in-c-using-boost
template <UChar32 a, UChar32 b>
class UnicodeRange {
  static_assert(a <= b, "Proper range");

 public:
  constexpr bool operator()(UChar32 x) const noexcept {
    return x >= a && x <= b;
  }
};

using UnifiedIdeographs = UnicodeRange<0x4E00, 0x9FFF>;
using UnifiedIdeographsA = UnicodeRange<0x3400, 0x4DBF>;
using UnifiedIdeographsB = UnicodeRange<0x20000, 0x2A6DF>;
using UnifiedIdeographsC = UnicodeRange<0x2A700, 0x2B73F>;
using UnifiedIdeographsD = UnicodeRange<0x2B740, 0x2B81F>;
using UnifiedIdeographsE = UnicodeRange<0x2B820, 0x2CEAF>;
using CompatibilityIdeographs = UnicodeRange<0xF900, 0xFAFF>;
using CompatibilityIdeographsSupplement = UnicodeRange<0x2F800, 0x2FA1F>;

constexpr bool is_chinese(UChar32 x) noexcept {
  return UnifiedIdeographs{}(x) || UnifiedIdeographsA{}(x) ||
         UnifiedIdeographsB{}(x) || UnifiedIdeographsC{}(x) ||
         UnifiedIdeographsD{}(x) || UnifiedIdeographsE{}(x) ||
         CompatibilityIdeographs{}(x) || CompatibilityIdeographsSupplement{}(x);
}

bool start_with_chinese(const std::string &str);

bool end_with_chinese(const std::string &str);

void clean_up();
