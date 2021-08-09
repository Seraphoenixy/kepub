file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/梗角色转生太过头了！2.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/梗角色轉生太過頭了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/魅魔的我2.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/魅魔的我2.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_demonovel COMMAND ${DEMONOVEL_EXECUTABLE} -v)
add_test(NAME run_demonovel
         COMMAND ${DEMONOVEL_EXECUTABLE} 梗角色轉生太過頭了！.txt --no-cover
                 --date=2021-05-29 --uuid=5208e6bb-5d25-45b0-a7fd-b97d79a85fd4)
add_test(NAME diff_demonovel
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-demonovel
                 梗角色转生太过头了！)
set_tests_properties(check_executable_demonovel PROPERTIES FIXTURES_SETUP
                                                           demonovel)
set_tests_properties(run_demonovel PROPERTIES FIXTURES_REQUIRED demonovel)
set_tests_properties(diff_demonovel PROPERTIES FIXTURES_CLEANUP demonovel)

add_test(NAME check_executable_masiro COMMAND ${MASIRO_EXECUTABLE} -v)
add_test(
  NAME run_masiro
  COMMAND ${MASIRO_EXECUTABLE} 梗角色转生太过头了！2.txt -c -p -i 3 --date=2021-05-29
          --uuid=5208e6bb-5d25-45b0-a7fd-b97d79a85fd4 --image 6)
add_test(NAME remove_images_masiro COMMAND rm -rf 梗角色转生太过头了！2/OEBPS/Images)
add_test(NAME diff_masiro
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-masiro
                 梗角色转生太过头了！2)
set_tests_properties(check_executable_masiro PROPERTIES FIXTURES_SETUP masiro)
set_tests_properties(run_masiro PROPERTIES FIXTURES_REQUIRED masiro)
set_tests_properties(remove_images_masiro PROPERTIES FIXTURES_REQUIRED masiro)
set_tests_properties(diff_masiro PROPERTIES FIXTURES_CLEANUP masiro)
set_tests_properties(remove_images_masiro PROPERTIES DEPENDS run_masiro)

add_test(NAME check_executable_xhtml COMMAND ${XHTML_EXECUTABLE} -v)
add_test(NAME run_xhtml COMMAND ${XHTML_EXECUTABLE} 梗角色转生太过头了！2.txt)
add_test(NAME diff_xhtml
         COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/data/standard.xhtml
                 梗角色转生太过头了！2.xhtml)
set_tests_properties(check_executable_xhtml PROPERTIES FIXTURES_SETUP xhtml)
set_tests_properties(run_xhtml PROPERTIES FIXTURES_REQUIRED xhtml)
set_tests_properties(diff_xhtml PROPERTIES FIXTURES_CLEANUP xhtml)

add_test(NAME check_executable_esjzone COMMAND ${ESJZONE_EXECUTABLE} -v)
add_test(NAME run_esjzone COMMAND ${ESJZONE_EXECUTABLE}
                                  https://esjzone.cc/detail/1578022447.html)
add_test(NAME diff_esjzone
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-esjzone.txt
                 魅魔的我.txt)
set_tests_properties(check_executable_esjzone PROPERTIES FIXTURES_SETUP esjzone)
set_tests_properties(run_esjzone PROPERTIES FIXTURES_REQUIRED esjzone)
set_tests_properties(diff_esjzone PROPERTIES FIXTURES_CLEANUP esjzone)

add_test(NAME check_executable_addition COMMAND ${ADDITION_EXECUTABLE} -v)
add_test(NAME run_addition COMMAND ${ADDITION_EXECUTABLE} 魅魔的我2.txt)
add_test(NAME diff_addition
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-addition
                 魅魔的我2)
set_tests_properties(check_executable_addition PROPERTIES FIXTURES_SETUP
                                                          addition)
set_tests_properties(run_addition PROPERTIES FIXTURES_REQUIRED addition)
set_tests_properties(diff_addition PROPERTIES FIXTURES_CLEANUP addition)
