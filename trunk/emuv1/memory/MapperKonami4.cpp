// MapperKonami4.cpp

#include <string>
#include "MapperKonami4.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;

MapperKonami4::MapperKonami4(string filename) {
	
    idString = "MapperKonami4";

	loadRom(filename);
	deviceSize = 4;
	for (unsigned int i=0;i<deviceSize;i++) {
        DBERR("memory: 0x%08X\n", memory+(i*BLOCKSIZE));
    }	
	
	for (unsigned int i=0;i<4;i++) {
		switchedBlock[i] = i;
	}
}

nw_byte MapperKonami4::read(nw_word address) {
	
	assert(false);                         //should not used, maybe for future use.
	return memory[address-offset];
}

void MapperKonami4::write(nw_word address, nw_byte value) {

    assert (address >= 0x4000 && address <0xc000);
	//DBERR("MapperKonami4::write address 0x" << hex << address << " = " << (unsigned int)value << endl);

	switch(address) {
	case 0x6000:
		switchedBlock[1] = value;
		activate(3);					// uitrekenen?
	    break;
	case 0x8000:
		switchedBlock[2] = value;
		activate(4);					// uitrekenen?
	    break;
	case 0xA000:
		switchedBlock[3] = value;
		activate(5);					// uitrekenen?
	    break;
	default:
		break;
	}		
		
//	nw_byte * writeable = (nw_byte *) memory;
//	writeable[address-offset] = value;

}

void MapperKonami4::install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address) {
	
    DBERR("MapperKonami4::install\n");
	unsigned int startBlocks = address / BLOCKSIZE;
    offset = address;
    
	for (unsigned int b=0;b<deviceSize;b++) {
        DBERR("  %u-%u in block: %u\n", mainSlot, subSlot, startBlocks+b);
		layout[mainSlot][subSlot][startBlocks+b].bind(this, &MapperKonami4::activate);
	}
}

void MapperKonami4::activate(unsigned int theBlock) {
    
	unsigned int blockOffset = offset / BLOCKSIZE;
	unsigned int bank = switchedBlock[theBlock - blockOffset];

//    DBERR("MapperKonami4::activate bank " << bank << " for block " << theBlock << endl);
	
    Z80::Instance()->readBlock[theBlock] = memory+(bank*BLOCKSIZE);
    Z80::Instance()->writeFunc[theBlock].bind(this, &MapperKonami4::write);
}

MapperKonami4::~MapperKonami4() {
    
    DBERR("MapperKonami4 destruction...\n");
}
