#pragma once

#include <cstdint>
#include <string>

#include <indicators/progress_bar.hpp>

#include "config.h"

namespace kepub {

class KEPUB_EXPORT ProgressBar {
 public:
  explicit ProgressBar(std::int32_t max_progress,
                       const std::string &postfix_text = "");

  void set_postfix_text(const std::string &postfix_text = "");
  void tick();

 private:
  indicators::ProgressBar bar_;
  std::string max_progress_str_;
};

}  // namespace kepub
