find_package(CLI11 REQUIRED)

add_executable(${EXTRACT_EPUB_EXECUTABLE} ${MIMALLOC_OBJECT} extract_epub.cpp)
target_link_libraries(
  ${EXTRACT_EPUB_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared klib::klib
                                     CLI11::CLI11 pugixml::pugixml)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)
  target_link_libraries(${EXTRACT_EPUB_EXECUTABLE} PRIVATE Backward::Backward)
endif()
