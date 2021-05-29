file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/梗角色转生太过头了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/梗角色轉生太過頭了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

add_test(NAME check_executable_xhtml COMMAND ${XHTML_EXECUTABLE} -v)
add_test(NAME run_xhtml COMMAND ${XHTML_EXECUTABLE} 梗角色转生太过头了！.txt)
add_test(NAME diff_xhtml
         COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/data/standard.xhtml
                 梗角色转生太过头了！.xhtml)
set_tests_properties(check_executable_xhtml PROPERTIES FIXTURES_SETUP xhtml)
set_tests_properties(run_xhtml PROPERTIES FIXTURES_REQUIRED xhtml)
set_tests_properties(diff_xhtml PROPERTIES FIXTURES_CLEANUP xhtml)

add_test(NAME check_executable_demonovel COMMAND ${DEMONOVEL_EXECUTABLE} -v)
add_test(NAME run_demonovel COMMAND ${DEMONOVEL_EXECUTABLE} 梗角色轉生太過頭了！.txt
                                    --no-cover)
add_test(NAME diff_demonovel
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-demonovel
                 梗角色转生太过头了！)
set_tests_properties(check_executable_demonovel PROPERTIES FIXTURES_SETUP
                                                           demonovel)
set_tests_properties(run_demonovel PROPERTIES FIXTURES_REQUIRED demonovel)
set_tests_properties(diff_demonovel PROPERTIES FIXTURES_CLEANUP demonovel)

add_test(NAME check_executable_masiro COMMAND ${MASIRO_EXECUTABLE} -v)
add_test(NAME run_masiro COMMAND ${MASIRO_EXECUTABLE} 梗角色转生太过头了！.txt)
add_test(NAME diff_masiro
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-masiro
                 梗角色转生太过头了！)
set_tests_properties(check_executable_masiro PROPERTIES FIXTURES_SETUP masiro)
set_tests_properties(run_masiro PROPERTIES FIXTURES_REQUIRED masiro)
set_tests_properties(diff_masiro PROPERTIES FIXTURES_CLEANUP masiro)

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
