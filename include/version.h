#pragma once

#include <filesystem>
#include <string>

#define KEPUB_VER_MAJOR 0

#define KEPUB_VER_MINOR 11

#define KEPUB_VER_PATCH 3

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define KEPUB_VERSION_STRING \
  "v" STRINGIZE(KEPUB_VER_MAJOR) "." STRINGIZE(KEPUB_VER_MINOR) "." STRINGIZE(KEPUB_VER_PATCH)

namespace kepub {

inline std::string kepub_version() { return KEPUB_VERSION_STRING; }

inline std::string version_str(const std::string &argv0) {
  return std::filesystem::path(argv0).filename().string() +
         " version: " + kepub_version();
}

}  // namespace kepub

#undef STRINGIZE2
#undef STRINGIZE
