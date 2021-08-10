file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/梗角色轉生太過頭了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_demonovel COMMAND ${DEMONOVEL_EXECUTABLE} -v)
add_test(NAME run_demonovel
         COMMAND ${DEMONOVEL_EXECUTABLE} 梗角色轉生太過頭了！.txt --no-cover
                 --date=2021-05-29 --uuid=5208e6bb-5d25-45b0-a7fd-b97d79a85fd4)
add_test(NAME diff_demonovel
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/standard 梗角色转生太过头了！)
set_tests_properties(check_executable_demonovel PROPERTIES FIXTURES_SETUP
                                                           demonovel)
set_tests_properties(run_demonovel PROPERTIES FIXTURES_REQUIRED demonovel)
set_tests_properties(diff_demonovel PROPERTIES FIXTURES_CLEANUP demonovel)
