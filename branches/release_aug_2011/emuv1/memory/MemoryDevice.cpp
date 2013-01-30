// MemoryDevice.cpp

#include "MemoryDevice.h"
#include "Debug.h"
#include "devices/SlotSelector.h"
#include <fstream>

using namespace std;

//see EmptyPage::EmptyPage()
// waarom niet in de constructor deze opvragen en bewaren?
const nw_byte   *MemoryDevice::dummyReadPage = 0; 
EmptyPage       *MemoryDevice::emptyPage = 0;

MemoryDevice::MemoryDevice() {
    
    DBERR("MemoryDevice constructor...\n");
    
    idString = "unassigned";
    mainSlot = 255;
    subSlot = 255;
    offset = 0xFFFFFFFF;
	slotSelector = SlotSelector::Instance();
	releaseMemory = false;
}

void MemoryDevice::loadRom(string theFilename) {
    
    string filename = Debug::Instance()->getPath()+theFilename;
	DBERR("MemoryDevice::loadRom: %s\n", filename.c_str());
    idString = idString + string(" (") + theFilename + ")";
	ifstream romfile(filename.c_str(),ios::binary);
	if (romfile.fail()) {
		DBERR("Error opening file!\n");
		fileSize = 0;
		deviceSize = 0;
		assert(false);
    }
	romfile.seekg(0, ios::end);
	fileSize = romfile.tellg();
	
	DBERR(" fileSize: %u\n", fileSize); 
	
	romfile.seekg(0);
	
	char *newmemChars = new char[fileSize];

	DBERR("  created rom at 0x%08X\n", newmemChars);
	DBERR("  reading %u bytes.\n", fileSize);

	romfile.read(newmemChars, fileSize);
	romfile.close();
	
	unsigned int sizeInBlocks = (fileSize / 0x2000);
	if (fileSize & 0x1fff) {
		sizeInBlocks += 1;
	}
	
    nw_byte * newMem = new nw_byte[sizeInBlocks*BLOCKSIZE];
	// transfer roms from unsigned char memory to native (faster?) datatype
	for (unsigned int i=0;i<fileSize;i++) {
        newMem[i] = newmemChars[i] & 0xff;
    }    
    memory = newMem;
    releaseMemory = true;
    delete [] newmemChars;
}

MemoryDevice::~MemoryDevice() {

	DBERR("MemoryDevice destruction for >> %s <<\n", idString.c_str());
        
    if (releaseMemory) {
	    DBERR("I will destroy: 0x%08X\n", memory);
        delete [] memory;
    }

    DBERR("MemoryDevice destruction ends\n");
}

void MemoryDevice::patch(nw_word address, nw_byte value) {
	
	assert(offset != 0xFFFFFFFF);
	nw_byte * writeable = (nw_byte *) memory;
	writeable[address-offset] = value;
}

string MemoryDevice::getIdString() { 
        
    return idString;
}
