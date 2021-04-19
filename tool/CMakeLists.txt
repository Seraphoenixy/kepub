file(GLOB EXECUTABLE_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(FILE_PATH ${EXECUTABLE_SRC})
  get_filename_component(FILE_NAME ${FILE_PATH} NAME_WE)
  list(APPEND EXECUTABLE ${FILE_NAME})
  add_executable(${FILE_NAME} ${FILE_PATH})
  target_link_libraries(${FILE_NAME} PRIVATE ${LIBRARY})

  if(KEPUB_USE_LIBCXX)
    target_link_libraries(${FILE_NAME} PRIVATE ${KEPUB_LIBCXX}
                                               ${KEPUB_LIBCXXABI})
  endif()
endforeach()
