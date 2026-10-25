//! Fat12Image.h
#ifndef NWTEST_FAT12IMAGE_H
#define NWTEST_FAT12IMAGE_H

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace nwtest {

// the geometry of an MSX floppy format, as in the boot sector
struct FloppyFormat {
    std::uint8_t media;
    std::uint8_t sectorsPerCluster;
    std::uint16_t rootEntries;
    std::uint16_t totalSectors;
    std::uint8_t sectorsPerFat;
    std::uint8_t fats;
};

// the formats in the nowind ROM's supportedMedia table (MAKEDPB in msxsrc/nowindDriver.asm), same order
inline const std::vector<FloppyFormat>& msxFloppyFormats()
{
    static const std::vector<FloppyFormat> formats = {
        {0xf8, 2, 112, 1 * 80 * 9, 2, 2}, // 360 kB
        {0xf9, 2, 112, 2 * 80 * 9, 3, 2}, // 720 kB
        {0xfa, 2, 112, 1 * 80 * 8, 1, 2}, // 320 kB
        {0xfb, 2, 112, 2 * 80 * 8, 2, 2}, // 640 kB
        {0xfc, 1, 64, 1 * 40 * 9, 2, 2},  // 180 kB
        {0xfd, 2, 112, 2 * 40 * 9, 2, 2}, // 360 kB
        {0xfe, 1, 64, 1 * 40 * 8, 1, 2},  // 160 kB
        {0xff, 2, 112, 2 * 40 * 8, 1, 1}, // 320 kB
    };
    return formats;
}

inline const FloppyFormat& format720k()
{
    return msxFloppyFormats()[1];
}

// an empty, formatted FAT12 floppy image in memory; files can be added to and read from the root directory
class Fat12Image {
public:
    static const unsigned SECTOR_SIZE = 512;

    explicit Fat12Image(const FloppyFormat& aFormat = format720k())
        : format(aFormat)
        , data(std::size_t(aFormat.totalSectors) * SECTOR_SIZE, 0)
    {
        writeBootSector(data.data(), format);
        for (unsigned f = 0; f < format.fats; ++f)
        {
            std::uint8_t* fat = &data[(1 + f * format.sectorsPerFat) * SECTOR_SIZE];
            fat[0] = format.media;
            fat[1] = 0xff;
            fat[2] = 0xff;
        }
    }

    // an existing image, the geometry comes from its boot sector
    explicit Fat12Image(std::vector<std::uint8_t> image)
        : format{image.at(21), image.at(13), std::uint16_t(get16(&image[17])), std::uint16_t(get16(&image[19])),
                 std::uint8_t(get16(&image[22])), image.at(16)}
        , data(std::move(image))
    {
    }

    // a boot sector with this geometry (only the BPB fields, the jump and the signature)
    static void writeBootSector(std::uint8_t* sector, const FloppyFormat& format)
    {
        std::fill(sector, sector + SECTOR_SIZE, 0);
        sector[0] = 0xeb;
        sector[1] = 0xfe;
        sector[2] = 0x90;
        std::copy_n("NOWINDTS", 8, &sector[3]);
        put16(&sector[11], SECTOR_SIZE);
        sector[13] = format.sectorsPerCluster;
        put16(&sector[14], 1); // reserved sectors
        sector[16] = format.fats;
        put16(&sector[17], format.rootEntries);
        put16(&sector[19], format.totalSectors);
        sector[21] = format.media;
        put16(&sector[22], format.sectorsPerFat);
        sector[510] = 0x55;
        sector[511] = 0xaa;
    }

    static std::vector<std::uint8_t> bootSector(const FloppyFormat& format)
    {
        std::vector<std::uint8_t> sector(SECTOR_SIZE);
        writeBootSector(sector.data(), format);
        return sector;
    }

    unsigned firstFatSector() const { return 1; }
    unsigned firstDirSector() const { return 1u + unsigned(format.fats) * format.sectorsPerFat; }
    unsigned dirSectors() const { return (format.rootEntries * 32 + SECTOR_SIZE - 1) / SECTOR_SIZE; }
    unsigned firstDataSector() const { return firstDirSector() + dirSectors(); }
    unsigned clusterCount() const { return (format.totalSectors - firstDataSector()) / format.sectorsPerCluster; }
    unsigned clusterSize() const { return format.sectorsPerCluster * SECTOR_SIZE; }

    // adds a file to the root directory, name in 8.3 form, like "AUTOEXEC.BAT"
    void addFile(const std::string& name, const std::vector<std::uint8_t>& content)
    {
        std::uint8_t* entry = freeDirEntry();
        const unsigned clusters = unsigned((content.size() + clusterSize() - 1) / clusterSize());
        unsigned first = 0;
        unsigned previous = 0;
        for (unsigned i = 0; i < clusters; ++i)
        {
            unsigned cluster = freeCluster();
            setFat(cluster, 0xfff); // reserve it now, so freeCluster() does not return it again
            if (previous == 0)
                first = cluster;
            else
                setFat(previous, cluster);
            previous = cluster;
            std::size_t offset = std::size_t(i) * clusterSize();
            std::size_t size = std::min<std::size_t>(clusterSize(), content.size() - offset);
            std::copy_n(content.begin() + std::ptrdiff_t(offset), size, &data[clusterSector(cluster) * SECTOR_SIZE]);
        }
        std::fill(entry, entry + 32, 0);
        std::string dirName = toDirName(name);
        std::copy(dirName.begin(), dirName.end(), entry);
        entry[11] = 0x20; // archive
        put16(&entry[26], first);
        put32(&entry[28], std::uint32_t(content.size()));
    }

    void addFile(const std::string& name, const std::string& content)
    {
        addFile(name, std::vector<std::uint8_t>(content.begin(), content.end()));
    }

    // the content of a file in the root directory, or nothing when it does not exist
    std::optional<std::vector<std::uint8_t>> readFile(const std::string& name) const
    {
        const std::uint8_t* entry = findDirEntry(toDirName(name));
        if (!entry) return std::nullopt;
        std::uint32_t size = get32(&entry[28]);
        std::vector<std::uint8_t> content;
        unsigned cluster = get16(&entry[26]);
        while (content.size() < size && cluster >= 2 && cluster < 0xff8)
        {
            const std::uint8_t* source = &data[clusterSector(cluster) * SECTOR_SIZE];
            std::size_t count = std::min<std::size_t>(clusterSize(), size - content.size());
            content.insert(content.end(), source, source + count);
            cluster = getFat(cluster);
        }
        if (content.size() != size) throw std::runtime_error("broken FAT chain for " + name);
        return content;
    }

    // the 8.3 names in the root directory, like "COMMAND.COM"
    std::vector<std::string> fileNames() const
    {
        std::vector<std::string> names;
        for (unsigned i = 0; i < format.rootEntries; ++i)
        {
            const std::uint8_t* entry = dirEntry(i);
            if (entry[0] == 0) break;
            if (entry[0] == 0xe5 || (entry[11] & 0x08)) continue; // deleted or volume label
            std::string base(reinterpret_cast<const char*>(entry), 8);
            std::string ext(reinterpret_cast<const char*>(entry) + 8, 3);
            base.erase(base.find_last_not_of(' ') + 1);
            ext.erase(ext.find_last_not_of(' ') + 1);
            names.push_back(ext.empty() ? base : base + "." + ext);
        }
        return names;
    }

    unsigned getFat(unsigned cluster) const
    {
        const std::uint8_t* fat = &data[firstFatSector() * SECTOR_SIZE];
        unsigned offset = cluster + cluster / 2;
        unsigned value = fat[offset] | (fat[offset + 1] << 8);
        return (cluster & 1) ? (value >> 4) : (value & 0xfff);
    }

    void setFat(unsigned cluster, unsigned value)
    {
        for (unsigned f = 0; f < format.fats; ++f)
        {
            std::uint8_t* fat = &data[(firstFatSector() + f * format.sectorsPerFat) * SECTOR_SIZE];
            unsigned offset = cluster + cluster / 2;
            if (cluster & 1)
            {
                fat[offset] = std::uint8_t((fat[offset] & 0x0f) | ((value << 4) & 0xf0));
                fat[offset + 1] = std::uint8_t(value >> 4);
            }
            else
            {
                fat[offset] = std::uint8_t(value);
                fat[offset + 1] = std::uint8_t((fat[offset + 1] & 0xf0) | ((value >> 8) & 0x0f));
            }
        }
    }

    FloppyFormat format;
    std::vector<std::uint8_t> data;

private:
    static void put16(std::uint8_t* p, unsigned value)
    {
        p[0] = std::uint8_t(value);
        p[1] = std::uint8_t(value >> 8);
    }
    static void put32(std::uint8_t* p, std::uint32_t value)
    {
        put16(p, value & 0xffff);
        put16(p + 2, value >> 16);
    }
    static unsigned get16(const std::uint8_t* p) { return p[0] | (p[1] << 8); }
    static std::uint32_t get32(const std::uint8_t* p) { return get16(p) | (std::uint32_t(get16(p + 2)) << 16); }

    static std::string toDirName(const std::string& name)
    {
        std::string upper = name;
        std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return char(std::toupper(c)); });
        std::string::size_type dot = upper.find('.');
        std::string base = upper.substr(0, dot);
        std::string ext = dot == std::string::npos ? "" : upper.substr(dot + 1);
        if (base.empty() || base.size() > 8 || ext.size() > 3) throw std::invalid_argument("not an 8.3 name: " + name);
        base.resize(8, ' ');
        ext.resize(3, ' ');
        return base + ext;
    }

    unsigned clusterSector(unsigned cluster) const { return firstDataSector() + (cluster - 2) * format.sectorsPerCluster; }

    const std::uint8_t* dirEntry(unsigned index) const { return &data[firstDirSector() * SECTOR_SIZE + index * 32]; }

    const std::uint8_t* findDirEntry(const std::string& dirName) const
    {
        for (unsigned i = 0; i < format.rootEntries; ++i)
        {
            const std::uint8_t* entry = dirEntry(i);
            if (entry[0] == 0) break;
            if (std::equal(dirName.begin(), dirName.end(), entry)) return entry;
        }
        return nullptr;
    }

    std::uint8_t* freeDirEntry()
    {
        for (unsigned i = 0; i < format.rootEntries; ++i)
        {
            std::uint8_t* entry = &data[firstDirSector() * SECTOR_SIZE + i * 32];
            if (entry[0] == 0 || entry[0] == 0xe5) return entry;
        }
        throw std::runtime_error("root directory full");
    }

    unsigned freeCluster() const
    {
        for (unsigned cluster = 2; cluster < clusterCount() + 2; ++cluster)
        {
            if (getFat(cluster) == 0) return cluster;
        }
        throw std::runtime_error("disk full");
    }
};

} // namespace nwtest

#endif // NWTEST_FAT12IMAGE_H
