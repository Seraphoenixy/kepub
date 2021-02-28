if(NOT DEFINED EPUB_MASTER_PROJECT)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(EPUB_MASTER_PROJECT ON)
  else()
    set(EPUB_MASTER_PROJECT OFF)
  endif()
endif()

option(EPUB_BUILD_STATIC "Build static library" ON)
option(EPUB_BUILD_SHARED "Build shared library" ON)

option(EPUB_BUILD_ALL
       "Build all executable, tests, benchmarks, documentations and coverage"
       OFF)
option(EPUB_BUILD_EXECUTABLE "Build executable" ${EPUB_MASTER_PROJECT})

option(EPUB_FORMAT "Format code using clang-format and cmake-format" OFF)
option(EPUB_CLANG_TIDY "Analyze code with clang-tidy" OFF)

option(EPUB_INSTALL "Generate the install target" ${EPUB_MASTER_PROJECT})
