// MapperKonami5.cpp

#include <string>
#include "MapperKonami5.h"
#include "Debug.h"
#include "cpu/Z80.h"


using namespace std;

MapperKonami5::MapperKonami5(string filename) {

    idString = "MapperKonami5";
	
	loadRom(filename);
	deviceSize = 4;
	for (unsigned int i=0;i<4;i++) {
		switchedBlock[i] = i;
	}
	scc = SCC::Instance();
}

nw_byte MapperKonami5::read(nw_word address) {
	
	assert(false);                         //should not used, maybe for future use.
	return memory[address-offset];
}

void MapperKonami5::write(nw_word address, nw_byte value) {

    assert (address >= 0x4000 && address <0xc000);
	//DBERR("MapperKonami5::write address 0x" << hex << address << " = " << (unsigned int)value << endl);

	if (address >= 0x5000 && address < 0x5800) { 
		switchedBlock[0] = value;
		activate(2);	
    } else if (address >= 0x7000 && address < 0x7800) { 
		switchedBlock[1] = value;
		activate(3);	
    } else if (address >= 0x9000 && address < 0x9800) { 
        if ((value & 0x3f) == 0x3f) {
			scc->activate(4);
        } else {
    		switchedBlock[2] = value;
    	   	activate(4);	
        }
    } else if (address >= 0xB000 && address < 0xB800) { 
		switchedBlock[3] = value;
		activate(5);	
    }
}

void MapperKonami5::install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address) {
	
    DBERR("MapperKonami5::install\n");
	unsigned int startBlocks = address / BLOCKSIZE;
	offset = address;

	for (unsigned int b=0;b<deviceSize;b++) {
        DBERR("  %u-%u in block: %u\n", mainSlot, subSlot, startBlocks+b);
		layout[mainSlot][subSlot][startBlocks+b].bind(this, &MapperKonami5::activate);
	}
}

void MapperKonami5::activate(unsigned int theBlock) {
    
	unsigned int blockOffset = offset / BLOCKSIZE;
	unsigned int bank = switchedBlock[theBlock - blockOffset];

    //DBERR("MapperKonami5::activate bank " << bank << " for block " << theBlock << endl);
	
    Z80::Instance()->readBlock[theBlock] = memory+(bank*BLOCKSIZE);
    Z80::Instance()->writeFunc[theBlock].bind(this, &MapperKonami5::write);
}

MapperKonami5::~MapperKonami5() {
    
    DBERR("MapperKonami5 destruction...\n");
}
