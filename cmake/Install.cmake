# ---------------------------------------------------------------------------------------
# Install executable
# ---------------------------------------------------------------------------------------
include(GNUInstallDirs)

# https://stackoverflow.com/questions/30398238/cmake-rpath-not-working-could-not-find-shared-object-file
set_target_properties(
  ${ESJZONE_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set_target_properties(
  ${KEPUB_EXECUTABLE}
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

set_target_properties(
  ${GEN_EPUB_EXECUTABLE}
  PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
             INSTALL_RPATH_USE_LINK_PATH TRUE)

set(FONTTOOLS_LOCATION "/usr/local/bin/pyftsubset")
install(
  FILES ${FONTTOOLS_LOCATION}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE)

install(
  TARGETS ${ADDITION_EXECUTABLE}
          ${ESJZONE_EXECUTABLE}
          ${KEPUB_EXECUTABLE}
          ${CIWEIMAO_EXECUTABLE}
          ${SFACG_EXECUTABLE}
          ${GEN_EPUB_EXECUTABLE}
          ${KEPUB_LIBRARY}-shared
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
set(CPACK_PACKAGE_CONTACT "Kaiser <KaiserLancelot123@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Crawl novels from esjzone, ciweimao and sfacg. Generate epub from txt file"
)
set(CPACK_PACKAGE_VERSION
    ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# https://cmake.org/cmake/help/latest/cpack_gen/archive.html
# https://cmake.org/cmake/help/latest/cpack_gen/deb.html
set(CPACK_GENERATOR "TZST;DEB")
# FIXME set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
