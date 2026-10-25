# nwhostapp: the command line host application

# getopt, a copy from glibc (third-party, without the project warnings); msvcrt has none, so on Windows its getopt.h
# is used too, elsewhere the system's (emuv1 links it on Windows)
add_library(getopt STATIC getopt.c getopt1.c)
target_include_directories(getopt INTERFACE $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}>)
target_compile_options(getopt PRIVATE $<IF:$<C_COMPILER_ID:MSVC>,/w,-w>)

# the MACOSX_BUNDLE supports the make install feature, it is ignored on other platforms
add_executable(nwhostapp MACOSX_BUNDLE mainUsbHost.cpp HostApp.cpp)
target_link_libraries(nwhostapp PRIVATE nowind nwgeneral getopt project::warnings_options project::warning_suppressions)

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/nwhostapp")
set(library_dest_dir distr)
if(APPLE)
  set(library_dest_dir nwhostapp.app/Contents/MacOS)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/nwhostapp.app")
endif()
if(WIN32)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/nwhostapp.exe")
endif()

#--------------------------------------------------------------------------------
# Install the application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
install(TARGETS nwhostapp
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

#--------------------------------------------------------------------------------
# Install needed libraries by copying from library installation directories
if(NOT WIN32)
  install(FILES ${FTDI_LIB} DESTINATION ${library_dest_dir} COMPONENT Runtime)
endif()

# this was suppose to copy the real files too, not just the symlinks, but it doesnt work,
# just copy them manually once
if(0)

install(CODE "
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
endif()

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
