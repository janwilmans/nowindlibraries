//! Mapper.h
#ifndef MAPPER_H
#define MAPPER_H

#include <iostream>
#include <string>

#include "msxtypes.h"
#include "memory/MemoryDevice.h"

class Mapper : public MemoryDevice {

private:
		/* directe pointers naar de geschakelde bank */
		nw_byte    *mapPage[4];

		/* de nummers van de geschakelde bank */
		nw_byte    bankOfPage[4];
		nw_byte    bankmask;

		/* hoogste bank */
		nw_byte    maxBank;

public:

		Mapper(nw_byte);

		void    setPage(nw_byte, nw_byte);
		nw_byte getPage(nw_byte);
		void    saveState();
		void    loadState();
		void 	wipe();
		void    dump();
		void    reset();
		
        virtual void write(nw_word address, nw_byte value);
        virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
        virtual void activate(unsigned int block);
		virtual ~Mapper();
};

#endif
