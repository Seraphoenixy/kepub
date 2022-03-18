find_package(CLI11 REQUIRED)

add_executable(${GEN_EPUB_EXECUTABLE} ${MIMALLOC_OBJECT} gen_epub.cpp)
target_link_libraries(${GEN_EPUB_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared
                                                     klib::klib CLI11::CLI11)

add_executable(${APPEND_EPUB_EXECUTABLE} ${MIMALLOC_OBJECT} append_epub.cpp)
target_link_libraries(${APPEND_EPUB_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared
                                                        klib::klib CLI11::CLI11)

add_executable(${EXTRACT_EPUB_EXECUTABLE} ${MIMALLOC_OBJECT} extract_epub.cpp)
target_link_libraries(
  ${EXTRACT_EPUB_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared klib::klib
                                     CLI11::CLI11 pugixml::pugixml)

add_executable(${SFACG_EXECUTABLE} ${MIMALLOC_OBJECT} sfacg.cpp)
target_link_libraries(
  ${SFACG_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared klib::klib fmt::fmt
                              ${Boost_LIBRARIES} CLI11::CLI11 TBB::tbb)

add_executable(${CIWEIMAO_EXECUTABLE} ${MIMALLOC_OBJECT} ciweimao.cpp)
target_link_libraries(
  ${CIWEIMAO_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared klib::klib ${Boost_LIBRARIES}
          pugixml::pugixml CLI11::CLI11 TBB::tbb)

add_executable(${ESJZONE_EXECUTABLE} ${MIMALLOC_OBJECT} esjzone.cpp)
target_link_libraries(
  ${ESJZONE_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared klib::klib ${Boost_LIBRARIES}
          pugixml::pugixml CLI11::CLI11 TBB::tbb)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)

  target_link_libraries(${GEN_EPUB_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${APPEND_EPUB_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${EXTRACT_EPUB_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${SFACG_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${CIWEIMAO_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${ESJZONE_EXECUTABLE} PRIVATE Backward::Backward)
endif()
