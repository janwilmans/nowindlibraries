#include "Library.h"

#include <algorithm>
#include <cctype>

namespace nowindgui {

namespace {

constexpr std::uintmax_t largestFloppyBytes = 720 * 1024;
constexpr std::size_t maximumItems = 5000;

[[nodiscard]] bool hasDskExtension(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return extension == ".dsk";
}

} // namespace

DriveKind guessDriveKind(std::uintmax_t sizeBytes)
{
    return sizeBytes > largestFloppyBytes ? DriveKind::HarddiskImage : DriveKind::FloppyImage;
}

std::vector<LibraryItem> scanLibrary(const std::filesystem::path& folder)
{
    std::vector<LibraryItem> items;
    std::error_code error;
    if (!std::filesystem::is_directory(folder, error))
    {
        return items;
    }

    auto iterator = std::filesystem::recursive_directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied, error);
    for (; !error && iterator != std::filesystem::recursive_directory_iterator() && items.size() < maximumItems; iterator.increment(error))
    {
        if (!iterator->is_regular_file(error) || !hasDskExtension(iterator->path()))
        {
            continue;
        }
        LibraryItem item;
        item.name = iterator->path().filename().string();
        item.path = iterator->path().string();
        item.sizeBytes = iterator->file_size(error);
        item.kind = guessDriveKind(item.sizeBytes);
        items.push_back(std::move(item));
    }

    std::sort(items.begin(), items.end(), [](const LibraryItem& left, const LibraryItem& right) { return left.name < right.name; });
    return items;
}

} // namespace nowindgui
