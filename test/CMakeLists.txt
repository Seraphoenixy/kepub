file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/梗角色转生太过头了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/梗角色轉生太過頭了！.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/魅魔的我.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/魅魔的我-old-style.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data/魅魔的我-old-style.epub"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

if(KEPUB_VALGRIND)
  message(STATUS "Execute test with valgrind")

  find_program(VALGRIND_EXECUTABLE valgrind)

  if(NOT VALGRIND_EXECUTABLE)
    message(FATAL_ERROR "Can not find valgrind")
  endif()

  set(VALGRIND_CMD
      ${VALGRIND_EXECUTABLE}
      --leak-check=full
      --show-leak-kinds=all
      --leak-resolution=med
      --track-origins=yes
      --vgdb=no
      --tool=memcheck
      --gen-suppressions=all
      --error-exitcode=1
      ${KEPUB_BINARY_DIR}/tool/)
endif()

add_test(NAME check_executable_demonovel COMMAND ${DEMONOVEL_EXECUTABLE} -v)
add_test(NAME run_demonovel COMMAND ${VALGRIND_CMD}${DEMONOVEL_EXECUTABLE}
                                    梗角色轉生太過頭了！.txt --no-cover --date=2021-05-29)
add_test(NAME remove_images_demonovel COMMAND rm -rf 梗角色转生太过头了！/OEBPS/Images)
add_test(NAME diff_demonovel
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-demonovel
                 梗角色转生太过头了！)
set_tests_properties(check_executable_demonovel PROPERTIES FIXTURES_SETUP
                                                           demonovel)
set_tests_properties(run_demonovel PROPERTIES FIXTURES_REQUIRED demonovel)
set_tests_properties(remove_images_demonovel PROPERTIES FIXTURES_REQUIRED
                                                        demonovel)
set_tests_properties(diff_demonovel PROPERTIES FIXTURES_CLEANUP demonovel)
set_tests_properties(remove_images_demonovel PROPERTIES DEPENDS run_demonovel)

add_test(NAME check_executable_masiro COMMAND ${MASIRO_EXECUTABLE} -v)
add_test(NAME run_masiro
         COMMAND ${VALGRIND_CMD}${MASIRO_EXECUTABLE} 梗角色转生太过头了！.txt -c -p -i 3
                 --date=2021-05-29 --image 3)
add_test(NAME remove_images_masiro COMMAND rm -rf 梗角色转生太过头了！/OEBPS/Images)
add_test(NAME diff_masiro
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-masiro
                 梗角色转生太过头了！)
set_tests_properties(check_executable_masiro PROPERTIES FIXTURES_SETUP masiro)
set_tests_properties(run_masiro PROPERTIES FIXTURES_REQUIRED masiro)
set_tests_properties(remove_images_masiro PROPERTIES FIXTURES_REQUIRED masiro)
set_tests_properties(diff_masiro PROPERTIES FIXTURES_CLEANUP masiro)
set_tests_properties(remove_images_masiro PROPERTIES DEPENDS run_masiro)
set_tests_properties(run_masiro PROPERTIES DEPENDS diff_demonovel)

add_test(NAME check_executable_xhtml COMMAND ${XHTML_EXECUTABLE} -v)
add_test(NAME run_xhtml COMMAND ${VALGRIND_CMD}${XHTML_EXECUTABLE}
                                梗角色转生太过头了！.txt)
add_test(NAME diff_xhtml
         COMMAND diff ${CMAKE_CURRENT_SOURCE_DIR}/data/standard.xhtml
                 梗角色转生太过头了！.xhtml)
set_tests_properties(check_executable_xhtml PROPERTIES FIXTURES_SETUP xhtml)
set_tests_properties(run_xhtml PROPERTIES FIXTURES_REQUIRED xhtml)
set_tests_properties(diff_xhtml PROPERTIES FIXTURES_CLEANUP xhtml)

add_test(NAME check_executable_esjzone COMMAND ${ESJZONE_EXECUTABLE} -v)
add_test(
  NAME run_esjzone
  COMMAND
    ${VALGRIND_CMD}${ESJZONE_EXECUTABLE}
    https://esjzone.cc/detail/1578022447.html --max-chapter=210
    --date=2021-05-29)
add_test(NAME remove_images_esjzone COMMAND rm -rf 魅魔的我/OEBPS/Images)
add_test(NAME diff_esjzone
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-esjzone 魅魔的我)
set_tests_properties(check_executable_esjzone PROPERTIES FIXTURES_SETUP esjzone)
set_tests_properties(run_esjzone PROPERTIES FIXTURES_REQUIRED esjzone)
set_tests_properties(remove_images_esjzone PROPERTIES FIXTURES_REQUIRED esjzone)
set_tests_properties(diff_esjzone PROPERTIES FIXTURES_CLEANUP esjzone)
set_tests_properties(remove_images_esjzone PROPERTIES DEPENDS run_esjzone)

add_test(NAME check_executable_addition COMMAND ${ADDITION_EXECUTABLE} -v)
add_test(NAME run_addition COMMAND ${VALGRIND_CMD}${ADDITION_EXECUTABLE}
                                   魅魔的我.txt --date=2021-05-29)
add_test(NAME remove_images_addition COMMAND rm -rf 魅魔的我/OEBPS/Images)
add_test(NAME diff_addition
         COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-addition
                 魅魔的我)
set_tests_properties(check_executable_addition PROPERTIES FIXTURES_SETUP
                                                          addition)
set_tests_properties(run_addition PROPERTIES FIXTURES_REQUIRED addition)
set_tests_properties(remove_images_addition PROPERTIES FIXTURES_REQUIRED
                                                       addition)
set_tests_properties(diff_addition PROPERTIES FIXTURES_CLEANUP addition)
set_tests_properties(remove_images_addition PROPERTIES DEPENDS run_addition)
set_tests_properties(run_addition PROPERTIES DEPENDS diff_esjzone)

add_test(NAME run_addition_old_style
         COMMAND ${VALGRIND_CMD}${ADDITION_EXECUTABLE} 魅魔的我-old-style.txt
                 --old-style)
add_test(NAME remove_images_addition_old_style
         COMMAND rm -rf 魅魔的我-old-style/OEBPS/Images)
add_test(
  NAME diff_addition_old_style
  COMMAND diff -r ${CMAKE_CURRENT_SOURCE_DIR}/data/standard-addition-old-style
          魅魔的我-old-style)
set_tests_properties(check_executable_addition PROPERTIES FIXTURES_SETUP
                                                          addition_old_style)
set_tests_properties(run_addition_old_style PROPERTIES FIXTURES_REQUIRED
                                                       addition_old_style)
set_tests_properties(remove_images_addition_old_style
                     PROPERTIES FIXTURES_REQUIRED addition_old_style)
set_tests_properties(diff_addition_old_style PROPERTIES FIXTURES_CLEANUP
                                                        addition_old_style)
set_tests_properties(remove_images_addition_old_style
                     PROPERTIES DEPENDS run_addition_old_style)

include(Coverage)
