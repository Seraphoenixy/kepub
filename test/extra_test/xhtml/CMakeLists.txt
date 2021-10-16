file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/梗角色转生太过头了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_xhtml COMMAND ${XHTML_EXECUTABLE} -v)
add_test(NAME run_xhtml COMMAND ${XHTML_EXECUTABLE} 梗角色转生太过头了！.txt -t)
add_test(NAME diff_xhtml
         COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/standard.xhtml
                 梗角色转生太过头了！.xhtml)
set_tests_properties(check_executable_xhtml PROPERTIES FIXTURES_SETUP xhtml)
set_tests_properties(run_xhtml PROPERTIES FIXTURES_REQUIRED xhtml)
set_tests_properties(diff_xhtml PROPERTIES FIXTURES_CLEANUP xhtml)
