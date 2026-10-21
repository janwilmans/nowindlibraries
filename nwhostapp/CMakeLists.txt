PROJECT(nwhostapp)
cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(../general)
INCLUDE_DIRECTORIES(../nwhost)
INCLUDE_DIRECTORIES(../ftdx)
INCLUDE_DIRECTORIES(../nowind)

if(WIN32)
  # static x64 FTD2XX library
  set(FTDI ${CMAKE_CURRENT_SOURCE_DIR}/../ftdx/FTD2XX-msc/FTD2XX.lib)
else()
  find_library(FTDI ftdi NAMES ftdi1)
endif()

MESSAGE ("find_library FTDI: ${FTDI}")

find_path(FTDI_INCLUDE_DIR
    NAMES ftdi.h
    PATH_SUFFIXES libftdi1
    REQUIRED
)

target_include_directories(ftdx PRIVATE ${FTDI_INCLUDE_DIR})

# the MACOSX_BUNDLE supports the make install feature, it is ignored on other platforms
ADD_EXECUTABLE(nwhostapp MACOSX_BUNDLE mainUsbHost.cpp HostApp.cpp getopt.c getopt1.c)

if(WIN32)
  # msvcrt has no getopt, use the bundled glibc copy
  target_include_directories(nwhostapp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

target_link_libraries(nwhostapp nowind nwhost ftdx nwgeneral ${FTDI})

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/nwhostapp")
SET(library_dest_dir distr)
IF(APPLE)
  SET(library_dest_dir nwhostapp.app/Contents/MacOS)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/nwhostapp.app")
ENDIF(APPLE)
IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/nwhostapp.exe")
ENDIF(WIN32)

#--------------------------------------------------------------------------------
# Install the application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS nwhostapp
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

#--------------------------------------------------------------------------------
# Install needed libraries by copying from library installation directories
if(NOT WIN32)
  INSTALL(FILES ${FTDI} DESTINATION ${library_dest_dir} COMPONENT Runtime)
endif()

# this was suppose to copy the real files too, not just the symlinks, but it doesnt work,
# just copy them manually once
if (0)

INSTALL(CODE "
        file(GLOB_RECURSE LIBS
        \"${CMAKE_INSTALL_PREFIX}/\${library_dest_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")

        foreach(item \${LIBS})
        message(\"check: \${item}\")
        get_filename_component(absfile \${item} REALPATH)
        if (NOT( \${item} STREQUAL \${absfile}))
        message(\"  real file: \${absfile}\")
	file(COPY \${absfile} DESTINATION \"${CMAKE_INSTALL_PREFIX}/\${library_dest_dir}\")
        list(APPEND REALFILES \${absfile})
        endif()
        endforeach()
")
ENDIF()

# # Now the work of copying dependencies into the bundle/package
# # The quotes are escaped and variables to use at install time have their $ escaped
# INSTALL(CODE "
#     file(GLOB_RECURSE LIBS
#       \"${CMAKE_INSTALL_PREFIX}/${library_dest_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
#     include(BundleUtilities)
#     fixup_bundle(\"${APPS}\" \"\${LIBS}\" \"\")
#     " COMPONENT Runtime)

# # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# # where CPackConfig.cmake is created by including CPack
# # And then there's ways to customize this as well
# set(CPACK_BINARY_DRAGNDROP ON)
# include(CPack)

