//! MapperKonami4.h
#ifndef MAPPERKONAMI4_H
#define MAPPERKONAMI4_H

#include "msxtypes.h"
#include "MemoryDevice.h"

class MapperKonami4 : public MemoryDevice {
private:
	  unsigned int switchedBlock[4];
public:
      MapperKonami4(std::string filename);
      nw_byte read(nw_word address);
      
      virtual void write(nw_word address, nw_byte value);
      virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
      virtual void activate(unsigned int block);
      virtual ~MapperKonami4();
};

#endif
