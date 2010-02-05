//! MapperKonami5.h
#ifndef MAPPERKONAMI5_H
#define MAPPERKONAMI5_H

#include "msxtypes.h"
#include "MemoryDevice.h"
#include "audio/SCC.h"

class MapperKonami5 : public MemoryDevice {
private:
    unsigned int switchedBlock[4];
	SCC *scc;
public:
    MapperKonami5(std::string filename);
    nw_byte read(nw_word address);
    virtual void write(nw_word address, nw_byte value);
    virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
    virtual void activate(unsigned int block);
    virtual ~MapperKonami5();
};

#endif


