enable_testing()

add_subdirectory(unit_test)
add_subdirectory(extra_test)

include(Coverage)
include(Valgrind)
