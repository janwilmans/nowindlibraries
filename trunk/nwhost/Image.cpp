// Image.cpp

#include <stdio.h>
#include <assert.h>

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

	const nw_byte * mbr = GetNewPartitionTable();
	PartitionInfo p;
	GetPartitionInfo(mbr, &p, partitionNumber);
	
	if (p.startLBA == 0 || p.length == 0) return false;   // not a valid partition entry

	if (ignoreBootflag || (p.disabled == false) )
	{
		offset = sectorSize*p.startLBA;
		length = p.length;
		return true; 
	}

	// the partition is disabled and the stream deleted, this is correct 
	// since the image is not inserted
    delete stream;
    stream = 0;
    containsDisk = false;
    return false;
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

    stream->seekg(source);
    stream->read((char *)buffer,count);
	if (stream->fail()) return -3;

/*    
    char *buf = buffer;
    for(unsigned int sector=startSector;sector<sectorCount;sector++) {
		// write sector to disk for debugging
		char filename[250];
		sprintf(filename,"sector_%i.bin",sector);
		ofstream ofs(filename,ios::binary|ios::trunc);
		if (ofs.fail()) {
				DEBUGERROR("Error opening file " << filename << "!\n");
		}
		ofs.write((char *)buf,512);
		ofs.close();
		buf+=512;
	}
*/   
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
		DBERR("Warning: trying to write outside FAT defined area, startSector %u, amount %u\n", startSector, sectorCount);
		return -2;
	}

    DBERR("Image::writeSectors, startSector %u, amount %u\n", startSector, sectorCount);

    stream->seekp(dest);
    stream->write((char *)buffer,count);
	if (stream->fail()) return -3;
	return 0;
}

const nw_byte * Image::GetNewPartitionTable()
{
	//nw_byte * table = new nw_byte[sectorSize];
	nw_byte * table = new nw_byte[512];

    stream->seekg(0);
    stream->read((char *) table, sectorSize);

	// This code might be broken????

	/// Write mbr + first FAT to file for debugging
	nw_byte * buffer = new nw_byte[2*sectorSize];

	DBERR("Alloc part: 0x%08X\n", buffer);
    stream->seekg(0);
    stream->read((char *) buffer, 2*sectorSize);

	char filename[250];
    Util::snprintf(filename, sizeof(filename), "mbr.bin");
	ofstream ofs(filename,ios::binary|ios::trunc);
	if (ofs.fail()) {
		DBERR("Error opening file %s!\n", filename);
	}
	ofs.write((char *) buffer, 2*sectorSize);					// visual studio throw an exception here
	ofs.close();

	DBERR("free part: 0x%08X\n", buffer);

	delete [] buffer;											// gdb chokes here (seqfault, free'd twice??)
	/// Write mbr + first FAT to file for debugging
	return table;
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

	//todo: read partitions media-descriptor
	info->mediaDescriptor = 0xff;
}

