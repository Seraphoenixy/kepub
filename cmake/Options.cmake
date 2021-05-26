option(KEPUB_BUILD_TEST "Build test" OFF)

option(KEPUB_FORMAT "Format code using clang-format and cmake-format" OFF)
option(KEPUB_CLANG_TIDY "Analyze code with clang-tidy" OFF)
option(KEPUB_SANITIZER "Build with AddressSanitizer and UndefinedSanitizer" OFF)

include(CMakeDependentOption)
cmake_dependent_option(KEPUB_BUILD_COVERAGE "Build with coverage information"
                       OFF "BUILD_TESTING;KEPUB_BUILD_TEST" OFF)
cmake_dependent_option(KEPUB_VALGRIND "Execute test with valgrind" OFF
                       "BUILD_TESTING;KEPUB_BUILD_TEST" OFF)
