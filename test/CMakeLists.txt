file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/masiro.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/demonovel.txt"
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

enable_testing()

add_test(
  NAME masiro-test
  COMMAND masiro masiro.txt
  WORKING_DIRECTORY ${KEPUB_BINARY_DIR}/test)

add_test(
  NAME demonovel-test
  COMMAND demonovel demonovel.txt
  WORKING_DIRECTORY ${KEPUB_BINARY_DIR}/test)

include(Coverage)
include(Valgrind)
