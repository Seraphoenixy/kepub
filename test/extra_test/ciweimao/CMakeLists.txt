file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/100012892.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_ciweimao_kepub COMMAND ${KEPUB_EXECUTABLE} -v)
add_test(NAME run_ciweimao_kepub
         COMMAND ${KEPUB_EXECUTABLE} 100012892.txt --datetime=2021-11-01
                 --uuid=3d33e282-e707-4e19-90d5-ba8cadf248fd)
add_test(NAME diff_ciweimao_kepub
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/100012892-standard
                 100012892)
set_tests_properties(check_executable_ciweimao_kepub PROPERTIES FIXTURES_SETUP
                                                                ciweimao_kepub)
set_tests_properties(run_ciweimao_kepub PROPERTIES FIXTURES_REQUIRED
                                                   ciweimao_kepub)
set_tests_properties(diff_ciweimao_kepub PROPERTIES FIXTURES_CLEANUP
                                                    ciweimao_kepub)
