find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)

add_executable(${DEMONOVEL_EXECUTABLE} demonovel.cpp)
target_link_libraries(${DEMONOVEL_EXECUTABLE} PRIVATE ${LIBRARY}-shared
                                                      klib::klib CLI11::CLI11)

add_executable(${ESJZONE_EXECUTABLE} esjzone.cpp)
target_link_libraries(
  ${ESJZONE_EXECUTABLE}
  PRIVATE ${LIBRARY}-shared pugixml::pugixml spdlog::spdlog klib::klib
          ${Boost_LIBRARIES} CLI11::CLI11)

add_executable(${MASIRO_EXECUTABLE} masiro.cpp)
target_link_libraries(${MASIRO_EXECUTABLE} PRIVATE ${LIBRARY}-shared klib::klib
                                                   CLI11::CLI11)

add_executable(${XHTML_EXECUTABLE} xhtml.cpp)
target_link_libraries(
  ${XHTML_EXECUTABLE} PRIVATE ${LIBRARY}-shared pugixml::pugixml klib::klib
                              CLI11::CLI11)

add_executable(${ADDITION_EXECUTABLE} addition.cpp)
target_link_libraries(${ADDITION_EXECUTABLE} PRIVATE ${LIBRARY}-shared
                                                     klib::klib CLI11::CLI11)

add_executable(${CIWEIMAO_EXECUTABLE} ciweimao.cpp json.cpp)
target_link_libraries(
  ${CIWEIMAO_EXECUTABLE}
  PRIVATE ${LIBRARY}-shared
          klib::klib
          spdlog::spdlog
          ${Boost_LIBRARIES}
          pugixml::pugixml
          CLI11::CLI11
          simdjson::simdjson)

add_executable(${SFACG_EXECUTABLE} sfacg.cpp json.cpp)
target_link_libraries(
  ${SFACG_EXECUTABLE}
  PRIVATE ${LIBRARY}-shared
          klib::klib
          fmt::fmt
          spdlog::spdlog
          ${Boost_LIBRARIES}
          CLI11::CLI11
          simdjson::simdjson)
