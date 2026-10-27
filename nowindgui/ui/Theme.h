#pragma once

#include "Palette.h"

struct ImVec4;
struct SDL_Renderer;

namespace nowindgui {

[[nodiscard]] ImVec4 toColor(Rgb color, float alpha = 1.0F);

// MSX look for Dear ImGui: square chunky borders and the MSX palette. Rebuilds the font, a pixel font drawn at a whole
// multiple of its size; the caller has to recreate the renderer's font texture afterwards.
void applyTheme(int zoom);

// Darkens every other row of pixels, like the scanlines of a CRT; thickness is the height of one row in pixels
void drawScanlines(SDL_Renderer* renderer, int intensityPercent, int thickness);

} // namespace nowindgui
