//! RamBlock.h
#ifndef RAMBLOCK_H
#define RAMBLOCK_H

#include "msxtypes.h"
#include "MemoryDevice.h"

class RamBlock : public MemoryDevice {
public:
    RamBlock(nw_word size);
    nw_byte read(nw_word address);
    
    virtual void write(nw_word address, nw_byte value);
    virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
    virtual void activate(unsigned int block);
};

#endif


