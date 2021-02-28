if(EPUB_INSTALL)
  message(STATUS "Generate the install target")

  include(GNUInstallDirs)

  # ---------------------------------------------------------------------------------------
  # Install executable
  # ---------------------------------------------------------------------------------------
  if(EPUB_BUILD_EXECUTABLE OR EPUB_BUILD_ALL)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR};$\{ORIGIN\}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
         "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
    if(${isSystemDir} STREQUAL "-1")
      set(CMAKE_INSTALL_RPATH
          "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR};$\{ORIGIN\}")
    endif()

    install(TARGETS ${EXECUTABLE_TARGETS} DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()

  # ---------------------------------------------------------------------------------------
  # Include files
  # ---------------------------------------------------------------------------------------
  install(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(
    TARGETS ${LIBRARY_TARGETS}
    EXPORT EPUBEXPORTS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  # ---------------------------------------------------------------------------------------
  # Install CMake config files
  # ---------------------------------------------------------------------------------------
  set(EPUB_EXPORT_DEST_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY}")
  set(EPUB_CONFIG_TARGETS_FILE "${LIBRARY}ConfigTargets.cmake")

  install(
    EXPORT EPUBEXPORTS
    DESTINATION ${EPUB_EXPORT_DEST_DIR}
    NAMESPACE ${LIBRARY}::
    FILE ${EPUB_CONFIG_TARGETS_FILE})

  set(EPUB_PROJECT_CONFIG_IN
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${LIBRARY}Config.cmake.in")
  set(EPUB_PROJECT_CONFIG_OUT
      "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY}Config.cmake")
  set(EPUB_VERSION_CONFIG_FILE
      "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY}ConfigVersion.cmake")

  include(CMakePackageConfigHelpers)
  configure_file(${EPUB_PROJECT_CONFIG_IN} ${EPUB_PROJECT_CONFIG_OUT} @ONLY)

  write_basic_package_version_file(${EPUB_VERSION_CONFIG_FILE}
                                   COMPATIBILITY SameMajorVersion)
  install(FILES ${EPUB_PROJECT_CONFIG_OUT} ${EPUB_VERSION_CONFIG_FILE}
          DESTINATION ${EPUB_EXPORT_DEST_DIR})

  # ---------------------------------------------------------------------------------------
  # Project information
  # ---------------------------------------------------------------------------------------
  set(EPUB_VENDOR "kaiser")
  set(EPUB_CONTACT "kaiser <KaiserLancelot123@gmail.com>")
  set(EPUB_PROJECT_URL "https://github.com/KaiserLancelot/epub")
  set(EPUB_DESCRIPTION_SUMMARY "Generate epub")

  # ---------------------------------------------------------------------------------------
  # Install pkg-config file
  # ---------------------------------------------------------------------------------------
  set(EPUB_PKG_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
  set(EPUB_PKG_CONFIG_IN "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${LIBRARY}.pc.in")
  set(EPUB_PKG_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY}.pc")
  set(EPUB_PKG_CFLAGS "-std=c++20")

  get_target_property(EPUB_PKG_CONFIG_DEFINES ${LIBRARY}
                      INTERFACE_COMPILE_DEFINITIONS)
  string(REPLACE ";" " -D" EPUB_PKG_CONFIG_DEFINES ${EPUB_PKG_CONFIG_DEFINES})
  string(CONCAT EPUB_PKG_CONFIG_DEFINES "-D" ${EPUB_PKG_CONFIG_DEFINES})
  configure_file(${EPUB_PKG_CONFIG_IN} ${EPUB_PKG_CONFIG} @ONLY)
  install(FILES ${EPUB_PKG_CONFIG} DESTINATION ${EPUB_PKG_CONFIG_INSTALL_DIR})

  # ---------------------------------------------------------------------------------------
  # Support creation of installable packages
  # ---------------------------------------------------------------------------------------
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
  set(CPACK_INSTALL_CMAKE_PROJECTS ${CMAKE_CURRENT_BINARY_DIR} ${LIBRARY} ALL .)

  set(CPACK_PROJECT_URL ${EPUB_PROJECT_URL})
  set(CPACK_PACKAGE_VENDOR ${EPUB_VENDOR})
  set(CPACK_PACKAGE_CONTACT ${EPUB_CONTACT})
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${EPUB_DESCRIPTION_SUMMARY})
  set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
  set(CPACK_PACKAGE_VERSION
      ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
  )
  if(PROJECT_VERSION_TWEAK)
    set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION}.${PROJECT_VERSION_TWEAK})
  endif()
  set(CPACK_PACKAGE_RELOCATABLE
      ON
      CACHE BOOL "Build relocatable package")

  set(CPACK_GENERATOR
      "TGZ;DEB"
      CACHE STRING "Semicolon separated list of generators")

  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
      "/usr/local/lib/libfmt.so.7.1.3;/usr/local/lib/libboost_program_options.so.1.75.0;/usr/local/lib/libicuuc.so.68.2;/usr/local/lib/libicui18n.so.68.2;/usr/local/lib/libicudata.so.68.2"
  )
  include(InstallRequiredSystemLibraries)

  include(CPack)
endif()
