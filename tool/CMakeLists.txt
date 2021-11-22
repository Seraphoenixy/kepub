find_package(spdlog REQUIRED)
find_package(CLI11 REQUIRED)

add_executable(${ESJZONE_EXECUTABLE} esjzone.cpp)
target_link_libraries(
  ${ESJZONE_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared pugixml::pugixml spdlog::spdlog klib::klib
          ${Boost_LIBRARIES} CLI11::CLI11)

add_executable(${MASIRO_EXECUTABLE} masiro.cpp)
target_link_libraries(${MASIRO_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared
                                                   klib::klib CLI11::CLI11)

add_executable(${ADDITION_EXECUTABLE} addition.cpp)
target_link_libraries(${ADDITION_EXECUTABLE} PRIVATE ${KEPUB_LIBRARY}-shared
                                                     klib::klib CLI11::CLI11)

add_executable(${CIWEIMAO_EXECUTABLE} ciweimao/ciweimao.cpp ciweimao/json.cpp)
target_link_libraries(
  ${CIWEIMAO_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared
          klib::klib
          spdlog::spdlog
          ${Boost_LIBRARIES}
          pugixml::pugixml
          CLI11::CLI11
          simdjson::simdjson)

add_executable(${SFACG_EXECUTABLE} sfacg/sfacg.cpp sfacg/json.cpp)
target_link_libraries(
  ${SFACG_EXECUTABLE}
  PRIVATE ${KEPUB_LIBRARY}-shared
          klib::klib
          fmt::fmt
          spdlog::spdlog
          ${Boost_LIBRARIES}
          CLI11::CLI11
          simdjson::simdjson)
