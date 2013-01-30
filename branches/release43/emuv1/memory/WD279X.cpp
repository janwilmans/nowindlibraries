// WD279X.cpp

#include <string>
#include "WD279X.h"
#include "Debug.h"
#include "cpu/Z80.h"
#include "memory/EmptyPage.h"
#include "memory/WD279X.h"

using namespace std;

WD279X::WD279X(string filename) {

    idString = "WD279X";

	loadRom(filename);
	deviceSize = 2;            /* in 8kb blocks */
	
	assert(fileSize == 16*1024);
    disk = new DiskDrive();
    
    trackRegister = 0;
    statusRegister = 0;
    dataRegister = 0;
    sectorRegister = 0;  // 1???
    commandRegister = 0;
    seekDirection = 1; // ???
    transferCounter = 0;
    diskSide = 0;
    
    currentSector = 0;
    currentTrack = 0;

    dataRequest = false;
    intRequest = false;
}

void WD279X::insertDisk(string filename) {

    disk->openDiskImage(filename);
}
/*
wd2791/93 single sided
wd2795/97 double sided
*/
bool WD279X::isEnabled(nw_word address) {

    if (address >= 0x7FF8 && address <= 0x7FFF) {
        Uint8 ms = slotSelector->getSelectedMainSlot(1);
        if (ms == mainSlot) {
            if (!slotSelector->isExpanded(ms)) return true;
            if (slotSelector->getSelectedSubSlot(1) == subSlot) return true;
        }
    }
    return false;
}

/*
 * should normally not be used in a memory-device,
 * but we make an exception for the WD279X
 */
nw_byte WD279X::read(nw_word address) {

    nw_byte val = 0xff;    
    switch (address) {
    case 0x7ff8: val = getStatusRegister(); break;
    case 0x7ff9: val = trackRegister; break;
    case 0x7ffa: val = sectorRegister; break;
    case 0x7ffb: val = readDataRegister(); break;
    case 0x7fff: val = (dataRequest ? 0x40:0x00) | (intRequest ? 0x80:0x00); break;
    default:
        //assert(false);
        break;
    }
    
    switch (address) {
    case 0x7ff8: DBERR("FDC read statusRegister(0x%02x)\n", val); break;
    case 0x7ff9: DBERR("FDC read trackRegister(0x%02x)\n", val); break;
    case 0x7ffa: DBERR("FDC read sectorRegister(0x%02x)\n", val); break;
//    case 0x7ffb: DBERR("FDC read getDataRegister(0x%02x)\n", val); break;
//    case 0x7fff: DBERR("FDC read interrupts(0x%02x)\n", val); break;
    }

    return val;
}

void WD279X::write(nw_word address, nw_byte value) {

	switch(address) {
	case 0x7ff8:   // command register
        commandRegister = value;
        switch (value & 0xf0) {
        
        // type I
        case 0x00:
            DBERR("FDC command: RESTORE\n");
            trackRegister = 0;
            statusCommandTypeI();
            break;

        case 0x10:  // seek
            DBERR("FDC command: SEEK track %u\n", dataRegister);
            trackRegister = 0; // ???? blue doet dat
            currentTrack = dataRegister;
            statusCommandTypeI();
            break;

        case 0x30: trackRegister = (trackRegister + seekDirection) & 0xff; // step
        case 0x20:
            currentTrack = (currentTrack + seekDirection) & 0xff;
            DBERR("FDC command: STEP/STEP_IN/STEP_OUT\n"); break;
            statusCommandTypeI();
            break;

        case 0x50: trackRegister = (++trackRegister) & 0xff; // step-in
        case 0x40:
            currentTrack++; 
            seekDirection = 1;
            statusCommandTypeI();
            break;
        
        case 0x70: trackRegister = (--trackRegister) & 0xff; // step-out
        case 0x60:
            currentTrack--;
            seekDirection = -1;
            statusCommandTypeI();
            break;
            
        // type II
        case 0x80:
        case 0x90: commandReadSector(); break;
        case 0xa0:
        case 0xb0: commandWriteSector(); break;
        
        // type III
        case 0xc0: DBERR("FDC command: READADDRESS\n"); break;
        case 0xe0: DBERR("FDC command: READTRACK\n"); break;
        case 0xf0: DBERR("FDC command: WRITETRACK\n"); break;
        
        // type IV
        case 0xd0:
            DBERR("FDC: force interrupt\n");
            assert((value & 0x0f) == 0);  // no interrupt
            if (statusRegister & BUSY) {
                //iets doen
                statusRegister &= ~BUSY;
            } else {
                //anders ook
            }
            dataRequest = false;
            intRequest = false;
            break;
        default: 
            assert(false);
        }
        break;
        
	case 0x7ff9: trackRegister = value; break;
	case 0x7ffa: sectorRegister = value; break;
	case 0x7ffb: writeDataRegister(value); break;
	case 0x7ffc: diskSide = value & 1; break;
    case 0x7ffd:
        // bit 0 side???
        // bit 1 ?? disk/density??
        // bit 2 ??
        // bit 7 ??
        DBERR("FDC DSELREG = 0x%02X\n", value);
        break;
    case 0x7ffe: break;
    case 0x7fff: break;
    	
	default:            
        DBERR("unsupported WD279X write (address = 0x%04X, value = 0x%02X\n", address, value);
		break;
	}		
}

void WD279X::statusCommandTypeI(void) {
    
    statusRegister &= ~(BUSY | CRC_ERROR | SEEK_ERROR);
    dataRequest = false;
    intRequest = true;
}

/*
 * will be called for each slot the memorydevice in installed in, usually this is just once,
 * except for un-expanded slots!
 */
void WD279X::install(layoutType layout, Uint8 theMainSlot, Uint8 theSubSlot, nw_word address) {
	
	mainSlot = theMainSlot;
	subSlot = theSubSlot;
	unsigned int startBlocks = address / BLOCKSIZE;
	offset = address;

	for (unsigned int b=0;b<deviceSize;b++) {
//        DBERR("  (" << mainSlot << "-" << subSlot << ") in block : " << startBlocks+b << endl);
		layout[mainSlot][subSlot][startBlocks+b].bind(this, &WD279X::activate);
	}
}

/* this method MUST activate the requested block */
void WD279X::activate(unsigned int theBlock) {
    
    assert(offset != 0xFFFFFFFF);
	unsigned int blockOffset = offset / BLOCKSIZE;
    Uint8 block = theBlock - blockOffset;
    assert(block < 2);
    
//    DBERR("WD279X::activate block " << theBlock << "(" << (Uint32) block << ")\n");
    Z80::Instance()->readBlock[theBlock] = memory+(block*8*1024);

    if (block == 1) {
        Z80::Instance()->writeFunc[theBlock].bind(this, &WD279X::write);
//        DBERR("WD279X::activated writing for 0x%04X-0x%04X\n", theBlock*8*1024, (theBlock*8*1024)+0x1FFF);
    } else {
        Z80::Instance()->writeFunc[theBlock].bind(emptyPage, &EmptyPage::write);
    }
    
    // for debugging info on memoryDevice names
    slotSelector->setPage(theBlock >> 1, this);
}

WD279X::~WD279X() {
    
    delete memory;
    DBERR("WD279X destruction...\n");
}

nw_byte WD279X::readDataRegister(void) {
    
    // TODO:
    // The user has the option of reading the status register through program control or using the DRQ line with
    // DMA or interrupt methods. When the data register is read, the DRQ bit in the status register and the DRQ
    // line are automatically reset. A write to the data register also causes both DRQ's to reset.
    
    nw_byte tmp = sectorBuffer[transferCounter++];
    //DBERR("readCounter %u\n", transferCounter);
    if (transferCounter == 512) {
        transferCounter = 0;
        if (commandRegister & MULTIPLE_RECORD_FLAG) {
            currentSector++;
            if (currentSector == 9) {
                currentTrack++;
                currentSector = 0;
            }
            assert(false);
            disk->readSectors(sectorBuffer, (currentTrack * 9) + currentSector, 1);
        } else {
            dataRequest = false;
            intRequest = true;
            statusRegister &= ~(BUSY);
        }
    }
    return tmp;
}

nw_byte WD279X::getStatusRegister(void) {
    
    nw_byte tmp = statusRegister;
    if (((commandRegister >> 4) < 8) && (currentTrack == 0)) tmp |= TRACK0;
    if (dataRequest) tmp |= 0x02;
    return tmp;
}

void WD279X::commandReadSector(void) {

    nw_word sectorNumber = (((trackRegister << 1) + diskSide) * 9) + sectorRegister - 1;
DBERR("FDC currentTrack %u\n", trackRegister); 
DBERR("FDC diskSide %u\n", diskSide); 
DBERR("FDC sectorRegister %u\n", sectorRegister); 

    DBERR("FDCcommand: READSECTOR (%u)\n", sectorNumber); //?????????????
    
    disk->readSectors(sectorBuffer, sectorNumber, 1);
    transferCounter = 0;
    dataRequest = true;
    intRequest = false;
    statusRegister &= ~(LOST_DATA | RECORD_NOT_FOUND | RECORD_TYPE | WRITE_PROTECT);
    statusRegister |= BUSY;
}

void WD279X::commandWriteSector(void) {

    nw_word sectorNumber = (((currentTrack << 1) + diskSide) * 9) + sectorRegister - 1;
    DBERR("FDCcommand: WRITESECTOR (%u)\n", sectorNumber);

    transferCounter = 0;
    dataRequest = true; // TODO: check!
    intRequest = false;
    statusRegister &= ~(LOST_DATA | RECORD_NOT_FOUND | RECORD_TYPE | WRITE_PROTECT);
    statusRegister |= BUSY;   
}

void WD279X::writeDataRegister(nw_byte value) {
    
    dataRegister = value;

    if (transferCounter == 512) {
        nw_word sectorNumber = (((currentTrack << 1) + diskSide) * 9) + sectorRegister - 1;
        disk->writeSectors(sectorBuffer, sectorNumber, 1);
        dataRequest = false;    // TODO: check!
        intRequest = true;
        statusRegister &= ~(BUSY);
    } else {
        sectorBuffer[transferCounter++] = value;
    }
}
