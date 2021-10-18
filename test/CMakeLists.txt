enable_testing()

add_subdirectory(unit_test)

include(Coverage)
include(Valgrind)

add_subdirectory(extra_test)
