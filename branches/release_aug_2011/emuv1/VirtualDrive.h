//! VirtualDrive.h
#ifndef VIRTUALDRIVE_H
#define VIRTUALDRIVE_H

#include <string>
#include <map>
#include <fstream>
#include "msxtypes.h"
#include "DiskDrive.h"

class VirtualDrive : public DiskDrive {

class PhysicalLocation {
    
public:
    PhysicalLocation(std::string theFileName, unsigned int theOffset) {
        fileName = theFileName;
        offset = theOffset;
    }   
    std::string fileName;
    unsigned int offset;
};

typedef std::map<unsigned int, unsigned char *> SectorMap;
typedef std::map<unsigned int, PhysicalLocation *> ClusterMap;

private:
    std::fstream *stream;
    bool containsDisk;
    bool readOnly;
    
    SectorMap sectorMap;
    ClusterMap clusterMap;

    unsigned int currentCluster;
    unsigned int currentDirEntry;

    unsigned int directoryStartSector;
    unsigned int dataAreaStartSector;

    void writeLEinc(unsigned char ** buffer, unsigned len, unsigned long value);
    void writeLE(unsigned char * buffer, unsigned len, unsigned long value);
    void writeStr(unsigned char ** buffer, char * str);
public:
    VirtualDrive();
	virtual ~VirtualDrive();

    virtual bool openDiskImage(std::string);
    virtual bool hasDisk();
    virtual bool isWriteProtected();
    virtual bool reOpenDiskImage();      /* close the disk-image and reopens it to make sure it's read-only status has not changed */
    virtual unsigned int getSectorSize();
    virtual void readSectors(char * buffer, unsigned int startSector, unsigned int sectorCount);
    virtual void writeSectors(char * buffer, unsigned int startSector, unsigned int sectorCount);

    void readSector(char * buffer, unsigned int sector);
    void writeSector(char * buffer, unsigned int sector);
    
    void addFile(std::string);
    void addFileToFat(std::string, std::string, unsigned int size);
    
    void linkFatEntry(unsigned int entryNr, unsigned int linkToEntry);
   
};

/*

Bootsector (sector 0)

BS_FAT12			STRUC			    (offset)     Default Value (LSB, MSB)
-----               ------              ------       ------
bsJmp			    DB 3 DUP (?)		0x00          EB FE 90        // unused on MSX, E9 of EB een 8086 JUMP instructie
bsOemName			DB 8 DUP (?)		0x03          "12345678"
bsFAT12             BPB_FAT12 Structure	0x0B            
                    (see below)
FAT16?:
bsDriveNumber		DB ?				0x24
bsUnused			DB ?				0x25
bsExtBootSignature	DB ?				0x26
bsSerialNumber		DD ?				0x27
bsVolumeLabel		DB "NO NAME    "	0x2B
bsFileSystem		DB "FAT12   "		0x36
bsBootCode		    DB 450 DUP (?)		0x3E
BS_FAT12	ENDS	

    
The FAT12 Disk Parameter Block (aka Bios Parameter Block) is at offset 

Field               Offset     Length       Default Value (LSB, MSB)
-----               ------     ------       ------
Bytes Per Sector      11         2          512     00 02
Sectors Per Cluster   13         1          2       02
Reserved Sectors      14         2          1       01
FATs                  16         1          2       02
Root Entries          17         2          112     70 00 
Small Sectors         19         2          1440    A0 05
Media Descriptor      21         1                  F9
Sectors Per FAT       22         2          3       03 00
Sectors Per Track     24         2          9       09 00
Heads                 26         2          2       02 00
Hidden Sectors        28         2          ??      ?? ?? (unused)

// on MSX at the boot-routine begins at offset 30 (1e) 
// the boot-sector is loaded at $c000 and a call to $c01e is made.

FAT16?:
Hidden Sectors        28         4          ??      
Large Sectors         32         4          ??


Values for the "Media Descriptor"
Byte   Capacity   Media Size and Type
F0     2.88 MB    3.5-inch, 2-sided, 36-sector
F0     1.44 MB    3.5-inch, 2-sided, 18-sector
F9     720 KB     3.5-inch, 2-sided, 9-sector
F9     1.2 MB     5.25-inch, 2-sided, 15-sector
FD     360 KB     5.25-inch, 2-sided, 9-sector
FF     320 KB     5.25-inch, 2-sided, 8-sector
FC     180 KB     5.25-inch, 1-sided, 9-sector
FE     160 KB     5.25-inch, 1-sided, 8-sector
F8     -----      Fixed disk

*/



#endif

