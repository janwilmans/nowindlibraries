//! MapperKonami5.h
#ifndef MAPPERFMPAC_H
#define MAPPERFMPAC_H

#include "msxtypes.h"
#include "MemoryDevice.h"

class MapperFMPAC : public MemoryDevice {

private:
			unsigned int 	switchedBlock;
			nw_byte			mem0x5ffe;
			nw_byte			mem0x5fff;
			nw_byte			sram[8192];
			std::string	   	sramFilename;

public:
			MapperFMPAC(std::string filename);
            nw_byte read(nw_word address);

            virtual void write(nw_word address, nw_byte value);
            virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
            virtual void activate(unsigned int block);
            virtual ~MapperFMPAC();
};

#endif


