# Third-party dependencies: SDL 1.2 and SDL_ttf 2.0 (emuv1) and googletest (the tests).
#
# NOWIND_FETCH_DEPENDENCIES=ON downloads them with FetchContent (the default on Windows, where there is no package
# manager to install them from); OFF uses the installed ones (the default elsewhere: apt install libsdl1.2-dev
# libsdl-ttf2.0-dev libgtest-dev). Either way the result is the same targets:
#   SDL::SDL, SDL::ttf   the libraries (emuv1lib); SDLmain is not used, emuv1 has its own main() for Windows
#                        (emuv1/SdlMainWindows.cpp), elsewhere it is part of the found SDL library
#   GTest::gtest, GTest::gtest_main
# and nowind_copy_runtime_dlls(<target>), which copies the DLLs next to a program that needs them (Windows).

option(NOWIND_FETCH_DEPENDENCIES "Download SDL 1.2, SDL_ttf and googletest with FetchContent" ${WIN32})

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
  FetchContent_MakeAvailable(googletest)

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
      set(SDL_ARCH x64)
    else()
      set(SDL_ARCH x86)
    endif()
    set(SDL_LIB_DIR ${sdl_SOURCE_DIR}/lib/${SDL_ARCH})
    set(SDL_TTF_LIB_DIR ${sdl_ttf_SOURCE_DIR}/lib/${SDL_ARCH})

    add_library(SDL::SDL SHARED IMPORTED)
    set_target_properties(SDL::SDL PROPERTIES
        IMPORTED_LOCATION ${SDL_LIB_DIR}/SDL.dll
        IMPORTED_IMPLIB ${SDL_LIB_DIR}/SDL.lib
        INTERFACE_INCLUDE_DIRECTORIES ${sdl_SOURCE_DIR}/include)
    add_library(SDL::ttf SHARED IMPORTED)
    set_target_properties(SDL::ttf PROPERTIES
        IMPORTED_LOCATION ${SDL_TTF_LIB_DIR}/SDL_ttf.dll
        IMPORTED_IMPLIB ${SDL_TTF_LIB_DIR}/SDL_ttf.lib
        INTERFACE_INCLUDE_DIRECTORIES ${sdl_ttf_SOURCE_DIR}/include
        INTERFACE_LINK_LIBRARIES SDL::SDL)
    # SDL_ttf.dll needs FreeType and zlib
    set(NOWIND_RUNTIME_DLLS ${SDL_LIB_DIR}/SDL.dll ${SDL_TTF_LIB_DIR}/SDL_ttf.dll ${SDL_TTF_LIB_DIR}/libfreetype-6.dll
        ${SDL_TTF_LIB_DIR}/zlib1.dll)
  endif()
endif()

if(NOT TARGET SDL::SDL)
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

# copies the third-party DLLs next to a program (before googletest lists its tests by running it)
function(nowind_copy_runtime_dlls target)
  if(NOWIND_RUNTIME_DLLS)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NOWIND_RUNTIME_DLLS} $<TARGET_FILE_DIR:${target}>
        VERBATIM)
  endif()
endfunction()
