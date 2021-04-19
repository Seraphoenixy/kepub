if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

# ---------------------------------------------------------------------------------------
# Architecture
# ---------------------------------------------------------------------------------------
if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
  message(STATUS "Architecture: x86_64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
  message(STATUS "Architecture: AMD64")
else()
  message(
    FATAL_ERROR
      "The architecture does not support: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

# ---------------------------------------------------------------------------------------
# System
# ---------------------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # https://kurotych.com/development/cmake_check_linux_kernel_version/
  execute_process(
    COMMAND uname -r
    OUTPUT_VARIABLE KERNEL_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "System: ${CMAKE_SYSTEM_NAME} ${KERNEL_VERSION}")
else()
  message(FATAL_ERROR "The system does not support: ${CMAKE_SYSTEM_NAME}")
endif()

# ---------------------------------------------------------------------------------------
# Generator
# ---------------------------------------------------------------------------------------
if(CMAKE_GENERATOR STREQUAL "Ninja")
  execute_process(
    COMMAND ninja --version
    OUTPUT_VARIABLE NINJA_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "CMake Generator: ${CMAKE_GENERATOR} ${NINJA_VERSION}")
else()
  message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")
  message(WARNING "CMake Generator should be Ninja")
endif()

# ---------------------------------------------------------------------------------------
# Compiler
# ---------------------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.2.0)
    message(
      FATAL_ERROR
        "GCC version must be at least 10.2.0, the current version is: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  message(
    STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0.0)
    message(
      FATAL_ERROR
        "Clang version must be at least 12.0.0, the current version is: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
  message(
    STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
else()
  message(FATAL_ERROR "The compiler does not support: ${CMAKE_CXX_COMPILER_ID}")
endif()

# ---------------------------------------------------------------------------------------
# Linker
# ---------------------------------------------------------------------------------------
execute_process(
  COMMAND ${CMAKE_LINKER} --version
  OUTPUT_VARIABLE LINKER_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE "\n" ";" LINKER_VERSION ${LINKER_VERSION})
list(GET LINKER_VERSION 0 LINKER_VERSION)

message(STATUS "Linker: ${LINKER_VERSION}")

# ---------------------------------------------------------------------------------------
# Option
# ---------------------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX AND KEPUB_USE_LIBCXX)
  message(FATAL_ERROR "GCC does not support libc++")
endif()

if(KEPUB_VALGRIND AND KEPUB_SANITIZER)
  message(FATAL_ERROR "Valgrind and sanitizer cannot be used at the same time ")
endif()

if((KEPUB_SANITIZER STREQUAL "Memory") AND CMAKE_COMPILER_IS_GNUCXX)
  message(FATAL_ERROR "GCC does not support MemorySanitizer")
endif()

if((KEPUB_SANITIZER STREQUAL "Memory" OR KEPUB_SANITIZER STREQUAL "Thread")
   AND (NOT KEPUB_USE_LIBCXX))
  message(
    FATAL_ERROR
      "When using MemorySanitizer or ThreadSanitizer, libc++ must be used")
endif()
