add_executable(${DEMONOVEL_EXECUTABLE} demonovel.cpp)
target_link_libraries(${DEMONOVEL_EXECUTABLE} PRIVATE ${LIBRARY})

add_executable(${ESJZONE_EXECUTABLE} esjzone.cpp)
target_link_libraries(${ESJZONE_EXECUTABLE} PRIVATE ${LIBRARY})

add_executable(${MASIRO_EXECUTABLE} masiro.cpp)
target_link_libraries(${MASIRO_EXECUTABLE} PRIVATE ${LIBRARY})

add_executable(${XHTML_EXECUTABLE} xhtml.cpp)
target_link_libraries(${XHTML_EXECUTABLE} PRIVATE ${LIBRARY})

add_executable(${ADDITION_EXECUTABLE} addition.cpp)
target_link_libraries(${ADDITION_EXECUTABLE} PRIVATE ${LIBRARY})

add_executable(${LIGHTNOVEL_EXECUTABLE} lightnovel.cpp)
target_link_libraries(${LIGHTNOVEL_EXECUTABLE} PRIVATE ${LIBRARY})
