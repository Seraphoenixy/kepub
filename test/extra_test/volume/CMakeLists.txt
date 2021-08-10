file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/魅魔的我.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_volume COMMAND ${MASIRO_EXECUTABLE} -v)
add_test(NAME run_volume
         COMMAND ${MASIRO_EXECUTABLE} 魅魔的我.txt --no-cover --date=2021-05-29
                 --uuid=5208e6bb-5d25-45b0-a7fd-b97d79a85fd4)
add_test(NAME diff_volume COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/standard
                                  魅魔的我)
set_tests_properties(check_executable_volume PROPERTIES FIXTURES_SETUP volume)
set_tests_properties(run_volume PROPERTIES FIXTURES_REQUIRED volume)
set_tests_properties(diff_volume PROPERTIES FIXTURES_CLEANUP volume)
