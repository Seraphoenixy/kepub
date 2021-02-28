cmake_minimum_required(VERSION 3.19)
project(examples LANGUAGES CXX)

option(EXAMPLES_USE_ADD_SUBDIRECTORY "Use add_subdirectory" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose Release, Debug, MinSizeRel or RelWithDebInfo" FORCE)
endif()

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT ERROR)

  if(LTO_SUPPORTED)
    message(STATUS "IPO / LTO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(FATAL_ERROR "IPO / LTO not supported: ${ERROR}")
  endif()
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(mangle_compiler_flag FLAG OUTPUT)
  string(TOUPPER "HAVE_FLAG_${FLAG}" SANITIZED_FLAG)
  string(REPLACE "+" "X" SANITIZED_FLAG ${SANITIZED_FLAG})
  string(REGEX REPLACE "[^A-Za-z_0-9]" "_" SANITIZED_FLAG ${SANITIZED_FLAG})
  string(REGEX REPLACE "_+" "_" SANITIZED_FLAG ${SANITIZED_FLAG})

  set(${OUTPUT}
      ${SANITIZED_FLAG}
      PARENT_SCOPE)
endfunction()

include(CheckCXXCompilerFlag)

function(add_cxx_compiler_flag FLAG)
  mangle_compiler_flag(${FLAG} MANGLED_FLAG)
  set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${FLAG}")
  check_cxx_compiler_flag(${FLAG} ${MANGLED_FLAG}_CXX)
  set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})

  if(${MANGLED_FLAG}_CXX)
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} ${FLAG}"
        PARENT_SCOPE)
  else()
    message(
      FATAL_ERROR "Required flag '${FLAG}' is not supported by the compiler")
  endif()
endfunction()

add_cxx_compiler_flag("-Wall")
add_cxx_compiler_flag("-Wextra")
add_cxx_compiler_flag("-Wpedantic")
add_cxx_compiler_flag("-Werror")

if(EXAMPLES_USE_ADD_SUBDIRECTORY)
  message(STATUS "Use add_subdirectory")
  add_subdirectory(epub)
else()
  message(STATUS "Use find_package")
  find_package(epub REQUIRED)
endif()

file(GLOB SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
foreach(FILE_PATH ${SRC})
  get_filename_component(FILE_NAME ${FILE_PATH} NAME_WE)
  add_executable(${FILE_NAME} ${FILE_PATH})
  target_link_libraries(${FILE_NAME} PRIVATE epub::epub)

  add_executable(${FILE_NAME}-shared ${FILE_PATH})
  target_link_libraries(${FILE_NAME}-shared PRIVATE epub::epub-shared)

  add_custom_target(
    run-${FILE_NAME} ALL
    COMMAND ${FILE_NAME} -h
    COMMAND ${FILE_NAME}-shared -h
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${FILE_NAME}
    COMMENT "Run ${FILE_NAME}")

  if(NOT EXAMPLES_USE_ADD_SUBDIRECTORY)
    add_custom_target(
      run-${FILE_NAME}-pkg-config ALL
      COMMAND
        ${CMAKE_CXX_COMPILER} ${FILE_PATH} -O3 -Wall -Wextra -Wpedantic -Werror
        -o ${FILE_NAME}-pkg-config `pkg-config --libs --cflags epub`
      COMMAND ${FILE_NAME} -h
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Run ${FILE_NAME} pkg-config")
  endif()
endforeach()
