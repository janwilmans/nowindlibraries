// SdlMainWindows.cpp, the real main() of emuv1 on Windows
//
// SDL.h renames the program's main() to SDL_main(); SDL's SDLmain.lib normally provides the real one, but the prebuilt
// one (SDL 1.2.15) was made for the Visual C++ runtime before 2015 and does not link with a current one (__iob_func).
// This does the part of it emuv1 needs: give SDL the module handle (for its window), then run the program.

#include "SDL.h"

#include <windows.h>

#undef main

int main(int argc, char* argv[])
{
    SDL_SetModuleHandle(GetModuleHandle(nullptr));
    return SDL_main(argc, argv);
}
