// RomBlock.cpp
#include "RomBlock.h"
#include "Debug.h" 
#include "cpu/Z80.h" 

using namespace std;

RomBlock::RomBlock(string filename) {

    idString = "RomBlock";
	
	loadRom(filename);
	deviceSize = fileSize / BLOCKSIZE;
	for (unsigned int i=0;i<deviceSize;i++) {
        DBERR("memory: 0x%08X\n", memory+(i*BLOCKSIZE));
    }	
}

nw_byte RomBlock::read(nw_word address) {
	
	assert(false);                         //should not used, maybe for future use.
	return memory[address-offset];
}

void RomBlock::write(nw_word address, nw_byte value) {
    
}

void RomBlock::install(layoutType layout, Uint8 theMainSlot, Uint8 theSubSlot, nw_word address) {
	
    DBERR("RomBlock::install, deviceSize: %u\n", deviceSize);
    offset = address;
	mainSlot = theMainSlot;
	subSlot = theSubSlot;

	unsigned int startBlocks = address / BLOCKSIZE;
	for (unsigned int b=0;b<deviceSize;b++) {
        DBERR("  in slot %u-%u in block %u\n", mainSlot, subSlot, startBlocks+b);
		layout[mainSlot][subSlot][startBlocks+b].bind(this, &RomBlock::activate);
	}
}

void RomBlock::activate(unsigned int theBlock) {

//    DBERR("%s activate in block %u (%u-%u)\n", idString.c_str(), theBlock, mainSlot, subSlot);

    assert(offset != 0xFFFFFFFF);

	unsigned int blockOffset = offset / BLOCKSIZE;
	unsigned int block = theBlock - blockOffset;

    Z80::Instance()->readBlock[theBlock] = memory+(block*BLOCKSIZE);
    Z80::Instance()->writeFunc[theBlock].bind(this, &RomBlock::write);
    
    // for debugging info on memoryDevice names
    slotSelector->setPage(theBlock >> 1, this);    
}


