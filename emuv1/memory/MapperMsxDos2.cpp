// MapperMsxDos2.cpp

#include <string>
#include "MapperMsxDos2.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;

MapperMsxDos2::MapperMsxDos2(string filename) {

    idString = "MapperMsxDos2";

	loadRom(filename);
	deviceSize = 2; 
    switchedBlock = 0;
}

nw_byte MapperMsxDos2::read(nw_word address) {
	
	assert(false);                         //should not be used, maybe for future use.
	return memory[address-offset];
}

void MapperMsxDos2::write(nw_word address, nw_byte value) {

    assert (address >= 0x4000 && address <0x8000);
//    DBERR("MapperMsxDos2::write address 0x%04x = 0x%04x\n", address, value);

	switch(address) {
	case 0x6000:
		DBERR("MapperMsxDos2::switch bank %u\n", value);
		switchedBlock = value & 3;
		activate(2);
		activate(3);
	    break;
	default:
		break;
	}		
		
//	nw_byte * writeable = (nw_byte *) memory;
//	writeable[address-offset] = value;

}

void MapperMsxDos2::install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address) {
	
    DBERR("MapperMsxDos2::install\n");
	unsigned int startBlocks = address / BLOCKSIZE;
	offset = address;

	for (unsigned int b=0;b<deviceSize;b++) {
        DBERR("  (%u-%u) in block: %u\n", mainSlot, subSlot, startBlocks+b);
		layout[mainSlot][subSlot][startBlocks+b].bind(this, &MapperMsxDos2::activate);
	}
}

void MapperMsxDos2::activate(unsigned int theBlock) {
    
//    DBERR("MapperMsxDos2::activate bank " << dec << switchedBlock << " for block " << theBlock << endl);
    	
    Z80::Instance()->readBlock[theBlock] = memory+(switchedBlock*16*1024)+((theBlock & 1)*8*1024);
    Z80::Instance()->writeFunc[theBlock].bind(this, &MapperMsxDos2::write);

    // for debugging info on memoryDevice names
    slotSelector->setPage(theBlock >> 1, this);
}

MapperMsxDos2::~MapperMsxDos2() {
    
    DBERR("MapperMsxDos2 destruction...\n");
}
