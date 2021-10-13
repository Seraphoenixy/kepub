#include "encoding.h"

#include <klib/error.h>
#include <klib/unicode.h>
#include <unicode/ucsdet.h>
#include <unicode/utypes.h>

#include "util.h"

namespace kepub {

// https://unicode-org.github.io/icu/userguide/conversion/detection.html#charsetdetector
std::string detect_encoding(const std::string &text) {
  if (klib::is_ascii(text)) {
    return "UTF-8";
  }

  UErrorCode status = U_ZERO_ERROR;

  auto csd = ucsdet_open(&status);
  check_icu(status);

  ucsdet_setText(csd, text.c_str(), -1, &status);
  check_icu(status);

  auto ucm = ucsdet_detect(csd, &status);
  check_icu(status);

  std::string result = ucsdet_getName(ucm, &status);
  check_icu(status);

  auto confidence = ucsdet_getConfidence(ucm, &status);
  check_icu(status);

  if (confidence < 95) {
    klib::warn("Confidence is less than 95%: {}%", confidence);
  }

  ucsdet_close(csd);

  return result;
}

}  // namespace kepub
