set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(AddCompilerFlag)

# ---------------------------------------------------------------------------------------
# lld
# ---------------------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(STATUS "Use lld")
  add_link_options("-fuse-ld=lld")
endif()

# ---------------------------------------------------------------------------------------
# Warning
# ---------------------------------------------------------------------------------------
add_cxx_compiler_flag("-Wall")
add_cxx_compiler_flag("-Wextra")
add_cxx_compiler_flag("-Wpedantic")
add_cxx_compiler_flag("-Werror")
