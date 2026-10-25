Prebuilt third-party libraries for 64-bit Windows builds
=========================================================

So a Windows build does not have to download or build them (cmake/Dependencies.cmake):

  sdl/          SDL 1.2.15, the official Visual C++ development package
                (https://www.libsdl.org/release/SDL-devel-1.2.15-VC.zip): include/, lib/SDL.lib,
                bin/SDL.dll; LGPL 2.1, see COPYING.txt
  sdl_ttf/      SDL_ttf 2.0.11, the official Visual C++ development package
                (https://www.libsdl.org/projects/SDL_ttf/release/SDL_ttf-devel-2.0.11-VC.zip):
                include/, lib/SDL_ttf.lib, bin/SDL_ttf.dll and the libfreetype-6.dll and zlib1.dll it needs;
                zlib license, see COPYING.txt and bin/LICENSE.*.txt
  googletest/   googletest 1.15.2 (https://github.com/google/googletest), built with this project by Visual
                Studio 2022, x64, without debug information: lib/release (/MD, for Release and RelWithDebInfo)
                and lib/debug (/MDd, for Debug) gtest.lib and gtest_main.lib, include/; BSD 3-clause, see LICENSE

SDLmain.lib is not included: it needs the Visual C++ runtime from before 2015, emuv1 has its own main() for
Windows (emuv1/SdlMainWindows.cpp).

Updating: configure with -DNOWIND_FETCH_DEPENDENCIES=ON (the versions are in cmake/Dependencies.cmake). That
downloads them and copies the headers, libraries and DLLs here; the googletest libraries are copied when they are
built, so build both a Debug and a Release tree. Then commit what 'git status' shows here, if you want to keep it.
