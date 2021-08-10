file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/魅魔的我.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/魅魔的我.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable COMMAND ${ADDITION_EXECUTABLE} -v)
add_test(NAME run COMMAND ${ADDITION_EXECUTABLE} 魅魔的我.txt --no-compress)
add_test(NAME diff COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard
                           魅魔的我)
set_tests_properties(check_executable PROPERTIES FIXTURES_SETUP addition)
set_tests_properties(run PROPERTIES FIXTURES_REQUIRED addition)
set_tests_properties(diff PROPERTIES FIXTURES_CLEANUP addition)
