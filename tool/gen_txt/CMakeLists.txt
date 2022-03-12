find_package(CLI11 REQUIRED)

add_executable(${GEN_TXT_EXECUTABLE} ${MIMALLOC_OBJECT} gen_txt.cpp)
target_link_libraries(
  ${GEN_TXT_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared klib::klib CLI11::CLI11
                                pugixml::pugixml)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)
  target_link_libraries(${GEN_TXT_EXECUTABLE} PRIVATE Backward::Backward)
endif()
