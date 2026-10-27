#pragma once

#include <filesystem>
#include <string>

namespace nowindgui {

struct Settings {
    std::string libraryFolder;
    int uiZoom = 2; // 1..3, the pixel font is only drawn at whole multiples
    int scanlineIntensity = 28; // 0..100 percent, 0 is off
    bool preferDos2 = false;
    bool allowOtherDiskRoms = false;
    bool phantomDrives = false;
    bool reduceEffects = false;
};

// key=value lines in the per-user preferences folder; a missing or damaged file gives the defaults
[[nodiscard]] Settings loadSettings(const std::filesystem::path& file);
void saveSettings(const std::filesystem::path& file, const Settings& settings);

} // namespace nowindgui
