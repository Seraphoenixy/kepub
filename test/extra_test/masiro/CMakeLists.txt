file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/梗角色转生太过头了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_masiro COMMAND ${MASIRO_EXECUTABLE} -v)
add_test(
  NAME run_masiro
  COMMAND
    ${VALGRIND_CMD}${MASIRO_EXECUTABLE} 梗角色转生太过头了！.txt -c -p -i 3
    --date=2021-05-29 --uuid=5208e6bb-5d25-45b0-a7fd-b97d79a85fd4 --image 6 -t)
add_test(NAME remove_images_masiro COMMAND rm -rf 梗角色转生太过头了！/OEBPS/Images)
add_test(NAME diff_masiro COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/standard
                                  梗角色转生太过头了！)
set_tests_properties(check_executable_masiro PROPERTIES FIXTURES_SETUP masiro)
set_tests_properties(run_masiro PROPERTIES FIXTURES_REQUIRED masiro)
set_tests_properties(remove_images_masiro PROPERTIES FIXTURES_REQUIRED masiro)
set_tests_properties(diff_masiro PROPERTIES FIXTURES_CLEANUP masiro)
set_tests_properties(remove_images_masiro PROPERTIES DEPENDS run_masiro)
