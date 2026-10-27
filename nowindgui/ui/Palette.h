#pragma once

#include <array>
#include <cstdint>

namespace nowindgui {

struct Rgb {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
};

// The MSX2 (V9938) palette holds 9-bit colours, 3 bits per channel
[[nodiscard]] constexpr Rgb fromVdp(int red3, int green3, int blue3)
{
    return {static_cast<std::uint8_t>(red3 * 255 / 7), static_cast<std::uint8_t>(green3 * 255 / 7), static_cast<std::uint8_t>(blue3 * 255 / 7)};
}

// The default palette entries 1..15 of an MSX2 (entry 0 is transparent and not used here)
namespace msx {
inline constexpr Rgb black = fromVdp(0, 0, 0);
inline constexpr Rgb mediumGreen = fromVdp(1, 6, 1);
inline constexpr Rgb lightGreen = fromVdp(3, 7, 3);
inline constexpr Rgb darkBlue = fromVdp(1, 1, 7);
inline constexpr Rgb lightBlue = fromVdp(2, 3, 7);
inline constexpr Rgb darkRed = fromVdp(5, 1, 1);
inline constexpr Rgb cyan = fromVdp(2, 6, 7);
inline constexpr Rgb mediumRed = fromVdp(7, 1, 1);
inline constexpr Rgb lightRed = fromVdp(7, 3, 3);
inline constexpr Rgb darkYellow = fromVdp(6, 6, 1);
inline constexpr Rgb lightYellow = fromVdp(6, 6, 4);
inline constexpr Rgb darkGreen = fromVdp(1, 4, 1);
inline constexpr Rgb magenta = fromVdp(6, 2, 5);
inline constexpr Rgb grey = fromVdp(5, 5, 5);
inline constexpr Rgb white = fromVdp(7, 7, 7);
} // namespace msx

// What the screens use, so they contain no colours of their own
namespace role {
inline constexpr Rgb background = msx::darkBlue;
inline constexpr Rgb text = msx::white;
inline constexpr Rgb disabledText = msx::grey;
inline constexpr Rgb accent = msx::lightYellow;
inline constexpr Rgb panel = msx::black;
inline constexpr Rgb border = msx::white;
inline constexpr Rgb control = msx::lightBlue;
inline constexpr Rgb controlHovered = msx::cyan;
inline constexpr Rgb controlActive = msx::lightYellow;
inline constexpr Rgb controlText = msx::black;
inline constexpr Rgb good = msx::lightGreen;
inline constexpr Rgb warning = msx::lightYellow;
inline constexpr Rgb error = msx::lightRed;
} // namespace role

} // namespace nowindgui
