# Project Info
SET(PROJECT_NAME_SHORT   "SDP")
SET(PROJECT_NAME_LONG    "SeismicDataPlayback")
SET(PROJECT_DESCRIPTION  "Seismologic tool for reviewing data")
SET(PROJECT_COPYRIGHT    "Copyright (C) 2015 OVSM-dev")
SET(PROJECT_CONTACT      "stephen.roselia@supinfo.com;dev@ovmp.martinique.univ-ag.fr")
SET(PROJECT_VENDOR       "OVSM")
SET(ORG_WEBSITE          "http://www.ipgp.fr")

STRING(TOLOWER ${PROJECT_NAME_SHORT} PROJECT_NAME_SHORT_LOWER)
PROJECT(${PROJECT_NAME_SHORT_LOWER})

# Codename of the Project
SET(CODENAME "Rupert Pumpkin")

#IF(APPLE)
#    # enable @rpath in the install name for any shared library being built
#    # note: it is planned that a future version of CMake will enable this by default
#    SET(CMAKE_MACOSX_RPATH 1)
#ENDIF(APPLE)

MESSAGE(STATUS "--------------------------------------------------------------")
MESSAGE(STATUS "        Configuration of project ${PROJECT_NAME_LONG}")
MESSAGE(STATUS "                         ${PROJECT_COPYRIGHT}")
MESSAGE(STATUS "                                  ${ORG_WEBSITE}")
MESSAGE(STATUS "--------------------------------------------------------------")

# Configuring build type
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    MESSAGE(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}'")
ELSE(NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "Build type set to '${CMAKE_BUILD_TYPE}'")
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(WIN32)
    # Building in release for WIN32
    SET(CMAKE_BUILD_TYPE "Release")
    SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELEASE QT_NO_DEBUG)
    # Use mingw's sprintf instead of windows's
    ADD_DEFINITIONS( -D__USE_MINGW_ANSI_STDIO=1 )
ENDIF(WIN32)

# On Mac OS build a universal 'release' binary
IF(APPLE)
    # enable @rpath in the install name for any shared library being built
    # note: it is planned that a future version of CMake will enable this by default
    SET(CMAKE_MACOSX_RPATH 1)
    SET(CMAKE_BUILD_TYPE "Release")
    SET(CMAKE_OSX_ARCHITECTURES x86_64)
ENDIF(APPLE)


# For projects using Git, use git version patch
IF("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    SET(PATCH_OUT "0")
ELSE("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    # Find Git Version Patch
    FIND_PROGRAM(GIT git)
    IF(GIT)
        EXECUTE_PROCESS(
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${GIT} rev-parse --short HEAD 
            OUTPUT_VARIABLE GIT_OUT OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        SET(PATCH_OUT "0-git${GIT_OUT}")
    ELSE(GIT)
        SET(PATCH_OUT "0")
    ENDIF(GIT)
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "Release")

SET(PROJECT_VERSION_PATCH ${PATCH_OUT})

# Project Version
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
SET(CPACK_PACKAGE_FILE_NAME "sdp-${CPACK_PACKAGE_VERSION}")
SET(PROJECT_VERSION_MAJOR ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR})
SET(PROJECT_VERSION ${CPACK_PACKAGE_VERSION})

MESSAGE(STATUS "Project's version: ${PROJECT_VERSION} ${CODENAME}")

# System on which project is built
SITE_NAME(HOSTNAME)
SET(SYSNAME ${CMAKE_SYSTEM}-${CMAKE_SYSTEM_PROCESSOR})

# CMake Settings
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# Set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})


# Path to contribs
SET(CONTRIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/contribs)


IF(NOT WIN32)
    SET(WITH_PROFILING FALSE CACHE BOOL "Build with profiling support" )
    # $ENV{HOME} seems to work better than ${HOME}
    SET(CMAKE_INSTALL_PREFIX $ENV{HOME}/sdp)
    MESSAGE(STATUS "Install dir set to: ${CMAKE_INSTALL_PREFIX}")
ENDIF(NOT WIN32)

# Linux intall fix
IF(NOT WIN32)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF(NOT WIN32)

# Search and include Qt4 parameters
# Set Qt min required version and add modules here
IF(WIN32)
    # FindQt4.cmake is so brain-dead when Xcompiling
    SET(QT_LIBRARY_DIR ${CONTRIBS_PATH}/lib)
    SET(QT_INCLUDE_DIR ${CONTRIBS_PATH}/include/qt4)
    ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtCore)
    ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtGui)
    ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtXml)
    ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtNetwork)
        ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/QtWebKit)
    ADD_DEFINITIONS( -I${CONTRIBS_PATH}/include/qt4/)
    FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtXml QtSvg QtNetwork REQUIRED )
ELSE(WIN32)
    FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtXml QtSvg QtNetwork REQUIRED )
ENDIF(WIN32)

IF (NOT WIN32)
    #FindQt4.cmake is so brain-dead when Xcompiling
    SET(SDP_LIBS ${QT_QTCORE_LIBRARY}
                     ${QT_QTGUI_LIBRARY}
                     ${QT_QTSVG_LIBRARY}
                     ${QT_QTNETWORK_LIBRARY}
                     ${QT_QTWEBKIT_LIBRARY}
                     ${QT_QTXML_LIBRARY}) # Add here any other QT module or other library
    INCLUDE(${QT_USE_FILE})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
ELSE (NOT WIN32)
    SET (SDP_LIBS -L${CONTRIBS_PATH}/lib # Manually specify paths
            ${CONTRIBS_PATH}/lib/libQtNetwork.a
            ${CONTRIBS_PATH}/lib/libQtXml.a
            ${CONTRIBS_PATH}/lib/libQtGui.a
            ${CONTRIBS_PATH}/lib/libQtCore.a
            ws2_32 winmm imm32 libeay32 ssleay32 crypt32 gdi32)
ENDIF(NOT WIN32)

# Copy the sh launcher
#IF(NOT WIN32)
#    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdp.sh" "${CMAKE_CURRENT_BINARY_DIR}/sdp" COPYONLY)
#ENDIF(NOT WIN32)

# Layout of sdp installation
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set common dirs for MAC and UNIX
SET(DEFAULT_BIN_SUBDIR     bin)
SET(DEFAULT_LIB_SUBDIR     lib)
SET(DEFAULT_DATA_SUBDIR    share)
SET(DEFAULT_SRC_SUBDIR     src)
SET(DEFAULT_INCLUDE_SUBDIR include)
SET(DEFAULT_CONFIG_SUBDIR  etc)


# Set SDP path params
SET(SDP_BIN_SUBDIR     "${DEFAULT_BIN_SUBDIR}"        CACHE STRING "Subdirectory where executables will be installed")
SET(SDP_LIB_SUBDIR     "${DEFAULT_LIB_SUBDIR}"        CACHE STRING "Subdirectory where libraries will be installed")
SET(SDP_DATA_SUBDIR    "${DEFAULT_DATA_SUBDIR}"       CACHE STRING "Subdirectory where SDP's data will be installed")
SET(SDP_SRC_SUBDIR     "${DEFAULT_SRC_SUBDIR}"        CACHE STRING "Subdirectory where SDP's source is")
SET(SDP_INCLUDE_SUBDIR "${DEFAULT_INCLUDE_SUBDIR}"    CACHE STRING "Subdirectory where header files will be installed")
SET(SDP_CONFIG_SUBDIR  "${DEFAULT_CONFIG_SUBDIR}"     CACHE STRING "Subdirectory where configuration files will be installed")

# Full paths for the installation
SET(SDP_BIN_DIR     ${CMAKE_INSTALL_PREFIX}/${SDP_BIN_SUBDIR})
SET(SDP_LIB_DIR     ${CMAKE_INSTALL_PREFIX}/${SDP_LIB_SUBDIR})
SET(SDP_SHARE_DIR   ${CMAKE_INSTALL_PREFIX}/${SDP_DATA_SUBDIR})
SET(SDP_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${SDP_INCLUDE_SUBDIR})
SET(SDP_CONFIG_DIR  ${CMAKE_INSTALL_PREFIX}/${SDP_CONFIG_SUBDIR})
SET(SDP_DATA_DIR    ${CMAKE_INSTALL_PREFIX}/${SDP_DATA_SUBDIR}/sdp/)
SET(SDP_DOC_DIR     ${CMAKE_INSTALL_PREFIX}/${SDP_DATA_SUBDIR}/doc/)
SET(SDP_EFFECTS_DIR ${CMAKE_INSTALL_PREFIX}/${SDP_LIB_SUBDIR}/sdp/effects/)
SET(SDP_DESKTOP_DIR ${CMAKE_INSTALL_PREFIX}/${SDP_DATA_SUBDIR}/applications/)
SET(SDP_PIXMAPS_DIR ${CMAKE_INSTALL_PREFIX}/${SDP_DATA_SUBDIR}/pixmaps/)
SET(SDP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(SDP_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

# Build internal libs as shared libs
OPTION(SHARED_LIBRARIES "Build shared libraries" ON)
#SET(SHARED_LIBRARIES 1)

# Manual page - makes sense only on unix systems
IF(UNIX)
    SET(DEFAULT_MANUAL_SUBDIR  ${SDP_DATA_SUBDIR}/man)
    SET(SDP_MANUAL_SUBDIR  "${DEFAULT_MANUAL_SUBDIR}"  CACHE STRING "Subdirectory where manual files will be installed")
    MARK_AS_ADVANCED(SDP_MANUAL_SUBDIR)
    SET(SDP_MANUAL_DIR  ${CMAKE_INSTALL_PREFIX}/${SDP_MANUAL_SUBDIR})
ENDIF(UNIX)

# Include macros
INCLUDE (${CMAKE_MODULE_PATH}/SDP.cmake)

# Recursively add sub dirs
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(ts)


# Copy stuff to doc subdirectory
INSTALL(
    FILES AUTHORS COPYING INSTALL NEWS README TRANSLATORS
    DESTINATION ${SDP_DOC_DIR}
)

# Copy html to doc subdirectory
INSTALL(
    CODE "execute_process(COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/share/help ${SDP_DOC_DIR})"
)

# Copy database file if there is none
IF(EXISTS "${SDP_SHARE_DIR}/sdp.sqlite")
    MESSAGE(STATUS "Skipping copy of empty database, file already present.")
ELSE(EXISTS "${SDP_SHARE_DIR}/sdp.sqlite")
    MESSAGE(STATUS "Copying empty database file into ${SDP_SHARE_DIR}/")
    INSTALL(
        FILES share/sdp.sqlite
        DESTINATION ${SDP_SHARE_DIR}
    )
ENDIF(EXISTS "${SDP_SHARE_DIR}/sdp.sqlite")

# Copy .desktop file
INSTALL(
    FILES share/sdp.desktop
    DESTINATION ${SDP_DESKTOP_DIR}
)

# Copy the SDP pixmap
INSTALL(
    FILES share/sdp.png
    DESTINATION ${SDP_PIXMAPS_DIR}
)

# Copy the share directory
#INSTALL (DIRECTORY share/ DESTINATION ${SDP_DATA_DIR}
#         PATTERN "*.desktop" EXCLUDE
#         PATTERN "*.ico" EXCLUDE
#         PATTERN "*.icns" EXCLUDE
#         PATTERN "sdp.png" EXCLUDE)

#############################################################
# Enable packaging

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME_SHORT})
SET(CPACK_PACKAGE_DESCRIPTION "${PROJECT_NAME_LONG}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME_SHORT} - ${PROJECT_DESCRIPTION}")
SET(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
SET(CPACK_PACKAGE_CONTACT ${PROJECT_CONTACT})
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README)
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# Force Package Name
SET(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${CPACK_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})

# NSIS Windows installer
IF(WIN32 AND NOT UNIX)
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backslashes.
    SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/share\\\\sdp.png")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "\\\\sdp.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} SDP")
    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.ipgp.fr")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.ipgp.fr")
    SET(CPACK_NSIS_CONTACT "stephen.roselia AT supinfo.com")
    SET(CPACK_NSIS_MODIFY_PATH ON)
ENDIF(WIN32 AND NOT UNIX)

# RPM packages
INCLUDE(${CMAKE_MODULE_PATH}/RpmBuild.cmake)
IF(RPMBUILD_FOUND AND NOT WIN32)
    SET(CPACK_GENERATOR "RPM")
    SET(CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    SET(CPACK_RPM_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
    SET(CPACK_RPM_PACKAGE_VERSION ${PROJECT_VERSION})
    SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3+ License")
    SET(CPACK_RPM_PACKAGE_GROUP ${PROJECT_VENDOR})
    SET(CPACK_RPM_PACKAGE_VENDOR ${PROJECT_VENDOR})
    SET(CPACK_RPM_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
    SET(CPACK_RPM_PACKAGE_DEPENDS "qt ( >= 4.6 ), qt-x11 ( >= 4.6 )") # Specify dependencies here
    SET(CPACK_SET_DESTDIR TRUE)
ENDIF(RPMBUILD_FOUND AND NOT WIN32)

# Debian packages
INCLUDE (${CMAKE_MODULE_PATH}/DpkgBuild.cmake)
IF(DPKG_FOUND AND NOT WIN32)
    SET(CPACK_GENERATOR "DEB")
    SET(CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
    SET(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
    SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
    SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    SET(CPACK_DEBIAN_PACKAGE_DEBUG ON)
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS ${PACKAGE_REQUIRES})
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-gui ( >= 4.6 ), libqt4-xml ( >= 4.6 )") # Specify dependencies here
    SET(CPACK_SET_DESTDIR TRUE)
ENDIF(DPKG_FOUND AND NOT WIN32)

# Mac App Bundle
IF(APPLE)
    SET(CPACK_BINARY_DRAGNDROP ON)
#    SET(CPACK_GENERATOR "DragNDrop")
    # Libraries are bundled directly
    SET(CPACK_COMPONENT_LIBRARIES_HIDDEN TRUE)
    # Bundle Properties
    SET(MACOSX_BUNDLE_BUNDLE_NAME sdp)
    SET(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "Version ${PROJECT_VERSION}")
    SET(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2015, OVSM-dev. All Rights Reserved.")
ENDIF(APPLE)


SET(CPACK_PACKAGE_EXECUTABLES "sdp" "SDP")
INCLUDE(CPack)
