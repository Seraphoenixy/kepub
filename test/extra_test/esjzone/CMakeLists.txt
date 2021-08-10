add_test(NAME check_executable_esjzone COMMAND ${ESJZONE_EXECUTABLE} -v)
add_test(NAME run_esjzone COMMAND ${ESJZONE_EXECUTABLE}
                                  https://esjzone.cc/detail/1578022447.html)
add_test(NAME diff_esjzone
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/standard.txt 魅魔的我.txt)
set_tests_properties(check_executable_esjzone PROPERTIES FIXTURES_SETUP esjzone)
set_tests_properties(run_esjzone PROPERTIES FIXTURES_REQUIRED esjzone)
set_tests_properties(diff_esjzone PROPERTIES FIXTURES_CLEANUP esjzone)
