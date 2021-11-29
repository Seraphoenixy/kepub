#pragma once

#include <filesystem>
#include <string>

#define KEPUB_VER_MAJOR 0

#define KEPUB_VER_MINOR 13

#define KEPUB_VER_PATCH 12

#define KEPUB_VERSION \
  (KEPUB_VER_MAJOR * 10000 + KEPUB_VER_MINOR * 100 + KEPUB_VER_PATCH)

#define KEPUB_STRINGIZE2(s) #s
#define KEPUB_STRINGIZE(s) KEPUB_STRINGIZE2(s)

#define KEPUB_VERSION_STRING       \
  KEPUB_STRINGIZE(KEPUB_VER_MAJOR) \
  "." KEPUB_STRINGIZE(KEPUB_VER_MINOR) "." KEPUB_STRINGIZE(KEPUB_VER_PATCH)

namespace kepub {

std::string version_str();

}  // namespace kepub
