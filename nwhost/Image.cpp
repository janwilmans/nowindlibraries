// Image.cpp

#include <stdio.h>
#include <assert.h>
#include <vector>

#include "Image.h"
#include "NwhostInternal.h"

#include <libgeneral.h>

#define DBERR nowindusb_debug_wrap_sprintf

using namespace general;
using namespace std;

Image::Image() {

    sectorSize = 512;
    containsDisk = false;
    readOnly = true;
    diskChanged = true;
    containsRomdisk = false;
    stream = 0;
    filename = "";
    offset = 0;
    length = 0;
    fileSize = 0;
    partitionNr = 0;
    harddiskPartition = false;
}

Image::~Image() {

	if (stream != 0) delete stream;
}


bool Image::openDiskImage(string aFilename) {
    
    filename = aFilename;
	if (stream != 0) {
        delete stream;
        stream = 0;
        containsDisk = false;
    }
    diskChanged = true;
    bool fileOpened = reOpenDiskImage();
	// for disk images, the offset is always zero.
	offset = 0;
	length = fileSize/sectorSize;
    harddiskPartition = false;      // the previous image may have been a harddisk partition
    return fileOpened;
}

bool Image::openPartitionImage(int partitionNumber, bool ignoreBootflag, string aFilename) {
    
	DBERR("Trying partition: %s:%u\n", aFilename.c_str(), partitionNumber);
    filename = aFilename;
	if (stream != 0) {
        delete stream;
        stream = 0;
        containsDisk = false;
    }
    diskChanged = true;
    if (!reOpenDiskImage()) return false;		// could not open image
	
	offset = 0;
    harddiskPartition = true;
    this->partitionNr = partitionNumber;

	if (!ReadPartitionInfo(partitionNumber)) return false;   // not a valid partition entry

	if (ignoreBootflag || (partitionInfo.disabled == false) )
	{
		offset = sectorSize*partitionInfo.startLBA;
		length = partitionInfo.length;
		return true; 
	}

	// the partition is disabled and the stream deleted, this is correct 
	// since the image is not inserted
    delete stream;
    stream = 0;
    containsDisk = false;
    return false;
}

// false (and nothing changed) when the partition does not exist
bool Image::SetActivePartition(int partitionNumber)
{
    if (!ReadPartitionInfo(partitionNumber)) return false;
    partitionNr = partitionNumber;
    diskChanged = true;     // the MSX must not keep using the FAT and DPB of the previous partition
    offset = sectorSize*partitionInfo.startLBA;
	length = partitionInfo.length;
    return true;
}

void Image::setRomdisk() {
    containsRomdisk = true;
}

bool Image::reOpenDiskImage() {

    bool fileOpenError = false;
    if (stream != 0) delete stream;
	stream = new fstream(filename.c_str(),ios::binary | ios::in | ios::out);
	if (stream->fail()) {
            fileOpenError = true;
			DBERR("Error opening file %s for read/write, trying read-only !\n", filename.c_str());
			delete stream;
			stream = new fstream(filename.c_str(),ios::binary | ios::in);
			if (stream->fail()) {
			    DBERR("Error opening file %s for read !\n", filename.c_str());
			} else { 
                fileOpenError = false;
                containsDisk = true;
			    readOnly = true;
          		DBERR("Opened file %s read only.\n", filename.c_str());
			} 			
	} else { 
	    containsDisk = true;
	    readOnly = false;
  		DBERR("Opened file %s read/write.\n", filename.c_str());
	}
	if (!fileOpenError)
	{
		diskChanged = true;
		stream->seekg(0, ios::end);
		fileSize = stream->tellg();
	}
	return !fileOpenError;
}

bool Image::hasDisk() {
    return containsDisk;
}

bool Image::isWriteProtected() {
    return readOnly;
}

bool Image::isDiskChanged() {

    bool tmp = diskChanged;
    diskChanged = false;
    return tmp;
}

bool Image::isRomdisk() {

    return containsRomdisk;
}

unsigned int Image::getSectorSize() {
    return sectorSize;    
}


int Image::readSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount) {

    unsigned int count = sectorCount * sectorSize;
    unsigned int source = offset + (startSector * sectorSize);

	if (stream == 0 || this->hasDisk() == false) {
        DBERR("Image::readSectors, no image loaded\n");
        return -1;
    }

    // TODO: check of sectornumber isn't too high (has to fit in the current image!!!
	//DBERR("[%s] Offset: 0x%08X Image::readSectors, startSector %u, amount %u\n", filename.c_str(), offset/512, startSector, sectorCount);

    // a failed read (past the end of a truncated image) leaves failbit set, then seekg and every next read fail too
    stream->clear();
    stream->seekg(source);
    stream->read((char *)buffer,count);
	if (stream->fail()) return -3;
	return 0;
}

int Image::writeSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount) {
    
    unsigned int count = sectorCount * sectorSize;
    unsigned int dest = offset + (startSector * sectorSize);

	if (stream == 0 || this->hasDisk() == false) {
       DBERR("Image::readSectors, no image loaded\n");
	   return -1;
    }

	if ((startSector+sectorCount) > length)
	{
		DBERR("Warning: writing outside FAT defined area, startSector %u, amount %u\n", startSector, sectorCount);
		//return -2;
	}

    DBERR("Image::writeSectors, startSector %u, amount %u\n", startSector, sectorCount);

    stream->clear();
    stream->seekp(dest);
    stream->write((char *)buffer,count);
	if (stream->fail()) return -3;
	return 0;
}

// false when the partition number is out of range, the table can not be read or the entry is empty
bool Image::ReadPartitionInfo(int partitionNumber)
{
    partitionInfo.startLBA = 0;
    partitionInfo.length = 0;
    // the entries are read backwards from 0x1EE, a higher number would be outside the sector
    if (partitionNumber < 0 || partitionNumber >= MAX_PARTITIONS || stream == 0) return false;

	std::vector<nw_byte> mbr(sectorSize);
    stream->clear();
    stream->seekg(0);
    stream->read((char *) &mbr[0], sectorSize);
    if (stream->fail()) return false;

	unsigned int tableEntry = 0x1EE - (partitionNumber*16);		// support more then 4 partitions

	// check for disabled partition
	unsigned int flags = mbr[tableEntry+0x00];

	partitionInfo.bootable = (flags & 0x80) == 0x80;
	partitionInfo.disabled = (flags & 0x02) == 0x02;
	partitionInfo.readonly = (flags & 0x01) == 0x01;

	// the offset for partitionNumber is read from the partitiontable
	unsigned int startLBA = mbr[tableEntry+0x08];
	startLBA += mbr[tableEntry+0x09]*256;
	startLBA += mbr[tableEntry+0x0A]*256*256;
	startLBA += mbr[tableEntry+0x0B]*256*256*256;
	partitionInfo.startLBA = startLBA;

	unsigned int length = mbr[tableEntry+0x0C];
	length += mbr[tableEntry+0x0D]*256;
	length += mbr[tableEntry+0x0E]*256*256;
	length += mbr[tableEntry+0x0F]*256*256*256;	
	partitionInfo.length = length;
	return partitionInfo.startLBA != 0 && partitionInfo.length != 0;
}

const nw_byte * Image::GetNewPartitionTable()
{
	nw_byte * mbr = new nw_byte[sectorSize];
    stream->seekg(0);
    stream->read((char *) mbr, sectorSize);
    return mbr;
}

/* Hardddisk image, first 512 bytes contains partition table:
 * we use an modified FAT16 partition table: http://en.wikipedia.org/wiki/Master_boot_record
 *
 * 000: EB FE 90 nn nn nn nn nn nn nn 20 
 *  |
 * code area
 *  | 
 * 1BE: 4x16 bytes (Table of primary partitions, Four 16-byte entries, IBM Partition Table scheme)
 *
 *
 * nn = disk label
 *
 * 1FE: 0x55  // MBR signature (bytenr: 510)
 * 1FF: 0xAA  // MBR signature (bytenr: 511)
 **/

/* partition entry:
 *
 * 00: 0x80 = bootable (bit 7)
 *     0x00 = non-bootable
 *     0x01 = read-only (bit 1)
 *     0x02 = disabled (bit 2)
 *     otherwise = invalid
 * 
 * 08: LBA (logical block address) of first sector in the partition (4 bytes)
 * 0C: amount of sectors in partition (4 bytes)
 */
void Image::GetPartitionInfo(const nw_byte * mbr, PartitionInfo *info, unsigned int partitionNumber)
{
	unsigned int tableEntry = 0x1EE - (partitionNumber*16);		// support more then 4 partitions

	// check for disabled partition
	unsigned int flags = mbr[tableEntry+0x00];

	info->bootable = (flags & 0x80) == 0x80;
	info->disabled = (flags & 0x02) == 0x02;
	info->readonly = (flags & 0x01) == 0x01;

	// the offset for partitionNumber is read from the partitiontable
	unsigned int startLBA = mbr[tableEntry+0x08];
	startLBA += mbr[tableEntry+0x09]*256;
	startLBA += mbr[tableEntry+0x0A]*256*256;
	startLBA += mbr[tableEntry+0x0B]*256*256*256;
	info->startLBA = startLBA;

	unsigned int length = mbr[tableEntry+0x0C];
	length += mbr[tableEntry+0x0D]*256;
	length += mbr[tableEntry+0x0E]*256*256;
	length += mbr[tableEntry+0x0F]*256*256*256;	
	info->length = length;
}

std::string Image::getDescription()
{
    return filename;
}

bool Image::isHarddiskPartition()
{
    return harddiskPartition;
}

unsigned int Image::getPartitionNr()
{
    return partitionNr;
}

unsigned int Image::getPartitionOffset()
{
    return offset;
}

