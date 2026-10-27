// nowindgui: a graphical host for the Nowind USB interface (see nwhostapp for the command line version)

#include "ui/MainWindow.h"
#include "ui/Theme.h"

#include <NwhostService.h>
#include <nowind.h>

#include <SDL.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <imgui.h>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>

namespace {

struct SdlDeleter {
    void operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }
    void operator()(SDL_Renderer* renderer) const { SDL_DestroyRenderer(renderer); }
    void operator()(char* text) const { SDL_free(text); }
};

constexpr int idleWaitMilliseconds = 100; // the host runs on another thread, the screen polls its state

[[nodiscard]] std::filesystem::path settingsFile()
{
    const std::unique_ptr<char, SdlDeleter> folder(SDL_GetPrefPath("nowind", "nowindgui"));
    return folder ? std::filesystem::path(folder.get()) / "settings.txt" : std::filesystem::path("nowindgui-settings.txt");
}

// How many physical pixels one point of the window is (2 on a HiDPI display)
[[nodiscard]] int pixelsPerPoint(SDL_Window* window, SDL_Renderer* renderer)
{
    int windowWidth = 0;
    int outputWidth = 0;
    int ignored = 0;
    SDL_GetWindowSize(window, &windowWidth, &ignored);
    SDL_GetRendererOutputSize(renderer, &outputWidth, &ignored);
    return windowWidth > 0 ? std::max(1, static_cast<int>(std::lround(static_cast<double>(outputWidth) / windowWidth))) : 1;
}

// Returns false when the program should end
[[nodiscard]] bool handleEvents(nowindgui::MainWindow& mainWindow)
{
    SDL_Event event;
    bool running = true;
    bool available = SDL_WaitEventTimeout(&event, idleWaitMilliseconds) != 0;
    while (available)
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
        {
            running = false;
        }
        else if (event.type == SDL_DROPFILE)
        {
            const std::unique_ptr<char, SdlDeleter> file(event.drop.file);
            mainWindow.addDroppedFile(file.get());
        }
        available = SDL_PollEvent(&event) != 0;
    }
    return running;
}

void rebuildTheme(int zoom)
{
    ImGui_ImplSDLRenderer2_DestroyDeviceObjects(); // recreated by the next NewFrame, with the new font
    nowindgui::applyTheme(zoom);
}

} // namespace

int main(int, char**)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // nearest neighbour keeps the pixel font sharp

    constexpr Uint32 windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    const std::unique_ptr<SDL_Window, SdlDeleter> window(SDL_CreateWindow("Nowind", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1100, 700, windowFlags));
    const std::unique_ptr<SDL_Renderer, SdlDeleter> renderer(window ? SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED) : nullptr);
    if (!renderer)
    {
        std::fprintf(stderr, "Cannot create the window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowMinimumSize(window.get(), 640, 400);

    nowind::initialize();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
    ImGui_ImplSDLRenderer2_Init(renderer.get());

    {
        nowindgui::MainWindow mainWindow(nowindgui::loadSettings(settingsFile()), settingsFile());
        int appliedZoom = mainWindow.settings().uiZoom;
        nowindgui::applyTheme(appliedZoom);

        while (handleEvents(mainWindow))
        {
            if (mainWindow.settings().uiZoom != appliedZoom)
            {
                appliedZoom = mainWindow.settings().uiZoom;
                rebuildTheme(appliedZoom);
            }

            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            mainWindow.draw();
            ImGui::Render();

            const ImVec4 background = nowindgui::toColor(nowindgui::role::background);
            SDL_SetRenderDrawColor(renderer.get(), static_cast<Uint8>(background.x * 255), static_cast<Uint8>(background.y * 255), static_cast<Uint8>(background.z * 255), 255);
            SDL_RenderClear(renderer.get());
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer.get());
            if (!mainWindow.settings().reduceEffects)
            {
                nowindgui::drawScanlines(renderer.get(), mainWindow.settings().scanlineIntensity, pixelsPerPoint(window.get(), renderer.get()));
            }
            SDL_RenderPresent(renderer.get());
        }
    } // the main window stops the host before the library state goes away

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_Quit();
    return 0;
}
