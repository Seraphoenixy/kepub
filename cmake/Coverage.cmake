if(KEPUB_BUILD_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    message(
      STATUS "Build with coverage information, use lcov to generate report")

    # https://github.com/nlohmann/json/blob/develop/test/CMakeLists.txt
    get_filename_component(COMPILER_PATH ${CMAKE_CXX_COMPILER} PATH)
    string(REGEX MATCH "^[0-9]+" GCC_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    find_program(
      GCOV_EXECUTABLE
      NAMES gcov-${GCC_VERSION} gcov
      HINTS ${COMPILER_PATH})
    if(NOT GCOV_EXECUTABLE)
      message(FATAL_ERROR "Can not find gcov")
    endif()

    find_program(LCOV_EXECUTABLE lcov)
    if(NOT LCOV_EXECUTABLE)
      message(FATAL_ERROR "Can not find lcov")
    endif()

    find_program(GENHTML_EXECUTABLE genhtml)
    if(NOT GENHTML_EXECUTABLE)
      message(FATAL_ERROR "Can not find genhtml")
    endif()

    # https://github.com/linux-test-project/lcov
    # https://github.com/nlohmann/json/blob/develop/test/CMakeLists.txt
    add_custom_target(
      coverage
      COMMAND ${LCOV_EXECUTABLE} -d ${KEPUB_BINARY_DIR} -z
      COMMAND ${TEST_EXECUTABLE}
      COMMAND
        ${LCOV_EXECUTABLE} -d ${KEPUB_BINARY_DIR} --include
        '${KEPUB_SOURCE_DIR}/src/*.cpp' --include
        '${KEPUB_SOURCE_DIR}/include/*.h' --gcov-tool ${GCOV_EXECUTABLE} -c -o
        lcov.info --rc lcov_branch_coverage=1
      COMMAND ${GENHTML_EXECUTABLE} lcov.info -o coverage -s --title
              "${PROJECT_NAME}" --legend --demangle-cpp --branch-coverage
      WORKING_DIRECTORY ${KEPUB_BINARY_DIR}/test/unit_test
      DEPENDS ${TEST_EXECUTABLE}
      COMMENT
        "Generate HTML report: ${KEPUB_BINARY_DIR}/test/unit_test/coverage/index.html"
    )
  else()
    message(
      STATUS "Build with coverage information, use llvm-cov to generate report")

    find_program(LLVM_PROFDATA_EXECUTABLE llvm-profdata)
    if(NOT LLVM_PROFDATA_EXECUTABLE)
      message(FATAL_ERROR "Can not find llvm-profdata")
    endif()

    find_program(LLVM_COV_EXECUTABLE llvm-cov)
    if(NOT LLVM_COV_EXECUTABLE)
      message(FATAL_ERROR "Can not find llvm-cov")
    endif()

    # https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
    # https://llvm.org/docs/CommandGuide/llvm-cov.html
    add_custom_target(
      coverage
      COMMAND ${TEST_EXECUTABLE}
      COMMAND ${LLVM_PROFDATA_EXECUTABLE} merge -sparse -o
              ${TEST_EXECUTABLE}.profdata default.profraw
      COMMAND
        ${LLVM_COV_EXECUTABLE} show ./${TEST_EXECUTABLE}
        -instr-profile=${TEST_EXECUTABLE}.profdata -format=html
        -show-branches=percent -show-line-counts-or-regions
        -ignore-filename-regex=${KEPUB_SOURCE_DIR}/test/* -output-dir=coverage
      COMMAND
        ${LLVM_COV_EXECUTABLE} export ./${TEST_EXECUTABLE}
        -instr-profile=${TEST_EXECUTABLE}.profdata
        -ignore-filename-regex=${KEPUB_SOURCE_DIR}/test/* -format=lcov >
        lcov.info
      WORKING_DIRECTORY ${KEPUB_BINARY_DIR}/test/unit_test
      DEPENDS ${TEST_EXECUTABLE}
      COMMENT
        "Generate HTML report: ${KEPUB_BINARY_DIR}/test/unit_test/coverage/index.html"
    )
  endif()
endif()
