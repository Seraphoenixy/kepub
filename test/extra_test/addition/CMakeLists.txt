file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/魅魔的我.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/魅魔的我.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_addition COMMAND ${ADDITION_EXECUTABLE} -v)
add_test(NAME run_addition COMMAND ${ADDITION_EXECUTABLE} 魅魔的我.txt --testing
                                   --datetime=2021-05-29)
add_test(NAME diff_addition
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/standard 魅魔的我-test)
set_tests_properties(check_executable_addition PROPERTIES FIXTURES_SETUP
                                                          addition)
set_tests_properties(run_addition PROPERTIES FIXTURES_REQUIRED addition)
set_tests_properties(diff_addition PROPERTIES FIXTURES_CLEANUP addition)
