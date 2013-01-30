// DiskDrive.cpp

#include <stdio.h>
#include <assert.h>

#include "DiskDrive.h"
#include "Debug.h"
#include "SDL.h"

using namespace std;

DiskDrive::DiskDrive() {

    //DBERR("DiskDrive constructor...\n");
    sectorSize = 512;
    containsDisk = false;
    readOnly = true;
    diskChanged = true;
    stream = 0;
    filename = "";
}

DiskDrive::~DiskDrive() {

	if (stream != 0) delete stream;
}

bool DiskDrive::openDiskImage(string theFilename) {
    
    filename = theFilename;
	if (stream != 0) {
        delete stream;
        stream = 0;
        containsDisk = false;
    }
    diskChanged = true;
    bool fileOpened = reOpenDiskImage();
    return fileOpened;
}

bool DiskDrive::reOpenDiskImage() {

    bool fileOpenError = false;
    // TODO: image.dsk is specified in maincons.cpp can be removed here ?
    if (stream != 0) delete stream;
	stream = new fstream(filename.c_str(),ios::binary | ios::in | ios::out);
	if (stream->fail()) {
            fileOpenError = true;
			DBERR("Error opening file %s for read/write, trying read-only !\n", filename.c_str());
			delete stream;
			stream = new fstream(filename.c_str(),ios::binary | ios::in);
			if (stream->fail()) {
					DBERR("Error opening file %s for read !\n", filename.c_str());
/*
    				string image = "disks/image.dsk";
    				DBERR("Trying " %s instead...\n", image);
    				stream = new fstream(image.c_str(),ios::binary | ios::in | ios::out);
    				if (stream->fail()) {
    						DBERR("Error opening file %s read/write !\n", image);
    				} else {
    				    containsDisk = true;
    					readOnly = false;    
					}    
*/					
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
	return !fileOpenError;
}


bool DiskDrive::hasDisk() {
    if (containsDisk) {
        assert(stream != 0 && (!stream->fail()));
    }
    return containsDisk;
}

bool DiskDrive::isWriteProtected() {
    return readOnly;
}

bool DiskDrive::isDiskChanged() {

    bool tmp = diskChanged;
    diskChanged = false;
    return tmp;
}

unsigned int DiskDrive::getSectorSize() {
    return sectorSize;    
}


void DiskDrive::readSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount) {

    unsigned int count = sectorCount * sectorSize;
    unsigned int source = startSector * sectorSize;

    // TODO: check of sectornumber isn't too high (has to fit in the current image!!!
	DBERR("DiskDrive::readSectors, startSector %u, amount %u\n", startSector, sectorCount);

    stream->seekg(source);
    stream->read((char *)buffer,count);

/*    
    char *buf = buffer;
    for(unsigned int sector=startSector;sector<sectorCount;sector++) {
		// write sector to disk for debugging
		char filename[250];
		sprintf(filename,"sector_%i.bin",sector);
		ofstream ofs(filename,ios::binary|ios::trunc);
		if (ofs.fail()) {
				DBERR("Error opening file " << filename << "!\n");
		}
		ofs.write((char *)buf,512);
		ofs.close();
		buf+=512;
	}
*/   
}

void DiskDrive::writeSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount) {
    
    unsigned int count = sectorCount * sectorSize;
    unsigned int dest = startSector * sectorSize;
	DBERR("DiskDrive::writeSectors, startSector %u, amount %u\n", startSector, sectorCount);

    stream->seekp(dest);
    stream->write((char *)buffer,count);
}
