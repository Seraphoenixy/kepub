find_package(CLI11 REQUIRED)

add_executable(${KEPUB_EXECUTABLE} ${MIMALLOC_OBJECT} kepub.cpp)
target_link_libraries(${KEPUB_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared
                                                  klib::klib CLI11::CLI11)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)
  target_link_libraries(${KEPUB_EXECUTABLE} PRIVATE Backward::Backward)
endif()
