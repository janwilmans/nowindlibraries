//! Image.h
#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <fstream>

#include "PartitionInfo.h"
#include "DiskHandler.hh"
#include "SectorMedium.hh"

class Image : public nowind::SectorMedium {

private:
    std::fstream *stream;
    unsigned int sectorSize;
    bool containsDisk;
    bool readOnly;
    bool diskChanged;
    bool containsRomdisk;
	unsigned int offset;
	unsigned int length;		// highest sector in image
	unsigned int fileSize;

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
    virtual bool reOpenDiskImage();      /* close the disk-image and reopens it to make sure it's read-only status has not changed */
    virtual unsigned int getSectorSize();

    virtual int readSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount);
    virtual int writeSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount);

	virtual SectorMedium * getSectorMedium() { return this; }

	const nw_byte * GetNewPartitionTable();
	static void GetPartitionInfo(const nw_byte * mbr, PartitionInfo *info, unsigned int partitionNumber);
};

#endif //IMAGE_H

