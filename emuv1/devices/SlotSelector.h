//! SlotSelector.h
#ifndef SLOTSELECTOR_H
#define SLOTSELECTOR_H

#include <string>
#include <list>
#include "msxtypes.h"
#include "FastDelegate.h"
#include "memory/RomBlock.h"
#include "memory/Mapper.h"
#include "memory/MapperKonami4.h"
#include "memory/MapperKonami5.h"
#include "memory/MapperMsxDos2.h"
#include "memory/MapperFMPAC.h"
#include "memory/NowindInterface.h"
#include "memory/WD279X.h"

#include "cpu/Z80.h"

class EmptyPage;

class SlotSelector {

friend class Mapper;   //!< make the Mapper a friend to allow it to change the slot-layout

private:
        Uint8			selectedMainSlot[4];
        Uint8			ffffRegister[4];
        bool            slotExpanded[4];
        unsigned int 	blockCount;
        layoutType      layout;

		Z80				*cpu;
		SlotSelector();
		std::list<MemoryDevice *> devices;
        MemoryDevice * currentPage[4];
        EmptyPage * emptyPage;
public:
        NowindInterface * usbInterface;
        WD279X * wd279x;

        static SlotSelector * Instance();
		void configure(unsigned int msxVersion);
		void setMainSlot(nw_byte selection);
		void setSubSlot(nw_byte selection);
		void updateSelection();
        void updateSelection(unsigned int block);
        void addMemoryDevice(MemoryDevice *mem, Uint8 mainSlot, Uint8 subSlot, nw_word address);
        bool possibleSubslotRead();
        bool isExpanded(nw_byte);
        Uint8 getSelectedMainSlot(Uint8 page);
        Uint8 getSelectedSubSlot(Uint8 page);
        nw_byte getSubSlotSelection(int);
		void putRam(unsigned int size, unsigned short mainSlot, unsigned short subSlot, nw_word address);
        std::string getDeviceName(Uint8 page);
        void setPage(Uint8 page, MemoryDevice * mem);
        bool isIllegalAddress(nw_word address);
         ~SlotSelector();
};

#endif


