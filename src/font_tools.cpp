#include "font_tools.h"

#include <filesystem>

#include <Python.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <klib/error.h>

// TODO FIXME
#ifdef KEPUB_SANITIZER
#include <sanitizer/lsan_interface.h>
#endif

namespace kepub {

namespace {

class ScopedInterpreter {
 public:
  explicit ScopedInterpreter(const char *argv0) {
    program_ = Py_DecodeLocale(argv0, nullptr);
    if (program_ == nullptr) {
      klib::error(KLIB_CURR_LOC, "Cannot decode argv[0]: {}", argv0);
    }
    Py_SetProgramName(program_);

    Py_Initialize();
  }

  ~ScopedInterpreter() {
    PyMem_RawFree(program_);

    if (Py_FinalizeEx() < 0) {
      klib::error(KLIB_CURR_LOC, "Py_FinalizeEx() failed");
    }
  }

 private:
  wchar_t *program_ = nullptr;
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
  static __lsan::ScopedDisabler disabler;
#endif

  static ScopedInterpreter guard(argv0);

  auto script = fmt::format(FMT_COMPILE(R"(
import sys
sys.path.append('/usr/local/lib')
import font_tools
font_tools.to_subset_woff2('{}','{}','{}')
)"),
                            in_name, out_name, text);

  auto rc = PyRun_SimpleString(script.c_str());
  if (rc != 0) {
    klib::error(KLIB_CURR_LOC, "Failed to cut and convert font");
  }

  if (!std::filesystem::remove(in_name)) {
    klib::error(KLIB_CURR_LOC, "Cannot delete file '{}'", in_name);
  }
}

}  // namespace kepub
