file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/masiro-txt.txt"
     DESTINATION ${KEPUB_BINARY_DIR}/tool)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/demonovel-txt.txt"
     DESTINATION ${KEPUB_BINARY_DIR}/tool)

enable_testing()

add_test(
  NAME masiro-test
  COMMAND masiro masiro-txt.txt
  WORKING_DIRECTORY ${KEPUB_BINARY_DIR}/tool)

add_test(
  NAME demonovel-test
  COMMAND demonovel demonovel-txt.txt
  WORKING_DIRECTORY ${KEPUB_BINARY_DIR}/tool)

include(Coverage)
include(Valgrind)
