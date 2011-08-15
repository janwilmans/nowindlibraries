//! MapperMsxDos2.h
#ifndef MAPPERMSXDOS2_H
#define MAPPERMSXDOS2_H

#include "msxtypes.h"
#include "MemoryDevice.h"

class MapperMsxDos2 : public MemoryDevice {
private:
			unsigned int switchedBlock;
public:
			MapperMsxDos2(std::string filename);
            nw_byte read(nw_word address);
            
            virtual void write(nw_word address, nw_byte value);
            virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
            virtual void activate(unsigned int block);
            virtual ~MapperMsxDos2();
};

#endif


