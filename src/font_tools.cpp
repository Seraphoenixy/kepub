#include "font_tools.h"

#include <filesystem>

#include <Python.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/error.h>
#include <scope_guard.hpp>

// TODO FIXME
#ifdef KEPUB_SANITIZER
#include <sanitizer/lsan_interface.h>
#endif

namespace kepub {

namespace {

class ScopedInterpreter {
 public:
  ScopedInterpreter() { Py_Initialize(); }

  ~ScopedInterpreter() {
    if (Py_FinalizeEx() < 0) {
      klib::error("Py_FinalizeEx() failed");
    }
  }
};

}  // namespace

void to_subset_woff2(const char *argv0, std::string_view font_file,
                     const std::string &text) {
  auto path = std::filesystem::path(font_file);
  auto stem = path.filename().stem().string();
  std::string in_name = path.parent_path() / (stem + ".otf");
  std::string out_name = path.parent_path() / (stem + ".woff2");

  std::filesystem::rename(path, in_name);

#ifdef KEPUB_SANITIZER
  __lsan::ScopedDisabler disabler;
#endif

  auto program = Py_DecodeLocale(argv0, nullptr);
  SCOPE_EXIT { PyMem_RawFree(program); };
  if (program == nullptr) {
    klib::error("Cannot decode argv[0]: {}", argv0);
  }
  Py_SetProgramName(program);

  ScopedInterpreter guard;

  auto script = fmt::format(FMT_COMPILE(R"(
import sys
sys.path.append('/usr/local/lib')
import font_tools
font_tools.to_subset_woff2('{}','{}','{}')
)"),
                            in_name, out_name, text);

  auto rc = PyRun_SimpleString(script.c_str());
  if (rc != 0) {
    klib::error("Failed to cut and convert font");
  }

  std::filesystem::remove(in_name);
}

}  // namespace kepub
