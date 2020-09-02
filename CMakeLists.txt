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

find_package(ICU REQUIRED COMPONENTS uc i18n)
find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(demonovel src/demonovel.cpp src/common.cpp)
target_link_libraries(demonovel ${ICU_LIBRARIES} ${Boost_LIBRARIES})

add_executable(esjzone src/esjzone.cpp src/common.cpp)
target_link_libraries(esjzone ${ICU_LIBRARIES} ${Boost_LIBRARIES})

add_executable(masiro src/masiro.cpp src/common.cpp)
target_link_libraries(masiro ${ICU_LIBRARIES} ${Boost_LIBRARIES})

if((CMAKE_BUILD_TYPE MATCHES "Debug") OR (CMAKE_BUILD_TYPE MATCHES
                                          "RelWithDebInfo"))
  include(Format)
endif()

install(TARGETS demonovel RUNTIME DESTINATION bin)
install(TARGETS esjzone RUNTIME DESTINATION bin)
install(TARGETS masiro RUNTIME DESTINATION bin)
