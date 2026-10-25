# Third-party dependencies: SDL 1.2 and SDL_ttf 2.0 (emuv1) and googletest (the tests).
#
# Windows (x64): thirdparty/windows-x64 holds a copy of the prebuilt libraries, so normally nothing is downloaded or
# built. NOWIND_FETCH_DEPENDENCIES=ON downloads them instead (FetchContent: the official SDL 1.2.15 and SDL_ttf 2.0.11
# Visual C++ packages, googletest 1.15.2 built with the project) and refreshes that copy with them: the files at
# configure time, the googletest libraries when they are built (lib/debug or lib/release, by the build type). Commit
# the copy when 'git status' shows a change you want to keep.
#
# Elsewhere: the installed packages (apt install libsdl1.2-dev libsdl-ttf2.0-dev libgtest-dev);
# NOWIND_FETCH_DEPENDENCIES=ON downloads and builds googletest.
#
# Either way the result is the same targets:
#   SDL::SDL, SDL::ttf   the libraries (emuv1lib); SDLmain is not used, emuv1 has its own main() for Windows
#                        (emuv1/SdlMainWindows.cpp), elsewhere it is part of the found SDL library
#   GTest::gtest, GTest::gtest_main
# and nowind_copy_runtime_dlls(<target>), which copies the DLLs next to a program that needs them (Windows).

option(NOWIND_FETCH_DEPENDENCIES "Download SDL 1.2, SDL_ttf and googletest (and refresh thirdparty/windows-x64)" OFF)

set(NOWIND_THIRDPARTY ${CMAKE_SOURCE_DIR}/thirdparty/windows-x64)
set(NOWIND_RUNTIME_DLLS "")

if(NOWIND_FETCH_DEPENDENCIES)
  include(FetchContent)

  FetchContent_Declare(googletest
      URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
      URL_HASH SHA256=7b42b4d6ed48810c5362c265a17faebe90dc2373c885e5216439d37927f02926
      DOWNLOAD_EXTRACT_TIMESTAMP ON)
  # use the same C runtime as the rest of the build (MSVC), and do not install googletest with the project
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  # no debug information in the googletest libraries, it would make the copy in thirdparty/ ten times bigger
  set(saved_flags_debug "${CMAKE_CXX_FLAGS_DEBUG}")
  set(saved_flags_relwithdebinfo "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  string(REGEX REPLACE "/Z[7iI]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  string(REGEX REPLACE "/Z[7iI]" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  FetchContent_MakeAvailable(googletest)
  set(CMAKE_CXX_FLAGS_DEBUG "${saved_flags_debug}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${saved_flags_relwithdebinfo}")

  if(WIN32)
    # the official Visual C++ development packages, with 32 and 64-bit libraries and DLLs
    FetchContent_Declare(sdl
        URL https://www.libsdl.org/release/SDL-devel-1.2.15-VC.zip
        URL_HASH SHA256=7443ee5cd1bd2a0f9fa8c01acbeb4c779d0b03f9ce8a2aa47b0637d2a5461e50
        DOWNLOAD_EXTRACT_TIMESTAMP ON)
    FetchContent_Declare(sdl_ttf
        URL https://www.libsdl.org/projects/SDL_ttf/release/SDL_ttf-devel-2.0.11-VC.zip
        URL_HASH SHA256=59265578a83e23bdf0365042ba8dbe0fd19f35bbac0a8c44c56fc7323e97ed61
        DOWNLOAD_EXTRACT_TIMESTAMP ON)
    FetchContent_MakeAvailable(sdl sdl_ttf)

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(sdl_arch x64)
    else()
      set(sdl_arch x86)
    endif()
    set(SDL_INCLUDE ${sdl_SOURCE_DIR}/include)
    set(SDL_LIB_DIR ${sdl_SOURCE_DIR}/lib/${sdl_arch})
    set(SDL_BIN_DIR ${SDL_LIB_DIR})
    set(SDL_TTF_INCLUDE ${sdl_ttf_SOURCE_DIR}/include)
    set(SDL_TTF_LIB_DIR ${sdl_ttf_SOURCE_DIR}/lib/${sdl_arch})
    set(SDL_TTF_BIN_DIR ${SDL_TTF_LIB_DIR})

    if(sdl_arch STREQUAL x64)
      # refresh the copy in thirdparty/windows-x64 (the libraries and DLLs, the headers and the licenses)
      file(COPY ${SDL_INCLUDE}/ DESTINATION ${NOWIND_THIRDPARTY}/sdl/include)
      file(COPY ${SDL_LIB_DIR}/SDL.lib DESTINATION ${NOWIND_THIRDPARTY}/sdl/lib)
      file(COPY ${SDL_LIB_DIR}/SDL.dll DESTINATION ${NOWIND_THIRDPARTY}/sdl/bin)
      file(COPY ${sdl_SOURCE_DIR}/COPYING.txt ${sdl_SOURCE_DIR}/README-SDL.txt DESTINATION ${NOWIND_THIRDPARTY}/sdl)
      file(COPY ${SDL_TTF_INCLUDE}/ DESTINATION ${NOWIND_THIRDPARTY}/sdl_ttf/include)
      file(COPY ${SDL_TTF_LIB_DIR}/SDL_ttf.lib DESTINATION ${NOWIND_THIRDPARTY}/sdl_ttf/lib)
      file(COPY ${SDL_TTF_LIB_DIR}/SDL_ttf.dll ${SDL_TTF_LIB_DIR}/libfreetype-6.dll ${SDL_TTF_LIB_DIR}/zlib1.dll
                ${SDL_TTF_LIB_DIR}/LICENSE.freetype.txt ${SDL_TTF_LIB_DIR}/LICENSE.zlib.txt
           DESTINATION ${NOWIND_THIRDPARTY}/sdl_ttf/bin)
      file(COPY ${sdl_ttf_SOURCE_DIR}/COPYING.txt ${sdl_ttf_SOURCE_DIR}/README.txt DESTINATION ${NOWIND_THIRDPARTY}/sdl_ttf)
      file(COPY ${googletest_SOURCE_DIR}/googletest/include/ DESTINATION ${NOWIND_THIRDPARTY}/googletest/include)
      file(COPY ${googletest_SOURCE_DIR}/LICENSE DESTINATION ${NOWIND_THIRDPARTY}/googletest)
      # the googletest libraries, once built (a POST_BUILD step can only be added in googletest's own directory)
      set(gtest_copy_dir ${NOWIND_THIRDPARTY}/googletest/lib/$<IF:$<CONFIG:Debug>,debug,release>)
      add_custom_target(googletestupdate ALL
          COMMAND ${CMAKE_COMMAND} -E make_directory ${gtest_copy_dir}
          COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:gtest> $<TARGET_FILE:gtest_main> ${gtest_copy_dir}
          DEPENDS gtest gtest_main
          COMMENT "Updating thirdparty/windows-x64/googletest/lib"
          VERBATIM)
    endif()
  endif()
elseif(WIN32)
  if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "thirdparty/windows-x64 only has 64-bit libraries, use -DNOWIND_FETCH_DEPENDENCIES=ON")
  endif()
  set(SDL_INCLUDE ${NOWIND_THIRDPARTY}/sdl/include)
  set(SDL_LIB_DIR ${NOWIND_THIRDPARTY}/sdl/lib)
  set(SDL_BIN_DIR ${NOWIND_THIRDPARTY}/sdl/bin)
  set(SDL_TTF_INCLUDE ${NOWIND_THIRDPARTY}/sdl_ttf/include)
  set(SDL_TTF_LIB_DIR ${NOWIND_THIRDPARTY}/sdl_ttf/lib)
  set(SDL_TTF_BIN_DIR ${NOWIND_THIRDPARTY}/sdl_ttf/bin)

  # googletest: a release and a debug build, they use different C++ runtimes (/MD, /MDd)
  set(gtest_lib_dir ${NOWIND_THIRDPARTY}/googletest/lib)
  add_library(GTest::gtest STATIC IMPORTED)
  set_target_properties(GTest::gtest PROPERTIES
      IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
      IMPORTED_LOCATION_DEBUG ${gtest_lib_dir}/debug/gtest.lib
      IMPORTED_LOCATION_RELEASE ${gtest_lib_dir}/release/gtest.lib
      MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
      MAP_IMPORTED_CONFIG_MINSIZEREL Release
      INTERFACE_INCLUDE_DIRECTORIES ${NOWIND_THIRDPARTY}/googletest/include)
  add_library(GTest::gtest_main STATIC IMPORTED)
  set_target_properties(GTest::gtest_main PROPERTIES
      IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
      IMPORTED_LOCATION_DEBUG ${gtest_lib_dir}/debug/gtest_main.lib
      IMPORTED_LOCATION_RELEASE ${gtest_lib_dir}/release/gtest_main.lib
      MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
      MAP_IMPORTED_CONFIG_MINSIZEREL Release
      INTERFACE_LINK_LIBRARIES GTest::gtest)
endif()

if(WIN32)
  add_library(SDL::SDL SHARED IMPORTED)
  set_target_properties(SDL::SDL PROPERTIES
      IMPORTED_LOCATION ${SDL_BIN_DIR}/SDL.dll
      IMPORTED_IMPLIB ${SDL_LIB_DIR}/SDL.lib
      INTERFACE_INCLUDE_DIRECTORIES ${SDL_INCLUDE})
  add_library(SDL::ttf SHARED IMPORTED)
  set_target_properties(SDL::ttf PROPERTIES
      IMPORTED_LOCATION ${SDL_TTF_BIN_DIR}/SDL_ttf.dll
      IMPORTED_IMPLIB ${SDL_TTF_LIB_DIR}/SDL_ttf.lib
      INTERFACE_INCLUDE_DIRECTORIES ${SDL_TTF_INCLUDE}
      INTERFACE_LINK_LIBRARIES SDL::SDL)
  # SDL_ttf.dll needs FreeType and zlib
  set(NOWIND_RUNTIME_DLLS ${SDL_BIN_DIR}/SDL.dll ${SDL_TTF_BIN_DIR}/SDL_ttf.dll ${SDL_TTF_BIN_DIR}/libfreetype-6.dll
      ${SDL_TTF_BIN_DIR}/zlib1.dll)
else()
  # CMake's FindSDL honours the SDLDIR and SDLTTFDIR environment variables to point at a local copy
  find_package(SDL)
  find_package(SDL_ttf)
  if(SDL_FOUND AND SDL_TTF_FOUND)
    if(NOT TARGET SDL::SDL)
      add_library(SDL::SDL INTERFACE IMPORTED)
      set_target_properties(SDL::SDL PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SDL_INCLUDE_DIR}"
                                                 INTERFACE_LINK_LIBRARIES "${SDL_LIBRARY}")
    endif()
    add_library(SDL::ttf INTERFACE IMPORTED)
    set_target_properties(SDL::ttf PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SDL_TTF_INCLUDE_DIRS}"
                                              INTERFACE_LINK_LIBRARIES "${SDL_TTF_LIBRARIES};SDL::SDL")
  endif()
endif()

if(NOT TARGET GTest::gtest_main)
  find_package(GTest)
endif()

# how to install what is missing (Windows has the copy in thirdparty/windows-x64)
if(NOT WIN32)
  if(NOT TARGET SDL::SDL OR NOT TARGET SDL::ttf)
    message(STATUS "SDL 1.2 and/or SDL_ttf 2.0 not found, emuv1 and its tests are not built; to install them:\n"
                   "     Debian/Ubuntu: sudo apt install libsdl1.2-dev libsdl-ttf2.0-dev\n"
                   "     Fedora:        sudo dnf install SDL-devel SDL_ttf-devel")
  endif()
  if(NOT TARGET GTest::gtest_main)
    message(STATUS "googletest not found, the tests are not built; to install it:\n"
                   "     Debian/Ubuntu: sudo apt install libgtest-dev\n"
                   "     Fedora:        sudo dnf install gtest-devel\n"
                   "     macOS:         brew install googletest\n"
                   "   or configure with -DNOWIND_FETCH_DEPENDENCIES=ON to download it")
  endif()
endif()

# copies the third-party DLLs next to a program (before googletest lists its tests by running it)
function(nowind_copy_runtime_dlls target)
  if(NOWIND_RUNTIME_DLLS)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NOWIND_RUNTIME_DLLS} $<TARGET_FILE_DIR:${target}>
        VERBATIM)
  endif()
endfunction()
