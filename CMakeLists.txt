cmake_minimum_required(VERSION 3.13)

project(epub LANGUAGES CXX)

if((CMAKE_BUILD_TYPE STREQUAL "Debug") OR (CMAKE_BUILD_TYPE STREQUAL
                                           "RelWithDebInfo"))
  # Address Memory Thread None
  set(SANITIZER "Address")
else()
  set(SANITIZER "None")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Check)
include(ClangTidy)
include(CompilerOptions)
include(Sanitizers)

file(GLOB_RECURSE CLANG_FORMAT_SOURCES CONFIGURE_DEPENDS include/*.h src/*.cpp)
file(GLOB_RECURSE CMAKE_FORMAT_SOURCES CONFIGURE_DEPENDS cmake/*.cmake
     CMakeLists.txt)

include_directories(include)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n)
find_package(Boost REQUIRED COMPONENTS program_options)

add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

add_library(common SHARED src/common.cpp)
target_link_libraries(common ${ICU_LIBRARIES} ${Boost_LIBRARIES}
                      fmt::fmt-header-only)

add_executable(demonovel src/demonovel.cpp)
target_link_libraries(demonovel common)

add_executable(esjzone src/esjzone.cpp)
target_link_libraries(esjzone ${CMAKE_THREAD_LIBS_INIT} common)

add_dependencies(demonovel common)
add_dependencies(esjzone common)

if((CMAKE_BUILD_TYPE MATCHES "Debug") OR (CMAKE_BUILD_TYPE MATCHES
                                          "RelWithDebInfo"))
  include(Format)
endif()

install(TARGETS demonovel RUNTIME DESTINATION bin)
install(TARGETS esjzone RUNTIME DESTINATION bin)
