#pragma once

#include <string>
#include <string_view>

namespace kepub {

// fonttools ttLib.woff2 decompress ...
// https://fontdrop.info/
void to_subset_woff2(const char *argv0, std::string_view font_file,
                     const std::string &text);

}  // namespace kepub
