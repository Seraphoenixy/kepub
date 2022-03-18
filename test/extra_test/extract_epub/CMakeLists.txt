file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/变成狐娘的我被真龙圣女捕获了.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/被神明误伤的我只想好好做个普通人.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/讨厌第四次的死属性魔法师.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/梗角色转生太过头了！.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME check_executable_extract_epub_1 COMMAND ${EXTRACT_EPUB_EXECUTABLE}
                                                      -v)
add_test(NAME run_extract_epub_1 COMMAND ${EXTRACT_EPUB_EXECUTABLE}
                                         变成狐娘的我被真龙圣女捕获了.epub)
add_test(
  NAME diff_extract_epub_1
  COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/变成狐娘的我被真龙圣女捕获了-standard.txt
          变成狐娘的我被真龙圣女捕获了.txt)
set_tests_properties(check_executable_extract_epub_1 PROPERTIES FIXTURES_SETUP
                                                                gen_txt_1)
set_tests_properties(run_extract_epub_1 PROPERTIES FIXTURES_REQUIRED gen_txt_1)
set_tests_properties(diff_extract_epub_1 PROPERTIES FIXTURES_CLEANUP gen_txt_1)

add_test(NAME check_executable_extract_epub_2 COMMAND ${EXTRACT_EPUB_EXECUTABLE}
                                                      -v)
add_test(NAME run_extract_epub_2 COMMAND ${EXTRACT_EPUB_EXECUTABLE}
                                         被神明误伤的我只想好好做个普通人.epub)
add_test(
  NAME diff_extract_epub_2
  COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/被神明误伤的我只想好好做个普通人-standard.txt
          被神明误伤的我只想好好做个普通人.txt)
set_tests_properties(check_executable_extract_epub_2 PROPERTIES FIXTURES_SETUP
                                                                gen_txt_2)
set_tests_properties(run_extract_epub_2 PROPERTIES FIXTURES_REQUIRED gen_txt_2)
set_tests_properties(diff_extract_epub_2 PROPERTIES FIXTURES_CLEANUP gen_txt_2)

add_test(NAME check_executable_extract_epub_3 COMMAND ${EXTRACT_EPUB_EXECUTABLE}
                                                      -v)
add_test(NAME run_extract_epub_3 COMMAND ${EXTRACT_EPUB_EXECUTABLE}
                                         讨厌第四次的死属性魔法师.epub)
add_test(NAME diff_extract_epub_3
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/讨厌第四次的死属性魔法师-standard.txt
                 讨厌第四次的死属性魔法师.txt)
set_tests_properties(check_executable_extract_epub_3 PROPERTIES FIXTURES_SETUP
                                                                gen_txt_3)
set_tests_properties(run_extract_epub_3 PROPERTIES FIXTURES_REQUIRED gen_txt_3)
set_tests_properties(diff_extract_epub_3 PROPERTIES FIXTURES_CLEANUP gen_txt_3)

add_test(NAME check_executable_extract_epub_4 COMMAND ${EXTRACT_EPUB_EXECUTABLE}
                                                      -v)
add_test(NAME run_extract_epub_4 COMMAND ${EXTRACT_EPUB_EXECUTABLE}
                                         梗角色转生太过头了！.epub)
add_test(NAME diff_extract_epub_4
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/梗角色转生太过头了！-standard.txt
                 梗角色转生太过头了！.txt)
set_tests_properties(check_executable_extract_epub_4 PROPERTIES FIXTURES_SETUP
                                                                gen_txt_4)
set_tests_properties(run_extract_epub_4 PROPERTIES FIXTURES_REQUIRED gen_txt_4)
set_tests_properties(diff_extract_epub_4 PROPERTIES FIXTURES_CLEANUP gen_txt_4)
