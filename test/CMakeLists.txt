file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/masiro.txt"
     DESTINATION ${KEPUB_BINARY_DIR}/tool)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/demonovel.txt"
     DESTINATION ${KEPUB_BINARY_DIR}/tool)

enable_testing()

file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

find_package(Catch2 REQUIRED)

add_executable(${TEST_EXECUTABLE} ${TEST_SRC})
target_link_libraries(${TEST_EXECUTABLE} PRIVATE ${LIBRARY}
                                                 Catch2::Catch2WithMain)

# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
include(Catch)
catch_discover_tests(${TEST_EXECUTABLE} REPORTER compact)

include(Coverage)
include(Valgrind)
