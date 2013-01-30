//! DiskInterface.h
#ifndef DISKINTERFACE_H
#define DISKINTERFACE_H

#include <string>
#include <fstream>
#include "msxtypes.h"
#include "DiskDrive.h"
#include "VirtualDrive.h"

class Z80;

/*!
 * The DiskInterface class emulates the disk-interface on a high level, this means
 * that bios-calls 0x4010 -> 0x401F are patched (see z80) with non-existing opcode (ED 00) 
 * and RET (C9) and the z80 calls functions in this class to replace them.
 */
class DiskInterface {

private:
		 	DiskInterface();
Z80     	*z80;

DiskDrive   *disk;
DiskDrive  	*drive[2];

public:
			~DiskInterface();
static		DiskInterface * Instance();

    /*! \brief insert a disk-image in drive a:
    *  \param filename name of the disk-image
    *
    *  If the image is read-only, it will be "write protected" in the emulation<br>
    *  Note: For testing and debugging, image.dsk is loaded when the specified image is not found
    */
    void insertDisk(unsigned int diskNumber, std::string);
    void diskIO();
    void setDrive(nw_byte register_a);

    void readSectors(unsigned int startSector, unsigned int sectorCount, nw_word destAddress);
    void writeSectors(unsigned int startSector, unsigned int sectorCount, nw_word sourceAddress);
};

#endif

