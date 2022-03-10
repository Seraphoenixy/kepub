#include "progress_bar.h"

#include <klib/util.h>

namespace kepub {

using namespace indicators;

ProgressBar::ProgressBar(const std::string &postfix_text,
                         std::int32_t max_progress)
    : bar_(option::BarWidth(klib::terminal_size().first / 3),
           option::Start("["), option::Fill("="), option::Lead(">"),
           option::Remainder(" "), option::End("]"),
           option::PostfixText(postfix_text),
           option::ForegroundColor(Color::green),
           option::FontStyles(std::vector<FontStyle>{FontStyle::bold}),
           option::ShowPercentage(false), option::MaxProgress(max_progress)),
      max_progress_str_(std::to_string(max_progress)) {}

void ProgressBar::set_postfix_text(const std::string &postfix_text) {
  static std::int32_t count = 1;
  auto str = std::to_string(count++) + "/" + max_progress_str_;
  bar_.set_option(option::PostfixText(str + " " + postfix_text));
}

void ProgressBar::tick() { bar_.tick(); }

}  // namespace kepub
