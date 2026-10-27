#include "Theme.h"

#include <SDL.h>
#include <imgui.h>

namespace nowindgui {

namespace {

constexpr float baseFontPixels = 13.0F; // the size the built-in font was drawn for

void setColors(ImGuiStyle& style)
{
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = toColor(role::text);
    colors[ImGuiCol_TextDisabled] = toColor(role::disabledText);
    colors[ImGuiCol_WindowBg] = toColor(role::background);
    colors[ImGuiCol_ChildBg] = toColor(role::panel);
    colors[ImGuiCol_PopupBg] = toColor(role::panel);
    colors[ImGuiCol_Border] = toColor(role::border);
    colors[ImGuiCol_BorderShadow] = toColor(role::panel, 0.0F);
    colors[ImGuiCol_FrameBg] = toColor(role::panel);
    colors[ImGuiCol_FrameBgHovered] = toColor(msx::darkGreen);
    colors[ImGuiCol_FrameBgActive] = toColor(msx::darkGreen);
    colors[ImGuiCol_TitleBg] = toColor(role::control);
    colors[ImGuiCol_TitleBgActive] = toColor(role::control);
    colors[ImGuiCol_ScrollbarBg] = toColor(role::panel);
    colors[ImGuiCol_ScrollbarGrab] = toColor(role::control);
    colors[ImGuiCol_ScrollbarGrabHovered] = toColor(role::controlHovered);
    colors[ImGuiCol_ScrollbarGrabActive] = toColor(role::controlActive);
    colors[ImGuiCol_CheckMark] = toColor(role::accent);
    colors[ImGuiCol_SliderGrab] = toColor(role::control);
    colors[ImGuiCol_SliderGrabActive] = toColor(role::controlActive);
    colors[ImGuiCol_Button] = toColor(role::control);
    colors[ImGuiCol_ButtonHovered] = toColor(role::controlHovered);
    colors[ImGuiCol_ButtonActive] = toColor(role::controlActive);
    colors[ImGuiCol_Header] = toColor(role::control);
    colors[ImGuiCol_HeaderHovered] = toColor(role::controlHovered);
    colors[ImGuiCol_HeaderActive] = toColor(role::controlActive);
    colors[ImGuiCol_Separator] = toColor(role::border);
    colors[ImGuiCol_TextSelectedBg] = toColor(msx::darkGreen);
    colors[ImGuiCol_NavHighlight] = toColor(role::accent);
}

} // namespace

ImVec4 toColor(Rgb color, float alpha)
{
    return ImVec4(color.red / 255.0F, color.green / 255.0F, color.blue / 255.0F, alpha);
}

void applyTheme(int zoom)
{
    const float scale = static_cast<float>(zoom);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFontConfig fontConfig;
    fontConfig.SizePixels = baseFontPixels * scale;
    fontConfig.OversampleH = 1;
    fontConfig.OversampleV = 1;
    fontConfig.PixelSnapH = true;
    io.Fonts->AddFontDefault(&fontConfig);

    ImGuiStyle style;
    setColors(style);
    style.WindowRounding = 0.0F;
    style.ChildRounding = 0.0F;
    style.FrameRounding = 0.0F;
    style.PopupRounding = 0.0F;
    style.ScrollbarRounding = 0.0F;
    style.GrabRounding = 0.0F;
    style.TabRounding = 0.0F;
    style.AntiAliasedLines = false;
    style.AntiAliasedFill = false;
    style.ScaleAllSizes(scale);
    style.WindowBorderSize = scale;
    style.ChildBorderSize = scale;
    style.FrameBorderSize = scale;
    style.PopupBorderSize = scale;
    style.WindowPadding = ImVec2(6.0F * scale, 6.0F * scale);
    style.FramePadding = ImVec2(4.0F * scale, 2.0F * scale);
    style.ItemSpacing = ImVec2(6.0F * scale, 4.0F * scale);
    ImGui::GetStyle() = style;
}

void drawScanlines(SDL_Renderer* renderer, int intensityPercent, int thickness)
{
    if (intensityPercent <= 0)
    {
        return;
    }
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(intensityPercent * 255 / 100));
    for (int y = thickness; y < height; y += 2 * thickness)
    {
        const SDL_Rect row{0, y, width, thickness};
        SDL_RenderFillRect(renderer, &row);
    }
}

} // namespace nowindgui
