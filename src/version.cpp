#include "version.h"

#include <sys/utsname.h>

#include <cerrno>
#include <cstring>

#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/error.h>
#include <klib/version.h>
#include <spdlog/version.h>
#include <unicode/uvernum.h>
#include <CLI/Version.hpp>
#include <boost/version.hpp>
#include <pugixml.hpp>

namespace kepub {

std::string version_str() {
  std::string result;

  result +=
      fmt::format(FMT_COMPILE("kepub version {}\n"), KEPUB_VERSION_STRING);

  result += "Libraries: ";
  result += fmt::format(FMT_COMPILE("klib/{} "), KLIB_VERSION_STRING);
  result += fmt::format(FMT_COMPILE("CLI11/{} "), CLI11_VERSION);
  result += fmt::format(FMT_COMPILE("Boost/{}.{}.{} "), BOOST_VERSION / 100000,
                        BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);
  result += fmt::format(FMT_COMPILE("ICU/{}.{}.{} "), U_ICU_VERSION_MAJOR_NUM,
                        U_ICU_VERSION_MINOR_NUM, U_ICU_VERSION_PATCHLEVEL_NUM);
  result +=
      fmt::format(FMT_COMPILE("pugixml/{}.{}.{} "), PUGIXML_VERSION / 1000,
                  PUGIXML_VERSION / 10 % 100, PUGIXML_VERSION % 10);
  result += fmt::format(FMT_COMPILE("fmt/{}.{}.{} "), FMT_VERSION / 10000,
                        FMT_VERSION / 100 % 100, FMT_VERSION % 100);
  result += fmt::format(FMT_COMPILE("spdlog/{}.{}.{} "), SPDLOG_VER_MAJOR,
                        SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
  // NOTE
  result += "indicators/2.2.0\n";

  result += "Compiler: ";
#if defined(__clang_version__)
  result.append("Clang ").append(__clang_version__);
#elif defined(__GNUG__)
  result.append("GCC ").append(__VERSION__);
#else
  result.append("Unknown compiler");
#endif
  result += "\n";

  result.append("Build time: ")
      .append(__DATE__)
      .append(" ")
      .append(__TIME__)
      .append("\n");

  if (utsname name; !uname(&name)) {
    if (!std::strstr(name.version, name.sysname) ||
        !std::strstr(name.version, name.release) ||
        !std::strstr(name.version, name.machine)) {
      result += fmt::format(FMT_COMPILE("System: {} {} {} {}"), name.sysname,
                            name.release, name.version, name.machine);
    } else {
      result += name.version;
    }
  } else {
    klib::error(std::strerror(errno));
  }

  return result;
}

}  // namespace kepub
