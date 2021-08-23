file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/ref.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/身为剑圣的我想要当魔王.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_title COMMAND ${TITLE_EXECUTABLE} -v)
add_test(NAME run_title COMMAND ${TITLE_EXECUTABLE} 身为剑圣的我想要当魔王.txt)
add_test(NAME diff_title COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/standard.txt
                                 身为剑圣的我想要当魔王.txt)
set_tests_properties(check_executable_title PROPERTIES FIXTURES_SETUP title)
set_tests_properties(run_title PROPERTIES FIXTURES_REQUIRED title)
set_tests_properties(diff_title PROPERTIES FIXTURES_CLEANUP title)
