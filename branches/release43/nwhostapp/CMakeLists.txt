PROJECT(nwhostapp)
cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(../general)
INCLUDE_DIRECTORIES(../nwhost)
INCLUDE_DIRECTORIES(../ftdx)
INCLUDE_DIRECTORIES(../nowind)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_library(FTDI ftdi NAMES ftdi PATHS /usr/lib /usr/local/lib /opt/local/lib /usr/lib/i386-linux-gnu)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

MESSAGE ("find_library FTDI: ${FTDI} <end>")
MESSAGE ("using Boost: ${Boost_LIBRARIES} <end>")

# the MACOSX_BUNDLE supports the make install feature, it is ignored on other platforms
ADD_EXECUTABLE(nwhostapp MACOSX_BUNDLE mainUsbHost.cpp HostApp.cpp getopt.c getopt1.c)

target_link_libraries(nwhostapp nowind nwhost ftdx nwgeneral ${FTDI} ${Boost_LIBRARIES})

IF(APPLE)

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/nwhostapp")
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
# INSTALL(DIRECTORY "${Boost_LIBRARY_DIRS}" DESTINATION ${library_dest_dir} COMPONENT Runtime)
INSTALL(FILES ${Boost_LIBRARIES} DESTINATION ${library_dest_dir} COMPONENT Runtime)
INSTALL(FILES ${FTDI} DESTINATION ${library_dest_dir} COMPONENT Runtime)

file(GLOB_RECURSE LIBS
      ${CMAKE_INSTALL_PREFIX}/${library_dest_dir}/*) 

# directories to look for dependencies
SET(DIRS ${Boost_LIBRARY_DIRS})

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
INSTALL(CODE "
    file(GLOB_RECURSE LIBS2
      \"${CMAKE_INSTALL_PREFIX}/${library_dest_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"${LIBS}\" \"${DIRS}\")
    " COMPONENT Runtime)


# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)
ENDIF(APPLE)

