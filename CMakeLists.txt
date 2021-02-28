cmake_minimum_required(VERSION 3.19)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Version)
epub_extract_version()

project(
  epub
  VERSION ${EPUB_VERSION}
  LANGUAGES CXX)

message(STATUS "Build ${PROJECT_NAME}: ${EPUB_VERSION}")

include(GNUInstallDirs)

include(Options)
include(Check)
include(ClangTidy)
include(CompilerOptions)
include(Format)

find_package(fmt REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n)
find_package(Boost REQUIRED COMPONENTS program_options)

# ---------------------------------------------------------------------------------------
# Set variables
# ---------------------------------------------------------------------------------------
set(LIBRARY ${PROJECT_NAME})

file(GLOB_RECURSE LIBRARY_SRC CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

set(EXECUTABLE_TARGETS)
set(LIBRARY_TARGETS)

# ---------------------------------------------------------------------------------------
# Build static library
# ---------------------------------------------------------------------------------------
if(EPUB_BUILD_STATIC)
  message(STATUS "Build static library")
  list(APPEND LIBRARY_TARGETS ${LIBRARY})

  add_library(${LIBRARY} STATIC ${LIBRARY_SRC})
  add_library(${LIBRARY}::${LIBRARY} ALIAS ${LIBRARY})

  target_include_directories(
    ${LIBRARY} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                      "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
  target_compile_definitions(${LIBRARY} PUBLIC EPUB_COMPILED_LIB)
  target_compile_features(${LIBRARY} PUBLIC cxx_std_20)
  target_link_libraries(${LIBRARY} PUBLIC fmt::fmt ${ICU_LIBRARIES}
                                          ${Boost_LIBRARIES})

  set_target_properties(
    ${LIBRARY}
    PROPERTIES OUTPUT_NAME ${LIBRARY}
               POSITION_INDEPENDENT_CODE ON
               DEBUG_POSTFIX d)
endif()

# ---------------------------------------------------------------------------------------
# Build shared library
# ---------------------------------------------------------------------------------------
include(LTO)

if(EPUB_BUILD_SHARED)
  message(STATUS "Build shared library")
  list(APPEND LIBRARY_TARGETS ${LIBRARY}-shared)

  add_library(${LIBRARY}-shared SHARED ${LIBRARY_SRC})
  add_library(${LIBRARY}::${LIBRARY}-shared ALIAS ${LIBRARY}-shared)

  target_include_directories(
    ${LIBRARY}-shared
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
  target_compile_definitions(${LIBRARY}-shared PUBLIC EPUB_COMPILED_LIB)
  target_compile_features(${LIBRARY}-shared PUBLIC cxx_std_20)
  target_link_libraries(${LIBRARY} PUBLIC fmt::fmt ${ICU_LIBRARIES}
                                          ${Boost_LIBRARIES})

  set_target_properties(
    ${LIBRARY}-shared
    PROPERTIES OUTPUT_NAME ${LIBRARY}
               VERSION ${EPUB_VERSION}
               SOVERSION ${EPUB_VERSION_MAJOR}
               DEBUG_POSTFIX d)
endif()

# ---------------------------------------------------------------------------------------
# Build executable
# ---------------------------------------------------------------------------------------
if(EPUB_BUILD_EXECUTABLE OR EPUB_BUILD_ALL)
  message(STATUS "Build executable")

  file(GLOB EXECUTABLE_SRC CONFIGURE_DEPENDS
       "${CMAKE_CURRENT_SOURCE_DIR}/tools/*.cpp")
  foreach(FILE_PATH ${EXECUTABLE_SRC})
    get_filename_component(FILE_NAME ${FILE_PATH} NAME_WE)
    list(APPEND LIBRARY_TARGETS ${FILE_NAME})
    add_executable(${FILE_NAME} ${FILE_PATH})
    target_link_libraries(${FILE_NAME} PRIVATE ${LIBRARY})
  endforeach()
endif()

# ---------------------------------------------------------------------------------------
# Install target
# ---------------------------------------------------------------------------------------
include(Install)
