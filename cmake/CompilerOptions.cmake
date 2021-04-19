include(AddCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------------------
# Static link
# ---------------------------------------------------------------------------------------
add_link_options("-static-libstdc++")
add_link_options("-static-libgcc")

# ---------------------------------------------------------------------------------------
# Warning
# ---------------------------------------------------------------------------------------
add_cxx_compiler_flag("-Wall")
add_cxx_compiler_flag("-Wextra")
add_cxx_compiler_flag("-Wpedantic")
add_cxx_compiler_flag("-Werror")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_cxx_compiler_flag("-Weverything")
  add_cxx_compiler_flag("-Wno-c++98-compat-pedantic")
  add_cxx_compiler_flag("-Wno-missing-prototypes")
  add_cxx_compiler_flag("-Wno-disabled-macro-expansion")
  add_cxx_compiler_flag("-Wno-exit-time-destructors")
  add_cxx_compiler_flag("-Wno-global-constructors")
endif()

# ---------------------------------------------------------------------------------------
# Link time optimization
# ---------------------------------------------------------------------------------------
# https://github.com/ninja-build/ninja/blob/master/CMakeLists.txt
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported(
    RESULT LTO_SUPPORTED
    OUTPUT ERROR
    LANGUAGES CXX)

  if(LTO_SUPPORTED)
    message(STATUS "Link time optimization: enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(FATAL_ERROR "Link time optimization not supported: ${ERROR}")
  endif()
else()
  message(STATUS "Link time optimization: disable")
endif()

# ---------------------------------------------------------------------------------------
# libc++
# ---------------------------------------------------------------------------------------
if(KEPUB_USE_LIBCXX)
  message(STATUS "Standard library: libc++")

  add_cxx_compiler_flag("-stdlib=libc++")
  # https://blog.jetbrains.com/clion/2019/10/clion-2019-3-eap-debugger-improvements/
  if((CMAKE_BUILD_TYPE STREQUAL "Debug") OR (CMAKE_BUILD_TYPE STREQUAL
                                             "RelWithDebInfo"))
    add_cxx_compiler_flag("-fstandalone-debug")
  endif()
else()
  message(STATUS "Standard library: libstdc++")
endif()

# ---------------------------------------------------------------------------------------
# Sanitizer
# ---------------------------------------------------------------------------------------
if(KEPUB_SANITIZER)
  if(NOT (KEPUB_SANITIZER STREQUAL "Thread"))
    add_cxx_compiler_flag_no_check("-fno-omit-frame-pointer")
  endif()

  macro(append_address_sanitizer_flags)
    add_cxx_compiler_flag_no_check("-fsanitize=address")
    add_cxx_compiler_flag_no_check("-fsanitize-address-use-after-scope")
    add_cxx_compiler_flag_no_check("-fno-optimize-sibling-calls")
  endmacro()

  macro(append_undefined_sanitizer_flags)
    add_cxx_compiler_flag_no_check("-fsanitize=undefined")
    add_cxx_compiler_flag_no_check("-fno-sanitize-recover=all")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      add_cxx_compiler_flag_no_check("-fsanitize=float-divide-by-zero")
      add_cxx_compiler_flag_no_check("-fsanitize=unsigned-integer-overflow")
      add_cxx_compiler_flag_no_check("-fsanitize=implicit-conversion")
      add_cxx_compiler_flag_no_check("-fsanitize=local-bounds")
      add_cxx_compiler_flag_no_check("-fsanitize=nullability")
      add_cxx_compiler_flag_no_check(
        "-fsanitize-recover=unsigned-integer-overflow")
    endif()
  endmacro()

  if(KEPUB_SANITIZER STREQUAL "Address")
    message(STATUS "Build with AddressSanitizer")
    append_address_sanitizer_flags()
  elseif(KEPUB_SANITIZER STREQUAL "Thread")
    message(STATUS "Build with ThreadSanitizer")
    add_cxx_compiler_flag_no_check("-fsanitize=thread")
  elseif(KEPUB_SANITIZER STREQUAL "Memory")
    message(STATUS "Build with MemorySanitizer")
    add_cxx_compiler_flag_no_check("-fsanitize=memory")
    add_cxx_compiler_flag_no_check("-fsanitize-memory-track-origins")
    add_cxx_compiler_flag_no_check("-fsanitize-memory-use-after-dtor")
    add_cxx_compiler_flag_no_check("-fno-optimize-sibling-calls")
  elseif(KEPUB_SANITIZER STREQUAL "Undefined")
    message(STATUS "Build with UndefinedSanitizer")
    append_undefined_sanitizer_flags()
  elseif(KEPUB_SANITIZER STREQUAL "AddressUndefined")
    message(STATUS "Build with AddressSanitizer and UndefinedSanitizer")
    append_address_sanitizer_flags()
    append_undefined_sanitizer_flags()
  else()
    message(FATAL_ERROR "The Sanitizer is not supported: ${KEPUB_SANITIZER}")
  endif()
endif()

# ---------------------------------------------------------------------------------------
# Coverage
# ---------------------------------------------------------------------------------------
if(KEPUB_BUILD_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    add_cxx_compiler_flag("--coverage")
  else()
    add_cxx_compiler_flag("-fprofile-instr-generate")
    add_cxx_compiler_flag("-fcoverage-mapping")
  endif()
endif()
