// SlotSelector.cpp

#include <string>
#include "SlotSelector.h"
#include "Debug.h"
#include "audio/SCC.h"
#include "Media.h"

#include "memory/EmptyPage.h"

using namespace std;
using namespace fastdelegate;

SlotSelector * SlotSelector::Instance() {
	// singleton class 
	static SlotSelector deInstantie;
	return &deInstantie;
}

SlotSelector::SlotSelector() {
    
    DBERR("SlotSelector constructor...\n");

    usbInterface = 0;
    cpu = Z80::Instance();

    blockCount = 64*1024/BLOCKSIZE;

    for (unsigned int slot=0;slot<4;slot++) {		
        slotExpanded[slot] = false;
	}
	
    for (unsigned int page=0;page<4;page++) {		
        selectedMainSlot[page] = 0;			
        ffffRegister[page] = 0;
    }
}

void SlotSelector::configure(unsigned int msxVersion) {
    
    DBERR("SlotSelector::configure...\n");

    /* SlotSelector initialization */
    emptyPage = new EmptyPage();    // TODO: Jan? Waarom mag dit niet in de constructor? Jan: Mag best, maar hier wordt hij voor het eerst gebruikt, ik denk dat hij daarom hier gemaakt wordt, wat mij betreft verplaatst hij naar de constructor. commentaar in de destructor kan dan ook weg..
    for (unsigned int m=0;m<4;m++) {
        for (unsigned int s=0;s<4;s++) {
            for (unsigned int b=0;b<8;b++) {
                layout[m][s][b].bind(emptyPage, &EmptyPage::activate);
            }
        }
    }
    /* end of SlotSelector initialization */

	if (msxVersion == 1) {
		/* load a MSX1 main-rom at SLOT 0, page 0 and 1 */
		addMemoryDevice(new RomBlock("../roms/hx-10_basic-bios1.rom"),0,0,0x0000);

		Z80::Instance()->mapper = new Mapper(4);
		addMemoryDevice(Z80::Instance()->mapper,2,0,0x0000);

        usbInterface = new NowindInterface("../msxsrc/nowindDos1.rom");
//        usbInterface = new NowindInterface("../msxsrc/nowindDos2.rom");        
        addMemoryDevice(usbInterface, 1, 0, 0x0000);
//        usbInterface->insertDisk("../disks/MSX20th.dsk");
//        usbInterface->insertDisk("../disks/dos2.dsk");        
        usbInterface->insertDisk("../disks/wb.dsk");
//    addMemoryDevice(new MapperMsxDos2("../roms/MSXDOS22_msx1.ROM"),3,0,0x4000);
	}
	
	if (msxVersion == 2) {
        
        //slotExpanded[0] = true;
        //slotExpanded[1] = true;
        slotExpanded[2] = true;
		slotExpanded[3] = true;
		RomBlock *mainRom = new RomBlock("../roms/MSX2.ROM");
        addMemoryDevice(mainRom, 0, 0, 0x0000);

		RomBlock *subRom = new RomBlock("../roms/MSX2EXT.ROM");
		addMemoryDevice(subRom, 3, 0, 0x0000);            // MSX2 sub-rom 3-1, address 0x000
		
        mainRom->patch(0x0180, 0xed);   // TURBO-R CHGCPU
        mainRom->patch(0x0181, 0x0a);   // assert(false)
		// NO start-up logo, patch should be applied to SUB-ROM 
		subRom->patch(0x2a0e,0);
		subRom->patch(0x2a0f,0);
		subRom->patch(0x2a10,0);
		
		// NO boot-delay, patch should be applied to SUB-ROM 
		subRom->patch(0x041d,0x18);
		subRom->patch(0x041e,2);
		
		Z80::Instance()->mapper = new Mapper(64);
		addMemoryDevice(Z80::Instance()->mapper,3,2,0x0000);
		
//        addMemoryDevice(new MapperKonami5("../roms/MJTT.ROM"),1,0,0x4000);

//		RomBlock *bla = new RomBlock("../../MSX Games/roms/THEXDER.ROM");
//      RomBlock *bla = new RomBlock("../../MSX Games/roms/Theseus.rom"); // 0x8000!!!
//		RomBlock *bla = new RomBlock("../roms/kobashi.rom");
//		addMemoryDevice(bla, 1, 0, 0x4000);
    
// KONAMI4 8kB
//		putMegaRom("../MSX Games/konami4/Metal Gear.rom",1,0,0x4000);
// KONAMI5 8kB met SCC
//		putMapperKonami5("../MSX Games/konami5/quarth.rom",1,0,0x4000);				
//		putMegaRom("../MSX Games/konami5/SOLID.ROM",1,0,0x4000);				
// ANDERE ROMS:
//		putMegaRom("../MSX Games/konami4/HEAVEN.ROM",1,0,0x4000);		// 6800/7000/7800
//		putMegaRom("../MSX Games/1942.rom",1,0,0x4000);					// writes to 6800/7800				
//		addMemoryDevice(new MapperKonami5("../MSX Games/konami5/Kings Valley2 (msx2).rom"),1,0,0x4000);
	
#define NOWINDDOS1_off
#define NOWINDDOS2_off
#define NOWIND              // combines MSXDOS 1&2
#define OPENDISKROM_off
#define NORMALDISKROM_off
#define MSXDOS2_off
#define WD279X_off

#ifdef NOWIND
    usbInterface = new NowindInterface("../msxsrc/nowind.rom");
    addMemoryDevice(usbInterface, 2, 1, 0); 
//  usbInterface->insertDisk("../disks/test.dsk");
    usbInterface->insertDisk("../disks/wb.dsk"); 
#endif


#ifdef OPENDISKROM
    usbInterface = new NowindInterface("../msxsrc/openDiskrom.rom");
    addMemoryDevice(usbInterface, 2, 1, 0); 
//  usbInterface->insertDisk("../disks/test.dsk");
    usbInterface->insertDisk("../disks/wb.dsk");        
#endif

#ifdef NOWINDDOS1
//    usbInterface = new NowindInterface("../msxsrc/openDiskrom.rom");
//    usbInterface = new NowindInterface("../msxsrc/nowindDos1.rom");
//    addMemoryDevice(usbInterface, 3, 3, 0);
//    usbInterface->insertDisk("../disks/wb.dsk");    

    usbInterface = new NowindInterface("../msxsrc/nowindDos1.rom");
    addMemoryDevice(usbInterface, 2, 1, 0);
//    usbInterface->insertDisk("../disks/dos1.dsk");        
//    usbInterface->insertDisk("../disks/test.dsk");
//    usbInterface->insertDisk("../msxdisks/dos2.dsk");
//    usbInterface->insertDisk("../disks/dos1.dsk");
//    usbInterface->insertDisk("../disks/hd5mbdos2.dsk");
//    usbInterface->insertDisk("../disks/MSX20th.dsk");
     usbInterface->insertDisk("../disks/wb.dsk");    
#endif


#ifdef NOWINDDOS2
    usbInterface = new NowindInterface("../msxsrc/nowindDos2.rom");
    addMemoryDevice(usbInterface, 2, 1, 0);
//    usbInterface->insertDisk("../msxdisks/dos2.dsk");    
//    usbInterface->insertDisk("../disks/hd5mbDOS2.dsk");
    
    usbInterface->insertDisk("../disks/dos1.dsk");    
//    usbInterface = new NowindInterface("../msxsrc/nowindDos2.rom");
//    addMemoryDevice(usbInterface, 2, 2, 0);
//    usbInterface->insertDisk("../disks/hd5mbdos2.dsk");    
    
//    usbInterface->insertDisk("../disks/dos2.dsk");    


#endif

#ifdef WD279X
    wd279x = new WD279X("../roms/8250_DISK.ROM");
    addMemoryDevice(wd279x, 3, 1, 0x4000);
    wd279x->insertInitialDisk("../disks/dos1.dsk");
//    wd279x->insertInitialDisk("../disks/UZIX.DSK");
#endif

#ifdef NORMALDISKROM
//    RomBlock *diskRom = new RomBlock("../roms/Diskroms/vy0010.rom");
    RomBlock *diskRom = new RomBlock("../roms/DISK.ROM");
    addMemoryDevice(diskRom, 3, 3, 0x4000);

	diskRom->patch(0x4010,0xed); //DISKIO
	diskRom->patch(0x4011,0x00);
	diskRom->patch(0x4012,0xc9);

	diskRom->patch(0x4013,0xed); //DSKCHG
	diskRom->patch(0x4014,0x00);
	diskRom->patch(0x4015,0xc9);

    // no need to patch GETDPB

	diskRom->patch(0x4019,0xed); //CHOICE
	diskRom->patch(0x401a,0x00);
	diskRom->patch(0x401b,0xc9);

	diskRom->patch(0x401c,0xed); //DSKFMT
	diskRom->patch(0x401d,0x00);
	diskRom->patch(0x401e,0xc9);

	diskRom->patch(0x401f,0); //DRVOFF
	diskRom->patch(0x4020,0);
	diskRom->patch(0x4021,0);
#endif
	
#ifdef MSXDOS2
    addMemoryDevice(new MapperMsxDos2("../roms/MSXDOS22.ROM"),1,0,0x4000);
//    addMemoryDevice(new MapperMsxDos2("../roms/MSXDOS23.ROM"),1,0,0x4000);
#endif    
   		
//    Insert a plain SCC without any megarom
//    addMemoryDevice(SCC::Instance(),1,0,0x9000);
//    addMemoryDevice(new MapperFMPAC("../roms/FMPAC.rom"),1,0,0x4000);
	
    }

/*
	if (Config::Instance()->msxVersion == 3) {
	    // MSX2+
		slot_expanded[0] = false;
		slot_expanded[1] = false;
		slot_expanded[2] = false;
		slot_expanded[3] = true;
		configureSlots();
		loadRomRel("../roms/MSX2P.ROM",0,0,0,2);
		loadRomRel("../roms/MSX2PMUS.ROM",0,2,1,1);
		installMapper(3,0);
		loadRomRel("../roms/MSX2PEXT.ROM",3,1,0,1);
		loadRomRel("../roms/MSXKANJI.ROM",3,1,1,2);
		installDiskRom(3,2,1); // ander diskrom misschien???
		loadRomRel("../roms/XBASIC2.ROM",3,3,1,1);
	}
	//loadRomRel("../roms/FMPAC.ROM",2,0,1,3);    
*/

	if (msxVersion == 99) {
        
		slotExpanded[3] = true;
        addMemoryDevice(new RomBlock("../roms/cbios/cbios_main_msx2.rom"),0,0,0x0000);
		RomBlock *subRom = new RomBlock("../roms/cbios/cbios_sub.rom");
		addMemoryDevice(subRom,3,1,0x0000);

		Z80::Instance()->mapper = new Mapper(32);
		addMemoryDevice(Z80::Instance()->mapper,3,2,0x0000);

//		RomBlock *diskRom = new RomBlock("../roms/cbios/cbios_disk.rom");
//		addMemoryDevice(diskRom,3,3,0x4000);
/*
		diskRom->patch(0x4010,0xed); //DISKIO
		diskRom->patch(0x4011,0x00);
		diskRom->patch(0x4012,0xc9);

		diskRom->patch(0x4013,0xed); //DSKCHG
		diskRom->patch(0x4014,0x00);
		diskRom->patch(0x4015,0xc9);

		diskRom->patch(0x4016,0xed); //GETDPB
		diskRom->patch(0x4017,0x00);
		diskRom->patch(0x4018,0xc9);

		diskRom->patch(0x4019,0xed); //CHOICE
		diskRom->patch(0x401a,0x00);
		diskRom->patch(0x401b,0xc9);

		diskRom->patch(0x401c,0xed); //DSKFMT
		diskRom->patch(0x401d,0x00);
		diskRom->patch(0x401e,0xc9);

		diskRom->patch(0x401f,0xed); // ?
		diskRom->patch(0x4020,0x00);
		diskRom->patch(0x4021,0xc9);
*/
		//addMemoryDevice(new MapperKonami5("../msx games/konami5/Kings Valley2 (msx2).rom"),1,0,0x4000);						  	
		addMemoryDevice(new RomBlock("../../msx games/galaga.rom"),1,0,0x4000);
	}


    /* Activate current pages in slot 0-0 */
    for (unsigned int b=0;b<blockCount;b++) {
        layout[0][0][b](b);
    }
}

void SlotSelector::setMainSlot(nw_byte selection) {

#ifdef STACKTRACK_ON
//		DBERR(" PC = 0x%X set MS: 0x%X\n",reg_pc & 0xffff, value & 255);
#endif

		selectedMainSlot[0] = selection & 0x03;
		selectedMainSlot[1] = (selection >> 2) & 0x03;
		selectedMainSlot[2] = (selection >> 4) & 0x03;
		selectedMainSlot[3] = (selection >> 6);
		updateSelection();
}

void SlotSelector::setSubSlot(nw_byte selection) {

        ffffRegister[selectedMainSlot[3]] = selection;
        
//		selectedSubSlot[0] = selection & 0x03;
//		selectedSubSlot[1] = (selection >> 2) & 0x03;
//		selectedSubSlot[2] = (selection >> 4) & 0x03;
//		selectedSubSlot[3] = (selection >> 6);
		updateSelection();
}

/* exclusively used by the usb-interface and WD279X */
nw_byte SlotSelector::getSelectedMainSlot(Uint8 page) {
		return selectedMainSlot[page];
}

nw_byte SlotSelector::getSelectedSubSlot(Uint8 page) {

        Uint8 ms = selectedMainSlot[page];
        Uint8 ss = (ffffRegister[ms] >> (page*2)) & 3;
		return ss;
}

/* called by readMem(0xffff) from the z80 */
nw_byte SlotSelector::getSubSlotSelection(int slot) {
		return ffffRegister[selectedMainSlot[slot]];
}

void SlotSelector::updateSelection() {
 
        unsigned int page = 0;
        for (unsigned int b=0;b<8;b++) {
            page = b >> 1;
            currentPage[page] = 0;
            //DBERR(" SlotSelector::updateSelection for block " << b << " slot: " << selectedMainSlot[page] << "-" << selectedSubSlot[page] << endl);
            Uint8 ms = selectedMainSlot[page];
            Uint8 ss = (ffffRegister[ms] >> (page*2)) & 3;
            layout[ms][ss][b](b);
        }
}

void SlotSelector::updateSelection(unsigned int block) {

    unsigned int page = block >> 1;
    currentPage[page] = 0;
    Uint8 ms = selectedMainSlot[page];
    Uint8 ss = (ffffRegister[ms] >> (page*2)) & 3;
    layout[ms][ss][block](block);
}

/* 
 * called by readMem(0xffff) from the z80 to found out if reading of 0xFFFF should 
 * return a subSlotSelection or the content of the device in page 3
 */ 
bool SlotSelector::possibleSubslotRead() {

	return slotExpanded[selectedMainSlot[3]];
}

bool SlotSelector::isExpanded(nw_byte mainSlot) {

    return slotExpanded[mainSlot];
}

void SlotSelector::addMemoryDevice(MemoryDevice *mem, Uint8 mainSlot, Uint8 subSlot, nw_word address) {

    DBERR("SlotSelector::addMemoryDevice on slot: %u-%u at address: 0x%04X\n", mainSlot, subSlot, address);
    assert(mem); // the memory-device must exist
    devices.push_back(mem);
    if (slotExpanded[mainSlot]) {
        mem->install(layout, mainSlot, subSlot, address);
    } else {
        for (unsigned int ss=0;ss<4;ss++)
            mem->install(layout, mainSlot, ss, address);
    }
    
}

bool SlotSelector::isIllegalAddress(nw_word address) {
    return currentPage[(address >> 14)] == emptyPage;
}

string SlotSelector::getDeviceName(Uint8 page) {
    MemoryDevice *mem = currentPage[page];
    if (mem != 0) return currentPage[page]->getIdString();
    return string("undefined!");
}
    
SlotSelector::~SlotSelector() {

    list<MemoryDevice *>::iterator i;
    for (i=devices.begin();i!=devices.end();i++) {
        delete *i;
    }
    delete emptyPage;   // TODO: gaat alleen goed als configure is aangeroepen (is dit altijd het geval???)
    DBERR("SlotSelector destruction...\n");
}

void SlotSelector::setPage(Uint8 page, MemoryDevice * mem) {

    currentPage[page] = mem;
}
