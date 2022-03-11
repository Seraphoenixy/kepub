file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/263060.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_sfacg_kepub COMMAND ${KEPUB_EXECUTABLE} -v)
add_test(NAME run_sfacg_kepub
         COMMAND ${KEPUB_EXECUTABLE} 263060.txt --datetime=2021-11-01
                 --uuid=3d33e282-e707-4e19-90d5-ba8cadf248fd)
add_test(NAME diff_sfacg_kepub
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/263060-standard 263060)
set_tests_properties(check_executable_sfacg_kepub PROPERTIES FIXTURES_SETUP
                                                             sfacg_kepub)
set_tests_properties(run_sfacg_kepub PROPERTIES FIXTURES_REQUIRED sfacg_kepub)
set_tests_properties(diff_sfacg_kepub PROPERTIES FIXTURES_CLEANUP sfacg_kepub)
