// Mapper.cc

#include "stdio.h"
#include <Debug.h>
#include "cpu/Z80.h"
#include "Mapper.h"
#include "cpu/Disassembler.h"

using namespace std;

/*! \brief creates a new mapper of memBanks 16 KB banks
 *  \param memBanks size in 16 KB banks of the mapper to create
 *
 *  Hier kan een wat langere omschrijving, zoveel regels als je wilt
 *
 */
Mapper::Mapper(nw_byte memBanks) {

	DBERR("Mapper constructor...\n");

    idString = "Mapper";

	/* maak geheugen, memBanks x 16 KB) */
	memory = new nw_byte[memBanks*16*1024];  // inherited from MemoryDevice
	memset((void *) memory, 0xff, memBanks*16*1024);
    
    DBERR("Mapper start: %u\n", memory);
    DBERR("Mapper ends : %u\n", memory+(memBanks*16*1024));

	/* masker voor ioports 0x0FCh - 0x0FFh
	   dit simuleert niet-aangesloten memory, dit wordt soms door programma's gebruikt voor memory-grootte-detectie
	*/

	maxBank = memBanks-1;
	bankmask = maxBank ^ 0xFF;

    unsigned int bank = 0;
    unsigned int p=4;
    do {
        p--;
        mapPage[p] = (nw_byte *) memory+(bank*16*1024);
        bankOfPage[p] = bank;
        DBERR("bank init: 0x%08x bank: %u p: %u\n", mapPage[p], bank, p);
        if (bank == maxBank) bank = 0; else bank++;
    } while (p != 0);
    
	wipe();	// fill entire memory with 0's 	(depends on maxBank to be set)
	DBERR("Mapper constructor...finished\n");
}

Mapper::~Mapper() {

	DBERR("Mapper destructor\n");
	delete [] memory;
	DBERR("Mapper destroyed.\n");
}

void Mapper::wipe() {

		memset((nw_byte *) memory,0,sizeof(nw_byte)*(maxBank+1)*16*1024);                // fill entire memory with 0's
}

/*! \brief Swiches a page (0-4) to a specified memory-bank
 *  \param page the page to switch
 *  \param bank the bank the switch to
 *
 */
void Mapper::setPage(nw_byte page, nw_byte bank) {

	bankOfPage[page] = bank & maxBank;
	mapPage[page] = (nw_byte *) memory+(bankOfPage[page]*16*1024);
	
	slotSelector->updateSelection(page*2);
	slotSelector->updateSelection(page*2+1);
}

/*! \brief Swiches a page (0-4) to a specified memory-bank
 *  \param page the page to query
 *  \return the bank currently switch
 *
 */
nw_byte Mapper::getPage(nw_byte page) {

	nw_byte result = bankOfPage[page] | bankmask;
	return result;
}

void Mapper::dump() {

	DBERR("page0: bank: %u\n", bankOfPage[0]);
	Debug::Instance()->memDump(mapPage[0]);

	DBERR("page1: bank: %u\n", bankOfPage[1]);
	Debug::Instance()->memDump(mapPage[1]);

	DBERR("page2: bank: %u\n", bankOfPage[2]);
	Debug::Instance()->memDump(mapPage[2]);

	DBERR("page3: bank: %u\n", bankOfPage[3]);
	Debug::Instance()->memDump(mapPage[3]);
}

void Mapper::saveState() {

	string filename("mapper.state");
	ofstream ofs_delete(filename.c_str(),ios::trunc);
	ofs_delete.close();

	ofstream ofs(filename.c_str(),ios::binary);
	if (ofs.fail()) DBERR("Error opening file %s\n", filename.c_str());
	ofs.write((char *)memory,(maxBank+1)*16*1024);
	ofs.close();
}

void Mapper::loadState() {

	string filename("mapper.state");
	ifstream ifs(filename.c_str(),ios::in|ios::binary);
	if (ifs.fail()) DBERR("Error opening file %s\n", filename.c_str());
	ifs.read((char *)memory,(maxBank+1)*16*1024);
	ifs.close();
}

void Mapper::write(nw_word address, nw_byte value) {
    unsigned int page = address >> 14;
    unsigned int offset = address & 0x3fff;
//    DBERR("write, address: " << address << " page " << page << endl);

    *(mapPage[page]+offset) = value;
}

void Mapper::install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address) {

    DBERR("Mapper::install in slot %u-%u\n", mainSlot, subSlot);
    offset = address;
    
	for (unsigned int b=0;b<8;b++) {
        DBERR("  %u-%u in block: %u\n", mainSlot, subSlot, b);
		layout[mainSlot][subSlot][b].bind(this, &Mapper::activate);
	}
}

void Mapper::activate(unsigned int block) {

//    DBERR("Mapper::activate block " << block << endl);
    unsigned int page = block >> 1;
    nw_word offset = (block & 1) * BLOCKSIZE;                       // use this syntax in romblock/ramblock
    Z80::Instance()->readBlock[block] = mapPage[page]+offset;
    Z80::Instance()->writeFunc[block].bind(this, &Mapper::write);

    // for debugging info on memoryDevice names
    slotSelector->setPage(block >> 1, this);
}

void Mapper::reset() {

    setPage(0,3);
    setPage(1,2);
    setPage(2,1);
    setPage(3,0);
}
