add_executable(${DEMONOVEL_EXECUTABLE} demonovel.cpp)
target_link_libraries(${DEMONOVEL_EXECUTABLE} PRIVATE ${LIBRARY})

if(KEPUB_USE_LIBCXX)
  target_link_libraries(${DEMONOVEL_EXECUTABLE} PRIVATE ${KEPUB_LIBCXX}
                                                        ${KEPUB_LIBCXXABI})
endif()

find_package(pugixml REQUIRED)
find_package(CURL CONFIG REQUIRED)

add_executable(${ESJZONE_EXECUTABLE} esjzone.cpp)
target_link_libraries(${ESJZONE_EXECUTABLE} PRIVATE ${LIBRARY} CURL::libcurl
                                                    pugixml::static tidys)

if(KEPUB_USE_LIBCXX)
  target_link_libraries(${ESJZONE_EXECUTABLE} PRIVATE ${KEPUB_LIBCXX}
                                                      ${KEPUB_LIBCXXABI})
endif()

add_executable(${MASIRO_EXECUTABLE} masiro.cpp)
target_link_libraries(${MASIRO_EXECUTABLE} PRIVATE ${LIBRARY})

if(KEPUB_USE_LIBCXX)
  target_link_libraries(${MASIRO_EXECUTABLE} PRIVATE ${KEPUB_LIBCXX}
                                                     ${KEPUB_LIBCXXABI})
endif()
