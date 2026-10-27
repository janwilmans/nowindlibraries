#pragma once

#include "../host/HostConfiguration.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace nowindgui {

struct LibraryItem {
    std::string name;
    std::string path;
    std::uintmax_t sizeBytes = 0;
    DriveKind kind = DriveKind::FloppyImage;
};

// Images above the size of a 720 kB floppy are taken to be harddisk images.
[[nodiscard]] DriveKind guessDriveKind(std::uintmax_t sizeBytes);

// The disk images (*.dsk) below a folder, sorted by name
[[nodiscard]] std::vector<LibraryItem> scanLibrary(const std::filesystem::path& folder);

} // namespace nowindgui
