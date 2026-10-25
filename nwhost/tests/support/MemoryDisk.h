//! MemoryDisk.h
#ifndef NWTEST_MEMORYDISK_H
#define NWTEST_MEMORYDISK_H

#include "DiskHandler.hh"
#include "SectorMedium.hh"

#include <algorithm>
#include <string>
#include <vector>

namespace nwtest {

// a disk drive with an in-memory disk, for testing NowindHost without image files
class MemoryDisk : public nwhost::SectorMedium, public nwhost::DiskHandler {
public:
    static const unsigned SECTOR_SIZE = 512;

    explicit MemoryDisk(unsigned sectors = 1440)
        : data(sectors * SECTOR_SIZE, 0)
    {
    }

    explicit MemoryDisk(std::vector<nw_byte> image)
        : data(std::move(image))
    {
    }

    // SectorMedium
    int readSectors(nw_byte* buffer, unsigned int startSector, unsigned int sectorCount) override
    {
        ++readCount;
        if (failReads || !inRange(startSector, sectorCount)) return -1;
        std::copy_n(&data[startSector * SECTOR_SIZE], sectorCount * SECTOR_SIZE, buffer);
        return 0;
    }

    int writeSectors(nw_byte* buffer, unsigned int startSector, unsigned int sectorCount) override
    {
        ++writeCount;
        if (failWrites || !inRange(startSector, sectorCount)) return -1;
        std::copy_n(buffer, sectorCount * SECTOR_SIZE, &data[startSector * SECTOR_SIZE]);
        return 0;
    }

    bool isWriteProtected() override { return writeProtected; }

    // DiskHandler
    nwhost::SectorMedium* getSectorMedium() override { return this; }
    int insertDisk(std::string filename) override
    {
        insertedFilenames.push_back(filename);
        return 0;
    }
    bool diskChanged() override
    {
        bool result = changed;
        changed = false;
        return result;
    }
    bool isRomdisk() override { return romdisk; }

    nw_byte* sector(unsigned number) { return &data[number * SECTOR_SIZE]; }
    unsigned sectorCount() const { return unsigned(data.size() / SECTOR_SIZE); }

    std::vector<nw_byte> data;
    bool writeProtected = false;
    bool changed = true;
    bool romdisk = false;
    bool failReads = false;
    bool failWrites = false;
    unsigned readCount = 0;
    unsigned writeCount = 0;
    std::vector<std::string> insertedFilenames;

private:
    bool inRange(unsigned startSector, unsigned count) const
    {
        return (std::size_t(startSector) + count) * SECTOR_SIZE <= data.size();
    }
};

} // namespace nwtest

#endif // NWTEST_MEMORYDISK_H
