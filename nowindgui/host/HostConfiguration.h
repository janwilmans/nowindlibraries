#pragma once

#include <string>
#include <vector>

namespace nowindgui {

enum class DriveKind {
    FloppyImage,   // a .dsk image, one drive
    HarddiskImage, // a harddisk image, one drive per partition found
    RomDisk        // the ROM disk of the interface, no file
};

struct DriveEntry {
    std::string path;
    DriveKind kind = DriveKind::FloppyImage;
};

// What nwhostapp takes on its command line, drives are numbered in the order they are listed
struct HostConfiguration {
    std::vector<DriveEntry> drives;
    bool preferDos2 = false;
    bool allowOtherDiskRoms = false;
    bool phantomDrives = false;
};

} // namespace nowindgui
