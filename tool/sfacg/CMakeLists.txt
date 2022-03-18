find_package(CLI11 REQUIRED)

add_executable(${SFACG_EXECUTABLE} ${MIMALLOC_OBJECT} sfacg.cpp)
target_link_libraries(
  ${SFACG_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared klib::klib fmt::fmt
                              ${Boost_LIBRARIES} CLI11::CLI11 TBB::tbb)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  find_package(Backward REQUIRED)
  target_link_libraries(${SFACG_EXECUTABLE} PRIVATE Backward::Backward)
endif()
