//! RomBlock.h
#ifndef ROMBLOCK_H
#define ROMBLOCK_H

#include "msxtypes.h"
#include "MemoryDevice.h"

class RomBlock : public MemoryDevice {

public:
    RomBlock(std::string);
    nw_byte read(nw_word address);
    virtual void write(nw_word address, nw_byte value);
    virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
    virtual void activate(unsigned int block);
};

#endif

