#pragma once

#include <cstdint>
#include <string>

#include <indicators/progress_bar.hpp>

namespace kepub {

class ProgressBar {
 public:
  explicit ProgressBar(const std::string &postfix_text,
                       std::int32_t max_progress);

  void set_postfix_text(const std::string &postfix_text);
  void tick();

 private:
  indicators::ProgressBar bar_;
  std::string max_progress_str_;
};

}  // namespace kepub
