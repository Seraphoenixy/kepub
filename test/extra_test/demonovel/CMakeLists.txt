file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/梗角色轉生太過頭了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable COMMAND ${DEMONOVEL_EXECUTABLE} -v)
add_test(NAME run
         COMMAND ${DEMONOVEL_EXECUTABLE} 梗角色轉生太過頭了！.txt --no-cover
                 --date=2021-05-29 --uuid=5208e6bb-5d25-45b0-a7fd-b97d79a85fd4)
add_test(NAME diff COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/standard
                           梗角色转生太过头了！)
set_tests_properties(check_executable PROPERTIES FIXTURES_SETUP demonovel)
set_tests_properties(run PROPERTIES FIXTURES_REQUIRED demonovel)
set_tests_properties(diff PROPERTIES FIXTURES_CLEANUP demonovel)
