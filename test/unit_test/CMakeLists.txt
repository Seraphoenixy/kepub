file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

find_package(Catch2 REQUIRED)

add_executable(${TEST_EXECUTABLE} ${TEST_SRC})
target_link_libraries(${TEST_EXECUTABLE} PRIVATE ${LIBRARY}
                                                 Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(${TEST_EXECUTABLE} REPORTER compact)
