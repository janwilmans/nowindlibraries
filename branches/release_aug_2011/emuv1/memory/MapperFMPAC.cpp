// MapperFMPAC.cpp

#include <string>
#include "MapperFMPAC.h"
#include "Debug.h"
#include "audio/YM2413.h"
#include "cpu/Z80.h"

#define DUMMY 0

using namespace std;



MapperFMPAC::MapperFMPAC(string filename) {
	
	DBERR("MapperFMPAC constructor...\n");

    idString = "MapperFMPAC";

	loadRom(filename);
	deviceSize = 2;
	switchedBlock = 0;
	mem0x5ffe = 0;
	mem0x5fff = 0;

	for (int i=0;i<8192;i++) sram[i] = 0xff;
	
	sramFilename = Debug::Instance()->getPath() + "nowind.pac";
	ifstream ifs(sramFilename.c_str(),ios::in|ios::binary);

	if (ifs.fail()) {
		DBERR("Error loading FMPAC SRAM backup (nowind.pac)!\n");
	} else {
		ifs.seekg(0, ios::end);
		unsigned long fileSize = ifs.tellg();
		ifs.seekg(0);
		
		if (fileSize == 8206) {
		    char * buffer = new char[8206];       
			ifs.read(buffer,8206);
			for (int i=0;i<8190;i++) {
				sram[i] = buffer[i+16] & 0xff;
			}  
        	delete [] buffer;  
		} else {
			DBERR(" FMPAC SRAM backup (nowind.pac) corrupt!\n");
		}
		ifs.close();
	}
	DBERR("MapperFMPAC constructor...finished\n");
}

MapperFMPAC::~MapperFMPAC() {

    DBERR("MapperFMPAC destruction...\n");
    
	ofstream ofs(sramFilename.c_str(),ios::binary|ios::trunc);
	if (ofs.fail()) {
		DBERR("Error writing FMPAC SRAM backup (nowind.pac)!\n");
	} else {
		// create new buffer to ensure that the native datatype is converted to chars		    
        char * buffer = new char[8206];
		strcpy(buffer, "PAC2 BACKUP DATA");
		for (int i=0;i<8190;i++) {
			buffer[i+16] = sram[i];
        }    
		ofs.write(buffer,8206);
		ofs.close();
        delete [] buffer;			
	}

    DBERR("MapperFMPAC destruction ends\n");
}

nw_byte MapperFMPAC::read(nw_word address) {
	
	assert(false);                         //should not used, maybe for future use.
	return memory[address-offset];
}

void MapperFMPAC::write(nw_word address, nw_byte value) {

    assert (address >= 0x4000 && address <0x8000);
    //DBERR("MapperFMPAC::write address 0x" << hex << address << " = " << (unsigned int)value << endl);

	switch (address) {
	case 0x5ffe:
		mem0x5ffe = value;
		activate(2);
		activate(3);
		break;
	case 0x5fff:
		mem0x5fff = value;
		activate(2);
		activate(3);
		break;

	case 0x7ff4:
		// write OPLL register port (write only)
		YM2413::Instance()->writeAddress(value);
		break;
	case 0x7ff5:
		// write OPLL data port (write only)
		YM2413::Instance()->writeData(value);
		break;
	case 0x7ff6:{
		// activate OPLL (read/write)
		nw_byte *tmp = (nw_byte *)memory;
		tmp[address - offset] = value;
		break;
	}
	case 0x7ff7:
		// rom mapper
		nw_byte *tmp = (nw_byte *)memory;
		tmp[address - offset] = value;

		switchedBlock = value & 3;
		activate(2);
		activate(3);
		break;
	}
}

void MapperFMPAC::install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address) {
	
    DBERR("MapperFMPAC::install\n");
	unsigned int startBlocks = address / BLOCKSIZE;
	offset = address;

	for (unsigned int b=0;b<deviceSize;b++) {
        DBERR("  %u-%u in block: %u\n", mainSlot, subSlot, startBlocks+b);
		layout[mainSlot][subSlot][startBlocks+b].bind(this, &MapperFMPAC::activate);
	}
}

void MapperFMPAC::activate(unsigned int theBlock) {
    
    assert (theBlock == 2 || theBlock == 3);
    //DBERR("MapperFMPAC::activate bank " << dec << switchedBlock << " for block " << theBlock << endl);
    	
    if (mem0x5ffe == 0x4d && mem0x5fff == 0x69) {
		// sram active
		DBERR("SRAM ACTIVE!!\n");
		Z80::Instance()->readBlock[theBlock] = (theBlock == 2) ? sram:dummyReadPage;
	   	Z80::Instance()->writeFunc[theBlock].bind(this, &MapperFMPAC::write);
	} else {
		// rom active
		Z80::Instance()->readBlock[theBlock] = memory+(switchedBlock*16*1024)+((theBlock & 1)*8*1024);
    	Z80::Instance()->writeFunc[theBlock].bind(this, &MapperFMPAC::write);
	}
}
