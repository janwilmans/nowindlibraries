#include "Settings.h"

#include <algorithm>
#include <exception>
#include <fstream>

namespace nowindgui {

namespace {

int parseInt(const std::string& text, int fallback, int minimum, int maximum)
{
    try
    {
        return std::clamp(std::stoi(text), minimum, maximum);
    }
    catch (const std::exception&) // not a number, a damaged file must not stop the program
    {
        return fallback;
    }
}

} // namespace

Settings loadSettings(const std::filesystem::path& file)
{
    Settings settings;
    std::ifstream stream(file);
    std::string line;
    while (std::getline(stream, line))
    {
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos)
        {
            continue;
        }
        const std::string key = line.substr(0, separator);
        const std::string value = line.substr(separator + 1);
        if (key == "libraryFolder") settings.libraryFolder = value;
        else if (key == "uiZoom") settings.uiZoom = parseInt(value, settings.uiZoom, 1, 3);
        else if (key == "scanlineIntensity") settings.scanlineIntensity = parseInt(value, settings.scanlineIntensity, 0, 100);
        else if (key == "preferDos2") settings.preferDos2 = value == "1";
        else if (key == "allowOtherDiskRoms") settings.allowOtherDiskRoms = value == "1";
        else if (key == "phantomDrives") settings.phantomDrives = value == "1";
        else if (key == "reduceEffects") settings.reduceEffects = value == "1";
    }
    return settings;
}

void saveSettings(const std::filesystem::path& file, const Settings& settings)
{
    std::ofstream stream(file, std::ios::trunc);
    stream << "libraryFolder=" << settings.libraryFolder << '\n'
           << "uiZoom=" << settings.uiZoom << '\n'
           << "scanlineIntensity=" << settings.scanlineIntensity << '\n'
           << "preferDos2=" << settings.preferDos2 << '\n'
           << "allowOtherDiskRoms=" << settings.allowOtherDiskRoms << '\n'
           << "phantomDrives=" << settings.phantomDrives << '\n'
           << "reduceEffects=" << settings.reduceEffects << '\n';
}

} // namespace nowindgui
