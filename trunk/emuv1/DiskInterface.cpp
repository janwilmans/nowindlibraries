// DiskInterface.cpp

#include "stdio.h"

#include "cpu/Z80.h"
#include "DiskInterface.h"
#include "Debug.h"
#include "osd/OnScreenDisplay.h"

using namespace std;

DiskInterface::DiskInterface() {

	DBERR("DiskInterface constructor...\n");

	z80 = Z80::Instance();
	drive[0] = new DiskDrive();
	drive[1] = new DiskDrive();
//	drive[1]->openDiskImage("disks/hd100f16.dsk");
//	drive[0] = new VirtualDrive(); 
	disk = drive[0];

	DBERR("DiskInterface constructor...finished\n");
}

DiskInterface *DiskInterface::Instance() {

		/* implies singleton class */

		static DiskInterface deInstantie;
		return &deInstantie;
}

DiskInterface::~DiskInterface() {
    // destructor
    DBERR("DiskInterface destructor.\n");
    if (drive[0] != 0) delete(drive[0]);
    if (drive[1] != 0) delete(drive[1]);
    DBERR("DiskInterface destroyed.\n");
}


void DiskInterface::insertDisk(unsigned int diskNumber, string filename) {

    if (diskNumber == 0) {
		DBERR("Disk for drive A: %s\n", filename.c_str());

// TODO: in textbuffer van command.cpp plaatsen
//    	OnScreenDisplay::Instance()->osdMessage("Disk for drive A: " + filename);
    } else {
    	DBERR("Disk for drive B: %s\n", filename.c_str());
//    	OnScreenDisplay::Instance()->osdMessage("Disk for drive B: " + filename);
    }            
    
    assert(diskNumber < 2);
	assert(drive[diskNumber] != 0);

    drive[diskNumber]->openDiskImage(filename);
}

/*! \brief method is called by an added z80-instruction ED 00
 *
 *	Several DiskInterface-io calls in Main-rom are patched with ED 00 C9 so this method 
 *  will be called instead of the real DiskInterface-rom
 * 
 *  (reg_pc-2) indicates which call was actually made:
 * 
 * DiskInterface calls:
 * 
 * 0x4010, DSKIO:  read/write sectors 
 * 0x4013, CHGDSK: change DiskInterface
 * 0x4016, GETDPB: DiskInterface parameter block
 * 0x401C, DRVFMT: DiskInterface format
 * 0x401F, DRVOFF: DiskInterface stop
 * 
 */
void DiskInterface::diskIO() {

    // TODO: support different than 512 sectorsize?
    unsigned int sector;
    unsigned int numberOfSectors;
    
	switch(z80->reg_pc-2) {
	case 0x4010:    //DSKIO
        /*
		A = drive number (relative)
		B = number of sectors
		C = media descriptor (also used for 23 bits sector support)
		DE = start sector
		HL = transfer address
        */
        
        setDrive(z80->reg_a & 1);
        if (!disk->hasDisk()) {
            DBERR("DISKIO: no disk!\n");
            z80->reg_a = 0x02;
            z80->reg_f |= CFLAG;
            return;
        }

        numberOfSectors = z80->reg_b;
                
        // TODO: verify the next two checks on real MSX and remove asserts
        if (numberOfSectors == 0) numberOfSectors = 256;

        if ((z80->reg_hl + (numberOfSectors * 512)) > 0xffff) {
            DBERR("To much sectors read/written!\n");
            z80->reg_a = 0;
            z80->reg_f |= CFLAG;
            assert(false);
            return;
        }            

        sector = z80->reg_de;
        if (z80->reg_c < 0x80) {
            // 23 bits sectornumber for FAT16 support
            sector += (z80->reg_c << 16);            
        }
        
        if (z80->reg_f & CFLAG) {
            // dskio write
            if (disk->isWriteProtected()) {
                z80->reg_a = 0;
                z80->reg_f |= CFLAG;
                return;
            }
            // TODO: should RAM be enabled first?
            writeSectors(sector, numberOfSectors, z80->reg_hl);
                        
        } else {
            // dskio read
            DBERR("DSKIO: read %i sector(s) from sector %i to addr: 0x%02x\n", numberOfSectors, sector, z80->reg_hl);
            readSectors(sector, numberOfSectors, z80->reg_hl);
        }
        z80->reg_f &= ~CFLAG;   // succesfull
        break;

	case 0x4013:    //DSKCHG
        /*
		A = drive number (relative)
		B  = 1st byte of FAT (media descriptor)
        C = media descriptor
		HL = address of DPB
        */
DBERR("DSKCHG_normal\n");
z80->dumpCpuInfo();
        setDrive(z80->reg_a);
        if (!disk->hasDisk()) {
            z80->reg_a = 2;   // not ready
            z80->reg_f |= CFLAG;
            return;
        }
        if (disk->isDiskChanged()) {
            // disk changed
            z80->reg_b = 255;   // TODO: dpb moet in dos1 opnieuw ingelezen worden!
        } else {
            // not changed
            z80->reg_b = 1;
        }
        z80->reg_f &= ~CFLAG;
        break;

	case 0x4016:    
		/* GETDPB
			A  = drive number (relative)
			B  = 1st byte of FAT (media descriptor)
			C  = Media descriptor?
            HL = Base address of DPB 				
		*/
        assert(false);
        break;
        
	case 0x4019:    //CHOICE
        z80->reg_hl = 0;
        break;

	case 0x401C:    //DSKFMT
		z80->reg_f |= CFLAG;
		z80->reg_a = 16;   // disk error
        break;
	default:
        assert(false);
    }
}

void DiskInterface::setDrive(nw_byte register_a) {
            
    switch (register_a) {
    case 0:
        disk = drive[0];
        //DBERR("Drive A: selected"<< endl);
        break;
    case 1:
        disk = drive[1];
        //DBERR("Drive B: selected"<< endl);
        break;
    default:
        DBERR("UNKNOWN drive: %u\n", register_a);
    }       
}    

void DiskInterface::readSectors(unsigned int startSector, unsigned int sectorCount, nw_word destAddress) {
     
    unsigned int dest = destAddress;
    unsigned int sectorSize = disk->getSectorSize();
    unsigned int count = sectorCount*sectorSize;
    
	nw_byte *buffer = new nw_byte[count];
	memset(buffer,0x00,count);                 // fill entire block with 0's
    disk->readSectors(buffer, startSector, sectorCount);
    
    for (unsigned int i=0;i<count;++i) {
    	Z80::Instance()->writeMemPublic(dest++, buffer[i] & 0xff);
    }
    delete [] buffer;     
}

void DiskInterface::writeSectors(unsigned int startSector, unsigned int sectorCount, nw_word sourceAddress) {
     
    unsigned int source = sourceAddress;
    unsigned int sectorSize = disk->getSectorSize();
    unsigned int count = sectorCount*sectorSize;
    
	nw_byte *buffer = new nw_byte[count];
	memset(buffer,0x00,count);                 // fill entire block with 0's

	for (nw_word i=0;i<count;++i) {
		buffer[i] = Z80::Instance()->readMemPublic(source++) & 0xff;
	}
    disk->writeSectors(buffer, startSector, sectorCount);
    delete [] buffer;
}
