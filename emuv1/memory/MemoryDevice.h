//! MemoryDevice.h
#ifndef MEMORYDEVICE_H
#define MEMORYDEVICE_H

#include <string>
#include "msxtypes.h"
#include "FastDelegate.h"

class SlotSelector;
class EmptyPage;

static const unsigned int BLOCKSIZE = 8192;
static const unsigned int BLOCKS = 64*1024/BLOCKSIZE;
typedef fastdelegate::FastDelegate1<unsigned int> activationDelegate;   // returns void 
typedef activationDelegate layoutType[4][4][8]; 

class MemoryDevice {
    
protected:
    Uint8                   mainSlot;
    Uint8                   subSlot;
	static const nw_byte   *dummyReadPage;
	static EmptyPage       *emptyPage;
			
    std::string             idString;
    const nw_byte          *memory;
    bool                    releaseMemory;
    nw_word                 offset;
    unsigned int            deviceSize;
    unsigned long           fileSize;
    SlotSelector           *slotSelector;
            
public:
    MemoryDevice();
    void loadRom(std::string filename);
    virtual void write(nw_word address, nw_byte value) = 0;
    virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address) = 0;
    virtual void activate(unsigned int block) = 0;
    virtual ~MemoryDevice();
    
    void patch(nw_word address, nw_byte value);
    std::string getIdString();
};

#endif
