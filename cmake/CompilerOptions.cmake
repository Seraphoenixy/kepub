include(AddCXXFlag)

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------------------
# Machine
# ---------------------------------------------------------------------------------------
add_cxx_compiler_flag("-march=haswell")
add_cxx_compiler_flag("-mtune=haswell")

# ---------------------------------------------------------------------------------------
# Static link
# ---------------------------------------------------------------------------------------
add_cxx_linker_flag("-static-libgcc")
add_cxx_linker_flag("-static-libstdc++")

# ---------------------------------------------------------------------------------------
# lld
# ---------------------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
  execute_process(
    COMMAND ld.gold --version
    OUTPUT_VARIABLE LINKER_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ";" LINKER_VERSION ${LINKER_VERSION})
  list(GET LINKER_VERSION 0 LINKER_VERSION)
  message(STATUS "Linker: ${LINKER_VERSION}")

  add_cxx_linker_flag("-fuse-ld=gold")
else()
  execute_process(
    COMMAND ld.lld --version
    OUTPUT_VARIABLE LINKER_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Linker: ${LINKER_VERSION}")

  add_cxx_linker_flag("-fuse-ld=lld")
endif()

# ---------------------------------------------------------------------------------------
# Warning
# ---------------------------------------------------------------------------------------
add_cxx_compiler_flag("-Wall")
add_cxx_compiler_flag("-Wextra")
add_cxx_compiler_flag("-Wpedantic")
add_cxx_compiler_flag("-Werror")

# ---------------------------------------------------------------------------------------
# Link time optimization
# ---------------------------------------------------------------------------------------
# https://github.com/ninja-build/ninja/blob/master/CMakeLists.txt
if((${CMAKE_BUILD_TYPE} STREQUAL "Release") OR (${CMAKE_BUILD_TYPE} STREQUAL
                                                "MinSizeRel"))
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
# strip
# ---------------------------------------------------------------------------------------
if((${CMAKE_BUILD_TYPE} STREQUAL "Release") OR (${CMAKE_BUILD_TYPE} STREQUAL
                                                "MinSizeRel"))
  message(STATUS "Discard symbols and other data from object files")

  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_cxx_compiler_flag("-Wno-error=unused-command-line-argument")
  endif()
  add_cxx_linker_flag("-s")
endif()

# ---------------------------------------------------------------------------------------
# Sanitizer
# ---------------------------------------------------------------------------------------
if(KEPUB_SANITIZER)
  message(STATUS "Build with AddressSanitizer and UndefinedSanitizer")

  add_cxx_compiler_flag("-fsanitize=address")
  add_cxx_compiler_flag("-fsanitize-address-use-after-scope")
  add_cxx_compiler_flag("-fno-omit-frame-pointer")
  add_cxx_compiler_flag("-fno-optimize-sibling-calls")

  add_cxx_compiler_flag("-fsanitize=undefined")
  add_cxx_compiler_flag("-fno-sanitize-recover=all")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_cxx_compiler_flag("-fsanitize=float-divide-by-zero")
    add_cxx_compiler_flag("-fsanitize=unsigned-integer-overflow")
    add_cxx_compiler_flag("-fsanitize=implicit-conversion")
    add_cxx_compiler_flag("-fsanitize=local-bounds")
    add_cxx_compiler_flag("-fsanitize=nullability")
    add_cxx_compiler_flag("-fsanitize-recover=unsigned-integer-overflow")
    # FIXME
    add_cxx_compiler_flag("-fsanitize-recover=implicit-conversion")
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
