file(COPY "${KEPUB_SOURCE_DIR}/test/extra_test/ciweimao/100012892.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_package(Catch2 REQUIRED)
add_definitions(-DCATCH_CONFIG_ENABLE_BENCHMARKING)

add_executable(${KEPUB_BENCH_EXECUTABLE} ${MIMALLOC_OBJECT} main.cpp)
target_link_libraries(${KEPUB_BENCH_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}
                                                        Catch2::Catch2WithMain)
