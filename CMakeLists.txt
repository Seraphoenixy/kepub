# https://github.com/gabime/spdlog/blob/v1.x/CMakeLists.txt
cmake_minimum_required(VERSION 3.19)
message(STATUS "CMake version: ${CMAKE_VERSION}")

# ---------------------------------------------------------------------------------------
# Start kepub project
# ---------------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Version)
kepub_extract_version()

project(
  kepub
  VERSION ${KEPUB_VERSION}
  LANGUAGES ASM CXX)

message(STATUS "${PROJECT_NAME} version: ${KEPUB_VERSION}")

# ---------------------------------------------------------------------------------------
# Set default build to release
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose Debug, Release, MinSizeRel or RelWithDebInfo" FORCE)
endif()
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# ---------------------------------------------------------------------------------------
# Set variables
# ---------------------------------------------------------------------------------------
set(LIBRARY lib${PROJECT_NAME})
set(EXECUTABLE "")

set(KEPUB_LIBCXX "/usr/local/lib/libc++.a")
set(KEPUB_LIBCXXABI "/usr/local/lib/libc++abi.a")

set(KEPUB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KEPUB_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# https://cmake.org/cmake/help/latest/command/file.html#glob-recurse
file(GLOB_RECURSE FONT_SRC CONFIGURE_DEPENDS "${KEPUB_SOURCE_DIR}/src/font.s")
file(GLOB_RECURSE LIBRARY_SRC CONFIGURE_DEPENDS "${KEPUB_SOURCE_DIR}/src/*.cpp")

# ---------------------------------------------------------------------------------------
# Include CMake module
# ---------------------------------------------------------------------------------------
include(CTest)
include(GNUInstallDirs)

include(Options)
include(Check)
include(ClangTidy)
include(Format)

# ---------------------------------------------------------------------------------------
# Find package
# ---------------------------------------------------------------------------------------
find_package(fmt REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n)
find_package(Boost REQUIRED COMPONENTS program_options)

# ---------------------------------------------------------------------------------------
# Compiler Options
# ---------------------------------------------------------------------------------------
include(CompilerOptions)

# ---------------------------------------------------------------------------------------
# Build font
# ---------------------------------------------------------------------------------------
file(COPY "${KEPUB_SOURCE_DIR}/font/MStiffHei PRC Black.ttf"
     DESTINATION ${KEPUB_BINARY_DIR})

add_library(font STATIC ${FONT_SRC})
target_compile_options(font PRIVATE "-Wno-unused-command-line-argument")

# ---------------------------------------------------------------------------------------
# Build static library
# ---------------------------------------------------------------------------------------
add_library(${LIBRARY} STATIC ${LIBRARY_SRC})
target_include_directories(${LIBRARY} PUBLIC "${KEPUB_SOURCE_DIR}/include")
target_link_libraries(${LIBRARY} PRIVATE font fmt::fmt ${Boost_LIBRARIES}
                                         ${ICU_LIBRARIES})

if(KEPUB_USE_LIBCXX)
  target_link_libraries(${LIBRARY} PRIVATE ${KEPUB_LIBCXX} ${KEPUB_LIBCXXABI})
endif()

set_target_properties(
  ${LIBRARY}
  PROPERTIES DEBUG_POSTFIX d
             OUTPUT_NAME ${PROJECT_NAME}
             POSITION_INDEPENDENT_CODE ON
             INTERPROCEDURAL_OPTIMIZATION FALSE)

# ---------------------------------------------------------------------------------------
# Build executable
# ---------------------------------------------------------------------------------------
add_subdirectory(tool)

# ---------------------------------------------------------------------------------------
# Build test
# ---------------------------------------------------------------------------------------
if(BUILD_TESTING AND KEPUB_BUILD_TEST)
  message(STATUS "Build test")
  add_subdirectory(test)
endif()

# ---------------------------------------------------------------------------------------
# Install target
# ---------------------------------------------------------------------------------------
include(Install)

# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#can-i-do-make-uninstall-with-cmake
configure_file("${KEPUB_SOURCE_DIR}/cmake/CMakeUninstall.cmake.in"
               "${KEPUB_BINARY_DIR}/CMakeUninstall.cmake" @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P
                                    ${KEPUB_BINARY_DIR}/CMakeUninstall.cmake)
