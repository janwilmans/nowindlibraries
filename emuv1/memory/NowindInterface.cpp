
// NowindInterface.cpp

#include <string>
#include "NowindInterface.h"
#include "Debug.h"
#include "cpu/Z80.h"
#include "memory/EmptyPage.h"
#include "../nwhost/libnwhost.h"

using namespace std;

#include "SDL_thread.h"

void debugout(const char *msg)
{
    DBERR(msg);
}

NowindInterface::NowindInterface(string filename) {
    
    idString = "NowindInterface";
    
	nwhost::initialize();

	loadRom(filename);
	deviceSize = 8;            /* in 8kb blocks */
    numberOfLatencyReads = 0;
	
	Uint32 switchedBlocks = fileSize/(16*1024);
	switchedBlockMask = switchedBlocks-1;
	DBERR("switchedBlockMask: %u\n", switchedBlockMask);
	assert(fileSize == (switchedBlocks*16*1024));
	
    switchedBlock = 0; // TODO: random!
}

NowindInterface::~NowindInterface() {
     DBERR("NowindInterface (LIBNOWIND) destruction...\n");
//     nowindusb_cleanup();
}

void NowindInterface::insertDisk(int driveNr, string filename) {

    nowindusb_set_image(driveNr, filename.c_str());
}

void NowindInterface::insertHarddisk(int driveNr, string filename, int partitionNr) {

    nowindusb_set_harddisk_image(driveNr, partitionNr, false, filename.c_str());
}

/*
 * should normally not be used in a memory-device,
 * but we make an exception for the NowindInterface
 */
nw_byte NowindInterface::read(nw_word address) {

    nw_byte val = nowindusb_read();
    //DBERR(" USBInterface read addr: 0x%02x value: 0x%02x\n", address, val);
	return val;
/*    
// TODO: is nowindusb_read() blocking?
// een waarde van 1000 voor numberOfLatencyReads lijkt goed te werken.

    if (numberOfLatencyReads == 1000) return nowindusb_read();
    numberOfLatencyReads++;
    if ((numberOfLatencyReads & 0xff)==0) {
        DBERR("numberOfLatencyReads %i\n",numberOfLatencyReads);
    }
    return 255;
*/
}

void NowindInterface::write(nw_word address, nw_byte value) {

	numberOfLatencyReads = 0;
//    DBERR("flashRom write op addr: 0x%04x   case: 0x%04x\n", address, address & 0xe000);
    
    switch(address & 0xe000) {
	case 0x0000:
		DBERR("flashRom write op addr: 0x%04X\n", address);
	case 0x4000:
	case 0x8000:
		//DBERR("nowindusb_write(0x%02x)\n", value);
        nowindusb_write(value);
		break;

	case 0x6000:
	case 0xa000:

        // snelle hack om nowind_sunrise te ondersteunen (mapper reageert alleen op oneven adressen)
        if ((address & 1)==0) {
            DBERR(" GEEN SWITCH val: %u (at addr: 0x%04x)\n", value, address);
            break;
        }
        
        switchedBlock = value & switchedBlockMask;
        //DBERR("NowindInterface switchedBlock: %u (at addr: 0x%04x)\n", switchedBlock, address);
		
		// both or neither might be switched, depending on current slotselection
		slotSelector->updateSelection(2);
		slotSelector->updateSelection(3);
		slotSelector->updateSelection(4);
		slotSelector->updateSelection(5);
	    break;
	default:            
        DBERR("unsupported NowindInterface write (address = 0x%04X, value = 0x%02X\n", address, value);
		break;
	}
}

bool NowindInterface::isEnabled(nw_word address) {

    if (address >= 0x2000 && address <= 0x3fff) {
        Uint8 ms = slotSelector->getSelectedMainSlot(0);
        if (ms == mainSlot) {
            if (!slotSelector->isExpanded(ms)) return true;
            if (slotSelector->getSelectedSubSlot(0) == subSlot) return true;
        }
        return false;
    }

    if (address >= 0x8000 && address <= 0x9fff) {
        Uint8 ms = slotSelector->getSelectedMainSlot(2);
        if (ms == mainSlot) {
            if (!slotSelector->isExpanded(ms)) return true;
            if (slotSelector->getSelectedSubSlot(2) == subSlot) return true;
        }
    }
    return false;
}

/*
 * will be called for each slot the memorydevice in installed in, usually this is just once,
 * except for un-expanded slots!
 */
void NowindInterface::install(layoutType layout, Uint8 theMainSlot, Uint8 theSubSlot, nw_word address) {
	
	mainSlot = theMainSlot;
	subSlot = theSubSlot;
	unsigned int startBlocks = address / BLOCKSIZE;
	offset = address;

	for (unsigned int b=0;b<deviceSize;b++) {
        DBERR("  NowindInterface::install (%u-%u) in block: %u\n", mainSlot, subSlot, startBlocks+b);
        assert((startBlocks+b) <= 7);
		layout[mainSlot][subSlot][startBlocks+b].bind(this, &NowindInterface::activate);
	}
}

/* this method MUST activate the requested block */
void NowindInterface::activate(unsigned int theBlock) {
    
    assert(offset != 0xFFFFFFFF);
    
    //DBERR("NowindInterface::activate bank " << dec << switchedBlock << " for block " << theBlock << endl);
    if (theBlock == 2 || theBlock == 3 || theBlock == 5) 
      Z80::Instance()->readBlock[theBlock] = memory+(switchedBlock*16*1024)+((theBlock & 1)*8*1024);
    else 
      Z80::Instance()->readBlock[theBlock] = dummyReadPage;

    if (theBlock == 2 || theBlock == 3 || theBlock == 4 || theBlock == 5) {
        Z80::Instance()->writeFunc[theBlock].bind(this, &NowindInterface::write);
        //DBERR("NowindInterface::activated writing for 0x%04X-0x%04X\n", theBlock*8*1024, (theBlock*8*1024)+0x1FFF);
    } else {
        Z80::Instance()->writeFunc[theBlock].bind(emptyPage, &EmptyPage::write);
    }
    
    // for debugging info on memoryDevice names
    slotSelector->setPage(theBlock >> 1, this);
}

