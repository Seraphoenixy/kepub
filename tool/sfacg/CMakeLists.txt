find_package(CLI11 REQUIRED)
find_package(simdjson REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS json)

add_executable(${SFACG_EXECUTABLE} ${MIMALLOC_OBJECT} sfacg.cpp json.cpp
                                   http.cpp)
target_link_libraries(
  ${SFACG_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared klib::klib fmt::fmt ${Boost_LIBRARIES}
          CLI11::CLI11 simdjson::simdjson)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)
  target_link_libraries(${SFACG_EXECUTABLE} PRIVATE Backward::Backward)
endif()
