file(COPY ${KEPUB_TEST_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/梗角色转生太过头了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_kepub COMMAND ${KEPUB_EXECUTABLE} -v)
add_test(
  NAME run_kepub
  COMMAND ${KEPUB_EXECUTABLE} 梗角色转生太过头了！.txt -c -i 3 --datetime=2021-05-29
          --uuid=5208e6bb-5d25-45b0-a7fd-b97d79a85fd4 -t)
add_test(NAME diff_kepub COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/standard
                                 梗角色转生太过头了！)
set_tests_properties(check_executable_kepub PROPERTIES FIXTURES_SETUP kepub)
set_tests_properties(run_kepub PROPERTIES FIXTURES_REQUIRED kepub)
set_tests_properties(diff_kepub PROPERTIES FIXTURES_CLEANUP kepub)
