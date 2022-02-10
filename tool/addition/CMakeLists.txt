find_package(CLI11 REQUIRED)

add_executable(${ADDITION_EXECUTABLE} ${MIMALLOC_OBJECT} addition.cpp)
target_link_libraries(${ADDITION_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared
                                                     klib::klib CLI11::CLI11)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)
  target_link_libraries(${ADDITION_EXECUTABLE} PRIVATE Backward::Backward)
endif()
