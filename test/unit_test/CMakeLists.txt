file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

find_package(Catch2 REQUIRED)

add_definitions(-DKEPUB_TEST_PROGRAM_NAME="${KEPUB_TEST_EXECUTABLE}")

add_executable(${KEPUB_TEST_EXECUTABLE} ${TEST_SRC})
target_link_libraries(${KEPUB_TEST_EXECUTABLE}
                      PRIVATE ${KEPUB_LIBRARY} Catch2::Catch2WithMain fmt::fmt)

include(Catch)
catch_discover_tests(${KEPUB_TEST_EXECUTABLE} REPORTER compact)
