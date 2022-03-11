file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/魅魔的我.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/魅魔的我.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_add_volume COMMAND ${ADDITION_EXECUTABLE} -v)
add_test(NAME run_add_volume COMMAND ${ADDITION_EXECUTABLE} 魅魔的我.txt --testing
                                     --datetime=2021-05-29)
add_test(NAME diff_add_volume COMMAND diff -r
                                      ${CMAKE_CURRENT_SOURCE_DIR}/standard 魅魔的我)
set_tests_properties(check_executable_add_volume PROPERTIES FIXTURES_SETUP
                                                            add_volume)
set_tests_properties(run_add_volume PROPERTIES FIXTURES_REQUIRED add_volume)
set_tests_properties(diff_add_volume PROPERTIES FIXTURES_CLEANUP add_volume)
