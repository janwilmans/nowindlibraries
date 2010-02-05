//! DiskDrive.h
#ifndef DISKDRIVE_H
#define DISKDRIVE_H

#include <string>
#include <fstream>

#include "msxtypes.h"

class DiskDrive {

private:
    std::fstream *stream;
    std::string filename;
    unsigned int sectorSize;
    bool containsDisk;
    bool readOnly;
    bool diskChanged;

public:
    DiskDrive();
	virtual ~DiskDrive();

    virtual bool openDiskImage(std::string);
    virtual bool hasDisk();
    virtual bool isWriteProtected();
    virtual bool isDiskChanged();
    virtual bool reOpenDiskImage();      /* close the disk-image and reopens it to make sure it's read-only status has not changed */
    virtual unsigned int getSectorSize();
    virtual void readSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount);
    virtual void writeSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount);

};

#endif

