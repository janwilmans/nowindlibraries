# nowindgui: graphical host for the Nowind interface (Dear ImGui on SDL2), one executable.
#
# SDL2 and Dear ImGui are downloaded and built with the project (FetchContent, pinned by hash), SDL2 as a static
# library so there are no DLLs to ship. Enable with -DNOWIND_BUILD_GUI=ON.

include(FetchContent)

FetchContent_Declare(sdl2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.9/SDL2-2.30.9.tar.gz
    URL_HASH SHA256=24b574f71c87a763f50704bbb630cbe38298d544a1f890f099a4696b1d6beba4
    DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_Declare(imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.5.tar.gz
    URL_HASH SHA256=2aa2d169c569368439e5d5667e0796d09ca5cc6432965ce082e516937d7db254
    DOWNLOAD_EXTRACT_TIMESTAMP ON)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sdl2 imgui)

# Dear ImGui has no CMake project of its own
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL2::SDL2-static)
target_compile_options(imgui PRIVATE $<IF:$<C_COMPILER_ID:MSVC>,/w,-w>) # third-party

add_executable(nowindgui WIN32 MACOSX_BUNDLE
    main.cpp
    app/Library.cpp
    app/Settings.cpp
    host/HostSession.cpp
    ui/MainWindow.cpp
    ui/Theme.cpp)
target_link_libraries(nowindgui PRIVATE nowind nwgeneral imgui SDL2::SDL2main project::warnings_options project::warning_suppressions)

install(TARGETS nowindgui
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime)
