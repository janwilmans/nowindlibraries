// RamBlock.cpp

#include <string>
#include "RamBlock.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;

RamBlock::RamBlock(nw_word size) {

    idString = "RamBlock";
	
	nw_byte * newMem = new nw_byte[size];
	deviceSize = 4;
	memset(newMem,0,size*sizeof(nw_byte));
	memory = newMem;
	releaseMemory = true;
}

nw_byte RamBlock::read(nw_word address) {
	
	assert(false);                         //should not used, maybe for future use.
	return memory[address-offset];
}

void RamBlock::write(nw_word address, nw_byte value) {

	nw_byte * writeable = (nw_byte *) memory;
	writeable[address-offset] = value;
}

void RamBlock::install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address) {
	
    DBERR("RamBlock::install\n");

	unsigned int startBlocks = address / BLOCKSIZE;
	offset = address;

	for (unsigned int b=0;b<deviceSize;b++) {
		DBERR("  (%u-%u) in block: %u\n", mainSlot, subSlot, startBlocks+b);
		layout[mainSlot][subSlot][startBlocks+b].bind(this, &RamBlock::activate);
	}
}

void RamBlock::activate(unsigned int theBlock) {

//    DBERR("RamBlock::activate for block " << theBlock << endl);
    
	unsigned int blockOffset = offset / BLOCKSIZE;
	unsigned int block = theBlock - blockOffset;
    Z80::Instance()->readBlock[theBlock] = memory+(block*BLOCKSIZE);
    Z80::Instance()->writeFunc[theBlock].bind(this, &RamBlock::write);
}
