add_test(NAME check_executable COMMAND ${ESJZONE_EXECUTABLE} -v)
add_test(NAME run COMMAND ${ESJZONE_EXECUTABLE}
                          https://esjzone.cc/detail/1578022447.html)
add_test(NAME diff
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard.txt 魅魔的我.txt)
set_tests_properties(check_executable PROPERTIES FIXTURES_SETUP esjzone)
set_tests_properties(run PROPERTIES FIXTURES_REQUIRED esjzone)
set_tests_properties(diff PROPERTIES FIXTURES_CLEANUP esjzone)
