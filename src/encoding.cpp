#include "encoding.h"

#include <cstdint>

#include <unicode/ucsdet.h>
#include <unicode/utypes.h>

#include "error.h"

namespace {

bool is_ascii(const std::string &text) {
  for (auto c : text) {
    if (static_cast<std::uint8_t>(c) > 0x7F) {
      return false;
    }
  }

  return true;
}

}  // namespace

namespace kepub {

std::string detect_encoding(const std::string &text) {
  if (is_ascii(text)) {
    return "UTF-8";
  }

  UErrorCode status = U_ZERO_ERROR;
  UCharsetDetector *csd = ucsdet_open(&status);

  ucsdet_setText(csd, text.c_str(), -1, &status);
  auto ucm = ucsdet_detect(csd, &status);

  if (!ucm) {
    error("unable to determine character set");
  }

  std::string result = ucsdet_getName(ucm, &status);
  if (U_FAILURE(status)) {
    error("detect encoding error");
  }

  ucsdet_close(csd);

  return result;
}

}  // namespace kepub
