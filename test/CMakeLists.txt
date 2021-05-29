file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/masiro.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/demonovel.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/standard.xhtml"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

add_test(NAME check_executable_xhtml COMMAND ${XHTML_EXECUTABLE} -v)
add_test(NAME run_xhtml COMMAND ${XHTML_EXECUTABLE} masiro.txt)
add_test(NAME diff_xhtml COMMAND diff standard.xhtml masiro.xhtml)

add_test(NAME check_executable_demonovel COMMAND ${DEMONOVEL_EXECUTABLE} -v)
# add_test(NAME run_demonovel COMMAND ${DEMONOVEL_EXECUTABLE} demonovel.txt)
# add_test(NAME diff_demonovel COMMAND diff -r)

add_test(NAME check_executable_masiro COMMAND ${MASIRO_EXECUTABLE} -v)
# add_test(NAME run_masiro COMMAND ${MASIRO_EXECUTABLE} masiro.txt)
# add_test(NAME diff_masiro COMMAND diff -r)

add_test(NAME check_executable_esjzone COMMAND ${ESJZONE_EXECUTABLE} -v)
# add_test(NAME run_esjzone COMMAND ${ESJZONE_EXECUTABLE}) add_test(NAME
# diff_esjzone COMMAND diff -r)

add_test(NAME check_executable_addition COMMAND ${ADDITION_EXECUTABLE} -v)
# add_test(NAME run_addition COMMAND ${ADDITION_EXECUTABLE}) add_test(NAME
# diff_addition COMMAND diff -r)

add_test(NAME check_executable_lightnovel COMMAND ${LIGHTNOVEL_EXECUTABLE} -v)
# add_test(NAME run_lightnovel COMMAND ${LIGHTNOVEL_EXECUTABLE}) add_test(NAME
# diff_lightnovel COMMAND diff -r)

include(Coverage)
include(Valgrind)
