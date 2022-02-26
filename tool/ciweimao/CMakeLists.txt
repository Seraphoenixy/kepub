find_package(CLI11 REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS json)

add_executable(${CIWEIMAO_EXECUTABLE} ${MIMALLOC_OBJECT} ciweimao.cpp json.cpp
                                      http.cpp aes.cpp)
target_link_libraries(
  ${CIWEIMAO_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared klib::klib ${Boost_LIBRARIES}
          pugixml::pugixml CLI11::CLI11 simdjson::simdjson)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)
  target_link_libraries(${CIWEIMAO_EXECUTABLE} PRIVATE Backward::Backward)
endif()
