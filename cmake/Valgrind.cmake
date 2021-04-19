if(KEPUB_VALGRIND)
  message(STATUS "Execute test with valgrind")

  find_program(VALGRIND_EXECUTABLE valgrind)

  if(NOT VALGRIND_EXECUTABLE)
    message(FATAL_ERROR "Can not find valgrind")
  endif()

  add_test(
    NAME masiro-valgrind
    COMMAND
      ${VALGRIND_EXECUTABLE} --leak-check=full --show-leak-kinds=all
      --leak-resolution=med --track-origins=yes --vgdb=no --tool=memcheck
      --gen-suppressions=all --error-exitcode=1 ./masiro masiro.txt
    WORKING_DIRECTORY ${KEPUB_BINARY_DIR}/test)

  add_test(
    NAME demonovel-valgrind
    COMMAND
      ${VALGRIND_EXECUTABLE} --leak-check=full --show-leak-kinds=all
      --leak-resolution=med --track-origins=yes --vgdb=no --tool=memcheck
      --gen-suppressions=all --error-exitcode=1 ./demonovel demonovel.txt
    WORKING_DIRECTORY ${KEPUB_BINARY_DIR}/test)
endif()
