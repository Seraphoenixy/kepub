file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/1578022447.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_esjzone_kepub COMMAND ${KEPUB_EXECUTABLE} -v)
add_test(NAME run_esjzone_kepub
         COMMAND ${KEPUB_EXECUTABLE} 1578022447.txt --datetime=2021-11-30
                 --uuid=72074299-583c-4b62-8bdf-18382b11622c)
add_test(NAME diff_esjzone_kepub
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/1578022447-standard
                 1578022447)
set_tests_properties(check_executable_esjzone_kepub PROPERTIES FIXTURES_SETUP
                                                               esjzone_kepub)
set_tests_properties(run_esjzone_kepub PROPERTIES FIXTURES_REQUIRED
                                                  esjzone_kepub)
set_tests_properties(diff_esjzone_kepub PROPERTIES FIXTURES_CLEANUP
                                                   esjzone_kepub)
