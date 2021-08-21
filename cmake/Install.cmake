# ---------------------------------------------------------------------------------------
# Install executable
# ---------------------------------------------------------------------------------------
include(GNUInstallDirs)

# https://stackoverflow.com/questions/30398238/cmake-rpath-not-working-could-not-find-shared-object-file
set_target_properties(
  ${DEMONOVEL_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${ESJZONE_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${MASIRO_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${XHTML_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${ADDITION_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${CIWEIMAO_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${SFACG_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

install(
  TARGETS ${ADDITION_EXECUTABLE}
          ${DEMONOVEL_EXECUTABLE}
          ${ESJZONE_EXECUTABLE}
          ${MASIRO_EXECUTABLE}
          ${XHTML_EXECUTABLE}
          ${CIWEIMAO_EXECUTABLE}
          ${SFACG_EXECUTABLE}
          ${LIBRARY}-shared
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ---------------------------------------------------------------------------------------
# Support creation of installable packages
# ---------------------------------------------------------------------------------------
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html
# https://cmake.org/cmake/help/latest/module/CPack.html
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_INSTALL_CMAKE_PROJECTS ${KEPUB_BINARY_DIR} ${PROJECT_NAME} ALL .)

# https://cmake.org/cmake/help/latest/cpack_gen/deb.html
set(CPACK_PACKAGE_CONTACT "kaiser <KaiserLancelot123@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Generate epub")
set(CPACK_PACKAGE_VERSION
    ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# https://cmake.org/cmake/help/latest/manual/cpack-generators.7.html
set(CPACK_GENERATOR "TGZ;DEB")

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# https://cmake.org/cmake/help/latest/module/InstallRequiredSystemLibraries.html
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
    "/usr/local/lib/libicuuc.so.69.1;/usr/local/lib/libicui18n.so.69.1;/usr/local/lib/libicudata.so.69.1"
)
include(InstallRequiredSystemLibraries)

include(CPack)
