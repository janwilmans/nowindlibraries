//! Image.h
#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <fstream>
#include <cstdint>

#include "PartitionInfo.h"
#include "DiskHandler.hh"
#include "SectorMedium.hh"

class Image : public nwhost::SectorMedium {
private:
    std::fstream* stream;
    unsigned int sectorSize;
    bool containsDisk;
    bool readOnly;
    bool diskChanged;
    bool containsRomdisk;
    std::uint64_t offset; // in bytes; hard disk images can be larger than 4 GB
    unsigned int length;  // highest sector in image
    std::uint64_t fileSize;
    unsigned int partitionNr;
    bool harddiskPartition;
    PartitionInfo partitionInfo;

public:
    std::string filename;
    Image();
    virtual ~Image();

    virtual bool openDiskImage(std::string);
    virtual bool openPartitionImage(int, bool, std::string);
    virtual void setRomdisk();
    virtual bool hasDisk();
    virtual bool isWriteProtected();
    virtual bool isDiskChanged();
    virtual bool isRomdisk();
    virtual bool reOpenDiskImage(); /* close the disk-image and reopens it to make sure it's read-only status has not changed */
    virtual unsigned int getSectorSize();
    virtual std::string getDescription();

    virtual bool isHarddiskPartition();
    virtual unsigned int getPartitionNr();
    virtual std::uint64_t getPartitionOffset();

    // the partition table holds this many entries, read backwards from 0x1EE
    static const int MAX_PARTITIONS = 30;

    bool SetActivePartition(int partitionNumber);

    virtual int readSectors(nw_byte* buffer, unsigned int startSector, unsigned int sectorCount);
    virtual int writeSectors(nw_byte* buffer, unsigned int startSector, unsigned int sectorCount);

    virtual SectorMedium* getSectorMedium() { return this; }

    const nw_byte* GetNewPartitionTable();                                                               // obsolete, remove
    static void GetPartitionInfo(const nw_byte* mbr, PartitionInfo* info, unsigned int partitionNumber); // obsolete, remove
    bool ReadPartitionInfo(int partitionNumber);                                                         // new
};

#endif //IMAGE_H
