// VirtualDrive.cpp

#include <string>
#include <algorithm>
#include "stdio.h"
#include "VirtualDrive.h"
#include "Debug.h"
#include "osd/OnScreenDisplay.h"

#ifdef WIN32
#include <io.h>         // not portable to linux nor MacOS, TODO: fix!
#endif

#define ToUpper(s) std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) toupper)

using namespace std;

static const unsigned int bytesPerSector = 512;
static const unsigned int numberOfFats = 2;
static const unsigned int sectorsPerCluster = 2;
static const unsigned int reservedSectors = 1;
static const unsigned int sectorsPerFat = 3;
static const unsigned int hiddenSectors = 1;    // bootsector
static const unsigned int directoryEntries = 112;
static const unsigned int dirEntrySize = 32;

VirtualDrive::VirtualDrive() {

    DBERR("DiskDrive constructor...\n");
    containsDisk = false;
    readOnly = false;
    stream = 0;

    directoryStartSector = hiddenSectors+(numberOfFats*sectorsPerFat);      // normally: 1+(2*3) = 7
    dataAreaStartSector = directoryStartSector + ((directoryEntries*dirEntrySize)/bytesPerSector); // 7+((112*32)/512) = 7
    
    // create 14 empty sectors
    unsigned char *newMem = new unsigned char[14*bytesPerSector];
    memset(newMem,0x0,14*bytesPerSector);        // empty sector
    for(unsigned int s=0;s<14;s++) {
        sectorMap[s] = newMem+(s*bytesPerSector);
    }
    
    unsigned char *boot = newMem;
    
    DBERR("boot: 0x%04X\n", boot);
    writeLEinc(&boot,1,0xEB); // initial JMP ?! (not an opcode?)  [EX DE,HL] [CP $90]
    writeLEinc(&boot,1,0xFE);
    writeLEinc(&boot,1,0x90);  
    
    writeStr(&boot,"SOMENAME");
    DBERR("boot: 0x%04X\n", boot);
    writeLEinc(&boot,2,bytesPerSector);          // bytes/sector
    writeLEinc(&boot,1,sectorsPerCluster);       // sectors/cluster
    writeLEinc(&boot,2,reservedSectors);         // reserved sectors
    writeLEinc(&boot,1,numberOfFats);      // number of FATs
    writeLEinc(&boot,2,112);               // number of root entries
    writeLEinc(&boot,2,1440);              // total number of sectors (for small disks, otherwise 0)
    writeLEinc(&boot,1,0xf9);              // media descriptor (720 KB floppy)
    writeLEinc(&boot,2,sectorsPerFat);     // sectors/FAT
    writeLEinc(&boot,2,9);                 // sectors/TRACK
    writeLEinc(&boot,2,2);                 // heads
    writeLEinc(&boot,2,0);                 // hidden sectors ? unused ?
    DBERR("boot-routine at: 0x%04X\n", boot-newMem);
//    writeLE(&boot,1,0xd0);              // RET NC
    writeLEinc(&boot,1,0xed);              // boot-routine... (ED 04 RET)
    writeLEinc(&boot,1,0x04);              
    writeLEinc(&boot,1,0xc9);              

    containsDisk = true;
    currentCluster = 2;
    currentDirEntry = 0;
    
    unsigned char *fat = newMem+512;
    writeLEinc(&fat,1,0xf9);              
    writeLEinc(&fat,1,0xff);              
    writeLEinc(&fat,1,0xff);              

    DBERR("DIRECTORY: \n\n");
    
#ifdef WIN32
    struct _finddata_t data;
    
    string pattern = Debug::Instance()->getPath() + "\\virtualdisk\\*.*";
    long sHandle = _findfirst(pattern.c_str(), &data);
    do {
        DBERR("file: %s size: %u\n", data.name, data.size);
        string filename(data.name);
        ToUpper(filename);
        addFile(Debug::Instance()->getPath()+ "virtualdisk" + DIR_SEPARATOR + filename);
    } while( _findnext(sHandle, &data) == 0 );
    _findclose( sHandle );
#endif    
        
//    addFile(Debug::Instance()->getPath()+ "virtualdisk" + DIR_SEPARATOR + "AUTOEXEC.BAS");
//    addFile(Debug::Instance()->getPath()+ "virtualdisk" + DIR_SEPARATOR + "TEST.BAS");
    
    DBERR("DiskDrive constructor...finished\n");
}

VirtualDrive::~VirtualDrive() {

	if (stream != 0) delete stream;
    // destructor
    DBERR("DiskDrive destroyed.\n");
}


void VirtualDrive::addFile(std::string fileName) {

    unsigned int fileSize = 0;
	ifstream ifs(fileName.c_str(),ios::in | ios::binary);
	if (ifs.fail()) {
		DBERR("Error opening file %s\n", fileName.c_str());
	} else {
		DBERR("SUCCES: opening file %s\n", fileName.c_str());
   		ifs.seekg(0, ios::end);
   		fileSize = ifs.tellg();    
   		ifs.seekg(0);
   		ifs.close();
   		
   		int last = fileName.find_last_of(DIR_SEPARATOR);
   		int len = fileName.length();
   		string name = fileName.substr(last+1,len);
   		
        addFileToFat(name, fileName, fileSize);
    }
}

void VirtualDrive::addFileToFat(std::string fileName, std::string fullFileName, unsigned int size) {

    unsigned int offset = 0;
    string::size_type first = fileName.find_first_of(".");
    string::size_type len = fileName.length();
    string name = fileName.substr(0,first);
    string ext = "";
    if (first != string::npos) {
        ext = fileName.substr(first+1,len);
    }
	DBERR("name: %s\n", name.c_str());
	DBERR(", ext: %s\n", ext.c_str());
    DBERR(", size: %u\n", size);

    DBERR("directoryStartSector: %u\n", directoryStartSector);
    
    unsigned char * sector = sectorMap[directoryStartSector];
    memset(sector+(currentDirEntry*32), 0x0, 32);            // fill with 32x 0x00
    memset(sector+(currentDirEntry*32), 0x20, 13);            // fill with 13x 0x20
    memcpy(sector+(currentDirEntry*32), name.c_str(),name.length());
    memcpy(sector+(currentDirEntry*32)+8, ext.c_str(),ext.length());
    writeLE(sector+(currentDirEntry*32)+26, 2, currentCluster);      // start cluster
    writeLE(sector+(currentDirEntry*32)+28, 4, size);      // filesize
    
    currentDirEntry++;
    
    unsigned int bytesToWrite = size;
    unsigned int clusterSize = bytesPerSector*sectorsPerCluster;
    while (bytesToWrite > clusterSize) {
        linkFatEntry(currentCluster, currentCluster+1);
        clusterMap[currentCluster] = new PhysicalLocation(fullFileName, offset);
		DBERR("clusterMap: %u = %s, %u\n", currentCluster, fileName.c_str(), offset);

        offset+=clusterSize;           
        currentCluster+=1;   
        bytesToWrite-=clusterSize;     
    }
    linkFatEntry(currentCluster, 0x0fff);           // terminate file
    clusterMap[currentCluster] = new PhysicalLocation(fullFileName, offset);
	DBERR("clusterMap: %u = %s, %u\n", currentCluster, fileName.c_str(), offset);
    currentCluster+=1;   
}

void VirtualDrive::linkFatEntry(unsigned int entryNr, unsigned int linkToEntry) {
    
    unsigned int fatSector = hiddenSectors;     // first FAT begins right after the bootsector 
    for (unsigned int f=0;f<numberOfFats;f++) {

            DBERR("linkFatEntry: %u to %u on sector: %u\n", entryNr, linkToEntry, fatSector);
            unsigned int offset = entryNr+(entryNr/2);
            // todo: if offset> 512 andere sector lezen
            
            unsigned char * sector = sectorMap[fatSector];

            DBERR("offset: %u\n", offset);
            DBERR("entryNr: %u\n", entryNr);
            DBERR("entryNr: %u\n", entryNr&1);
            
            if ((entryNr&1) == 0) {
                unsigned int value = linkToEntry&0xff;
                *(sector+offset) = value;
                DBERR("  offset_1_1: %u = %u\n", offset, value);

                unsigned char save = *(sector+offset+1);
                value  = (save & 0xf0) | (linkToEntry >> 8);
                *(sector+offset+1) = value;
                DBERR("  offset_1_2: %u = %u\n", offset+1, value);

            } else {
                unsigned char save = *(sector+offset);
                unsigned int value = ((linkToEntry << 4) & 0xf0) | (save & 0x0f);
                *(sector+offset) = value;
                DBERR("  offset_2_1: %u = %u\n", offset, value);
                value = linkToEntry >> 4;
                *(sector+offset+1) = value;
                DBERR("  offset_2_2: %u = %u\n", offset+1, value);
            }                
            
            fatSector += sectorsPerFat;
    }   
}

bool VirtualDrive::openDiskImage(string theFilename) {
    
    return true;
}

bool VirtualDrive::reOpenDiskImage() {

    return true;
}

bool VirtualDrive::hasDisk() {
//    if (containsDisk) {
//        assert(stream != 0 && (!stream->fail()));
//    }
    return containsDisk;
}

bool VirtualDrive::isWriteProtected() {
    return readOnly;
}

unsigned int VirtualDrive::getSectorSize() {
    return bytesPerSector;    
}


void VirtualDrive::readSectors(char * buffer, unsigned int startSector, unsigned int sectorCount) {

    DBERR("VirtualDrive::readSectors: \n");

    for (unsigned int i=startSector;i<startSector+sectorCount;i++) {
        readSector(buffer,i);
        buffer+=bytesPerSector;
    }
//    DBERR("VirtualDrive::readSectors: ends\n");
}

void VirtualDrive::writeSectors(char * buffer, unsigned int startSector, unsigned int sectorCount) {

    DBERR("VirtualDrive::writeSectors: \n");
    
    for (unsigned int i=startSector;i<startSector+sectorCount;i++) {
        writeSector(buffer,i);
        buffer+=bytesPerSector;
    }
//    DBERR("VirtualDrive::writeSectors: ends\n");
}

void VirtualDrive::readSector(char * buffer, unsigned int sector) {
    
    DBERR("VirtualDrive::readSector: %u\n", sector);

    if (sector > 1440) {
        DBERR("  Sector out of range!\n");
    }
    
    SectorMap::iterator i = sectorMap.find(sector);
    if (i != sectorMap.end()) {
        // sector is in the sectorMap
        memcpy(buffer, i->second, 512);
            
    } else {
        // previously an empty sector was returned here.
        unsigned int relSector = sector-dataAreaStartSector;
        unsigned int offset = (relSector%sectorsPerCluster)*bytesPerSector;
        unsigned int cluster = 2+(relSector/sectorsPerCluster);     //todo: werkt dit ook voor bv sectorsPerCluster == 3 ?

        DBERR("cluster: %u\n", cluster);

        ClusterMap::iterator i = clusterMap.find(cluster);
        if (i == clusterMap.end()) {
            DBERR("Warning, reading unallocated cluster: %u\n", cluster);
            memset(buffer, 0x0, bytesPerSector);
            return;    
        }
        PhysicalLocation *loc = i->second;
		DBERR("    file: %s offset: %u\n", loc->fileName.c_str(), loc->offset+offset);

    	fstream ifs(loc->fileName.c_str(),ios::in | ios::binary);
    	if (ifs.fail()) {
    		DBERR("Error opening file %s for reading!\n", loc->fileName.c_str());
    	} else {
       		ifs.seekg(loc->offset+offset);
   	    	ifs.read(buffer,bytesPerSector);
       		ifs.close();
        }
    }   

	// write sector to disk for debugging
	char filename[250];
	sprintf(filename,"sector_%i.bin",sector);
	ofstream ofs(filename,ios::binary|ios::trunc);
	if (ofs.fail()) {
			DBERR("Error opening file %s!\n", filename);
	}
	ofs.write((char *)buffer,512);
	ofs.close();

    DBERR("VirtualDrive::readSector: %u ends\n", sector);
}

 /*   
char * VirtualDrive::getSector(unsigned int sector) {
    
    DBERR("VirtualDrive::getSector: " << sector << endl);

    if (sector > 1440) {
        DBERR("  Sector out of range!\n");
        return 0;
    }
    SectorMap::iterator i = sectorMap.find(sector);
    if (i != sectorMap.end()) {
        // sector is in the sectorMap
        memcpy(buffer, i->second, 512);
            
    } else {
        // sector is in not the sectorMap
        memset(buffer, 0xc9,512);       // return empty sector
    }
    

		// write sector to disk for debugging
		char filename[250];
		sprintf(filename,"sector_%i.bin",sector);
		ofstream ofs(filename,ios::binary|ios::trunc);
		if (ofs.fail()) {
				DBERR("Error opening file " << filename << "!\n");
		}
		ofs.write((char *)buffer,512);
		ofs.close();

		delete [] buffer;
    
    DBERR("VirtualDrive::readSector: " << sector << " ends \n");
}
    */

void VirtualDrive::writeSector(char * buffer, unsigned int sector) {

    DBERR("VirtualDrive::writeSector\n");
    
    SectorMap::iterator i = sectorMap.find(sector);
    if (i != sectorMap.end()) {
        // sector is in the sectorMap, just write it in memory
        memcpy(buffer, i->second, 512);
    } else {
        // sector is in not the sectorMap
        // resolve the sector to it's cluster and write to the associated file.

        unsigned int relSector = sector-dataAreaStartSector;
        unsigned int offset = (relSector%sectorsPerCluster)*bytesPerSector;
        unsigned int cluster = 2+(relSector/sectorsPerCluster);             //todo: werkt dit ook voor bv sectorsPerCluster == 3 ?

        ClusterMap::iterator i = clusterMap.find(cluster);
        assert(i != clusterMap.end());
        PhysicalLocation *loc = i->second;
		DBERR("    file: %s offset: %u\n", loc->fileName.c_str(), loc->offset+offset);

    	fstream ofs(loc->fileName.c_str(),ios::out | ios::binary);
    	if (ofs.fail()) {
			DBERR("Error opening file %s for writing!\n", loc->fileName.c_str());
    	} else {
       		ofs.seekp(loc->offset+offset);
   	    	ofs.write(buffer,bytesPerSector);
       		ofs.close();
        }
    }   
}


void VirtualDrive::writeLEinc(unsigned char ** pBuffer, unsigned len, unsigned long aValue) { 

    DBERR("writeLE:\n");
    unsigned long value = aValue;
    for (unsigned int i=len;i>0;i--) { 
        unsigned char part = value & 0xff;
        DBERR("value: 0x%04X\n", part);
        value = value >> 8;
        **pBuffer = part;
        (*pBuffer)++; 
    }
}

void VirtualDrive::writeLE(unsigned char * buffer, unsigned len, unsigned long aValue) { 

    DBERR("writeLE:\n");
    unsigned long value = aValue;
    for (unsigned int i=len;i>0;i--) { 
        unsigned char part = value & 0xff;
        DBERR("value: 0x%04X\n", part);
        value = value >> 8;
        *buffer = part;
        buffer++; 
    }
}

void VirtualDrive::writeStr(unsigned char ** pBuffer, char * str) {

    DBERR("VirtualDrive::writeStr\n");
    
    unsigned int len = strlen(str);
    memcpy(*pBuffer, str, len);    
    (*pBuffer) += len;
    
    DBERR("VirtualDrive::writeStr ends\n");
    
}
