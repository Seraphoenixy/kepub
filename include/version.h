#pragma once

#include <string>

#define KEPUB_VER_MAJOR 0

#define KEPUB_VER_MINOR 15

#define KEPUB_VER_PATCH 15

#define KEPUB_STRINGIZE2(s) #s
#define KEPUB_STRINGIZE(s) KEPUB_STRINGIZE2(s)

#define KEPUB_VERSION_STRING       \
  KEPUB_STRINGIZE(KEPUB_VER_MAJOR) \
  "." KEPUB_STRINGIZE(KEPUB_VER_MINOR) "." KEPUB_STRINGIZE(KEPUB_VER_PATCH)

namespace kepub {

std::string version_str();

}  // namespace kepub
