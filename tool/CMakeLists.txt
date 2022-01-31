find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)

add_executable(${ESJZONE_EXECUTABLE} ${MIMALLOC_OBJECT} esjzone.cpp)
target_link_libraries(
  ${ESJZONE_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared pugixml::pugixml spdlog::spdlog klib::klib
          ${Boost_LIBRARIES} CLI11::CLI11)

add_executable(${KEPUB_EXECUTABLE} ${MIMALLOC_OBJECT} kepub.cpp)
target_link_libraries(
  ${KEPUB_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared spdlog::spdlog klib::klib
                              CLI11::CLI11)

add_executable(${ADDITION_EXECUTABLE} ${MIMALLOC_OBJECT} addition.cpp)
target_link_libraries(${ADDITION_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared
                                                     klib::klib CLI11::CLI11)

add_executable(${CIWEIMAO_EXECUTABLE} ${MIMALLOC_OBJECT} ciweimao/ciweimao.cpp
                                      ciweimao/json.cpp)
target_link_libraries(
  ${CIWEIMAO_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared
          klib::klib
          spdlog::spdlog
          ${Boost_LIBRARIES}
          pugixml::pugixml
          CLI11::CLI11
          simdjson::simdjson)

add_executable(${SFACG_EXECUTABLE} ${MIMALLOC_OBJECT} sfacg/sfacg.cpp
                                   sfacg/json.cpp)
target_link_libraries(
  ${SFACG_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared
          klib::klib
          fmt::fmt
          spdlog::spdlog
          ${Boost_LIBRARIES}
          CLI11::CLI11
          simdjson::simdjson)

add_executable(${GEN_EPUB_EXECUTABLE} ${MIMALLOC_OBJECT} gen_epub.cpp)
target_link_libraries(
  ${GEN_EPUB_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared klib::klib
                                 spdlog::spdlog CLI11::CLI11)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)

  target_link_libraries(${ESJZONE_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${KEPUB_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${ADDITION_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${CIWEIMAO_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${SFACG_EXECUTABLE} PRIVATE Backward::Backward)
  target_link_libraries(${GEN_EPUB_EXECUTABLE} PRIVATE Backward::Backward)
endif()
