/* 
 * EmulatorTester.cpp: Emulation test class.
 *
 * Copyright (C) 2004 Jan Wilmans <jw@dds.nl>
 *                    Aaldert Dekker <a.dekker@student.tue.nl>
 */
#include "cpu/Z80.h"
#include "Debug.h"
#include "video/V9938.h"

using namespace std;

EmulatorTester * EmulatorTester::Instance() {

		/* implies singleton class */
		static EmulatorTester deInstantie;
		return &deInstantie;
}

void EmulatorTester::start() {    

	cpu = Z80::Instance();
	vdp = V9938::Instance();

    z80Tests();
    vdpTests();
}

void EmulatorTester::z80Tests() {    

// A*100h  LD (xx),A               ;xx=BC,DE,nn
	
	initTest();
    DBERR("reg_wz: LD (bc),a\n");
    cpu->reg_a = 255;
    cpu->writeMemPublic(0xc000,0x02);
	cpu->executeInstructions();
	DBERR("reg_a: 0x%02X\n", cpu->reg_a);
	DBERR("reg_pc: 0x%02X\n", cpu->reg_pc);
	DBERR("reg_wz: 0x%02X\n", cpu->reg_wz);
	assert(cpu->reg_wz == 0xff00);
	
	initTest();
    DBERR("reg_wz: LD (de),a\n");
    cpu->reg_a = 0xaa;
    cpu->writeMemPublic(0xc000, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xaa00);

    initTest();
    DBERR("reg_wz: LD (nn),a\n");
    cpu->reg_a = 0x28;
    cpu->writeMemPublic(0xc000, 0x32);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x2800);


// xx+1    LD A,(xx)               ;xx=BC,DE,nn

    initTest();
    DBERR("reg_wz: LD a,(bc)\n");
    cpu->reg_b = 0xdf;
    cpu->reg_c = 0xff;
    cpu->writeMemPublic(0xc000, 0x0a);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xe000);

    initTest();
    DBERR("reg_wz: LD a,(de)\n");
    cpu->reg_de = 0xefff;
    cpu->writeMemPublic(0xc000, 0x1a);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xf000);

    initTest();
    DBERR("reg_wz: LD a,(nn)\n");
    cpu->writeMemPublic(0xc000, 0x3a);
    cpu->writeMemPublic(0xc001, 0x34);
    cpu->writeMemPublic(0xc002, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);


// nn+1    LD (nn),rr; LD rr,(nn)  ;rr=BC,DE,HL,IX,IY

    initTest();
    DBERR("reg_wz: LD (nn),bc\n");
    cpu->writeMemPublic(0xc000, 0xed);
    cpu->writeMemPublic(0xc001, 0x43);
    cpu->writeMemPublic(0xc002, 0x34);
    cpu->writeMemPublic(0xc003, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);
    // todo: check op echte z80: ld (nn),sp  en ld sp,(nn)

    initTest();
    DBERR("reg_wz: LD (nn),de\n");
    cpu->writeMemPublic(0xc000, 0xed);
    cpu->writeMemPublic(0xc001, 0x53);
    cpu->writeMemPublic(0xc002, 0x34);
    cpu->writeMemPublic(0xc003, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);

    initTest();
    DBERR("reg_wz: LD (nn),hl\n");
    cpu->writeMemPublic(0xc000, 0xed);
    cpu->writeMemPublic(0xc001, 0x63);
    cpu->writeMemPublic(0xc002, 0x34);
    cpu->writeMemPublic(0xc003, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);

    initTest();
    DBERR("reg_wz: LD (nn),ix\n");
    cpu->writeMemPublic(0xc000, 0xdd);
    cpu->writeMemPublic(0xc001, 0x22);
    cpu->writeMemPublic(0xc002, 0x34);
    cpu->writeMemPublic(0xc003, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);

    initTest();
    DBERR("reg_wz: LD bc,(nn)\n");
    cpu->writeMemPublic(0xc000, 0xed);
    cpu->writeMemPublic(0xc001, 0x4b);
    cpu->writeMemPublic(0xc002, 0x34);
    cpu->writeMemPublic(0xc003, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);
    // todo: check op echte z80: ld (nn),sp  en ld sp,(nn)

    initTest();
    DBERR("reg_wz: LD de,(nn)\n");
    cpu->writeMemPublic(0xc000, 0xed);
    cpu->writeMemPublic(0xc001, 0x5b);
    cpu->writeMemPublic(0xc002, 0x34);
    cpu->writeMemPublic(0xc003, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);

    initTest();
    DBERR("reg_wz: LD hl,(nn)\n");
    cpu->writeMemPublic(0xc000, 0xed);
    cpu->writeMemPublic(0xc001, 0x6b);
    cpu->writeMemPublic(0xc002, 0x34);
    cpu->writeMemPublic(0xc003, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);

    initTest();
    DBERR("reg_wz: LD iy,(nn)\n");
    cpu->writeMemPublic(0xc000, 0xfd);
    cpu->writeMemPublic(0xc001, 0x2a);
    cpu->writeMemPublic(0xc002, 0x34);
    cpu->writeMemPublic(0xc003, 0x12);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0x1235);


// rr      EX (SP),rr              ;rr=HL,IX,IY (MEMPTR=new value of rr)

    initTest();
    DBERR("reg_wz: EX (sp),hl\n");
    cpu->reg_sp = 0xc000;
    cpu->reg_pc = 0xc002;
    cpu->reg_hl = 0x7654;
    cpu->writeMemPublic(0xc000, 0xcd);
    cpu->writeMemPublic(0xc001, 0xab);
    cpu->writeMemPublic(0xc002, 0xe3);
    cpu->executeInstructions();
    assert(cpu->reg_hl == 0xabcd);
    assert(cpu->reg_wz == 0xabcd);

    initTest();
    DBERR("reg_wz: EX (sp),ix\n");
    cpu->reg_sp = 0xc000;
    cpu->reg_pc = 0xc002;
    cpu->reg_ix = 0x7654;
    cpu->writeMemPublic(0xc000, 0xcd);
    cpu->writeMemPublic(0xc001, 0xab);
    cpu->writeMemPublic(0xc002, 0xdd);
    cpu->writeMemPublic(0xc003, 0xe3);
    cpu->executeInstructions();
    assert(cpu->reg_ix == 0xabcd);
    assert(cpu->reg_wz == 0xabcd);


// rr+1    ADD/ADC/SBC rr,xx       ;rr=HL,IX,IY (MEMPTR=old value of rr+1)

    initTest();
    DBERR("reg_wz: ADD hl,bc\n");
    cpu->reg_hl = 0xabcd;
    cpu->reg_b = 0x00;
    cpu->reg_c = 0x12;
    cpu->writeMemPublic(0xc000, 0x09);
    cpu->executeInstructions();
    assert(cpu->reg_wz == (0xabcd+1));
    assert(cpu->reg_hl == (0xabcd+0x12));

    initTest();
    DBERR("reg_wz: ADD ix,bc\n");
    cpu->reg_ix = 0xabcd;
    cpu->reg_b = 0x12;
    cpu->reg_c = 0x34;
    cpu->writeMemPublic(0xc000, 0xdd);
    cpu->writeMemPublic(0xc001, 0x09);
    cpu->executeInstructions();
    assert(cpu->reg_wz == (0xabcd+1));
    assert(cpu->reg_ix == (0xabcd+0x1234));

    initTest();
    DBERR("reg_wz: ADD iy,iy\n");
    cpu->reg_iy = 0xabcd;
    cpu->writeMemPublic(0xc000, 0xfd);
    cpu->writeMemPublic(0xc001, 0x29);
    cpu->executeInstructions();
    assert(cpu->reg_wz == (0xabcd+1));
    assert(cpu->reg_iy == ((0xabcd + 0xabcd) & 0xffff));

    initTest();
    DBERR("reg_wz: ADC hl,sp\n");
    cpu->reg_hl = 0xabcd;
    cpu->reg_sp = 0x4321;
    cpu->writeMemPublic(0xc000, 0xed);
    cpu->writeMemPublic(0xc001, 0x7a);
    cpu->executeInstructions();
    assert(cpu->reg_wz == (0xabcd+1));
    assert(cpu->reg_hl == (0xabcd+0x4321));

    initTest();
    DBERR("reg_wz: SBC hl,de\n");
    cpu->reg_hl = 0xabcd;
    cpu->reg_de = 0x4321;
    cpu->writeMemPublic(0xc000, 0xed);
    cpu->writeMemPublic(0xc001, 0x52);
    cpu->executeInstructions();
    assert(cpu->reg_wz == (0xabcd+1));
    assert(cpu->reg_hl == (0xabcd - 0x4321));

//  dest    JP nn; CALL nn; JR nn   ;dest=nn

    initTest();
    DBERR("reg_wz: JP\n");
    cpu->writeMemPublic(0xc000, 0xc3);
    cpu->writeMemPublic(0xc001, 0x08);
    cpu->writeMemPublic(0xc002, 0xc0);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xc008);

    initTest();
    DBERR("reg_wz: JR\n");
    cpu->writeMemPublic(0xc000, 0x18);
    cpu->writeMemPublic(0xc001, 4);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xc006);

    initTest();
    DBERR("reg_wz: CALL\n");
    cpu->writeMemPublic(0xc000, 0xcd);
    cpu->writeMemPublic(0xc001, 0x07);
    cpu->writeMemPublic(0xc002, 0xc0);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xc007);


//  dest    JP f,nn; CALL f,nn      ;regardless of condition true/false

    initTest();
    DBERR("reg_wz: JP C,xxxx\n");
    cpu->writeMemPublic(0xc000, 0xda);
    cpu->writeMemPublic(0xc001, 0x08);
    cpu->writeMemPublic(0xc002, 0xc0);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xc008);

    initTest();
    DBERR("reg_wz: JP NC,xxxx\n");
    cpu->writeMemPublic(0xc000, 0xd2);
    cpu->writeMemPublic(0xc001, 0x08);
    cpu->writeMemPublic(0xc002, 0xc0);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xc008);

    initTest();
    DBERR("reg_wz: CALL Z,xxxx\n");
    cpu->writeMemPublic(0xc000, 0xcc);
    cpu->writeMemPublic(0xc001, 0x07);
    cpu->writeMemPublic(0xc002, 0xc0);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xc007);

    initTest();
    DBERR("reg_wz: CALL NZ,xxxx\n");
    cpu->writeMemPublic(0xc000, 0xc4);
    cpu->writeMemPublic(0xc001, 0x07);
    cpu->writeMemPublic(0xc002, 0xc0);
    cpu->executeInstructions();
    assert(cpu->reg_wz == 0xc007);


//

    DBERR("reg_wz: 0x%02X\n", cpu->reg_wz);
    DBERR("eind Test\n");	
	cpu->reset();
}

void EmulatorTester::initTest() {

	cpu->writeIo(0xa8, 255);
	cpu->writeMemPublic(0xffff, 0xaa);

	cpu->reg_a = 0; 	cpu->reg_f = 0;
	cpu->reg_b = 0; 	cpu->reg_c = 0;
	cpu->reg_hl = 0; 	cpu->reg_de = 0;
	cpu->reg_ix = 0; 	cpu->reg_iy = 0;
	cpu->reg_sp = 0xf000; 
    cpu->reg_pc = 0xc000;
    
	for (int i=0;i<16;i++) cpu->writeMemPublic(0xc000 + i, 0);
    cpu->writeMemPublic(0xc010, 0xed);
    cpu->writeMemPublic(0xc011, 0x03);
    cpu->emuTime = 0;
    cpu->nextInterrupt = 1000000;
}

void EmulatorTester::vdpTests() {    

    nw_byte dummy = 0;

    vdp->port1DataLatched = true;
    vdp->port2DataLatched = true;
    dummy = vdp->readVram();                    // result untested (but msx would not do much AT ALL if this failed)
    assert(vdp->port1DataLatched == false);     // should reset
    assert(vdp->port2DataLatched == true);      // should not change

    vdp->port1DataLatched = true;
    vdp->port2DataLatched = true;
    vdp->writeVram(0);                          // result untested (but msx would not do much AT ALL if this failed)
    assert(vdp->port1DataLatched == false);     // should reset
    assert(vdp->port2DataLatched == true);      // should not change
    
    vdp->port1DataLatched = true;
    dummy = vdp->readStatusRegister();
    assert(vdp->port1DataLatched == false);     // should reset

    vdp->port1DataLatched = false;
    vdp->dataLatch = 1;
    vdp->writeControlRegister(0);
    assert(vdp->port1DataLatched == true);      // should invert
    assert(vdp->dataLatch == 0);                // should contains the latched '0'
     
    vdp->vdpReg[23] = 170;
    vdp->dataLatch = 0;
    vdp->writeControlRegister(0x80+23);
    assert(vdp->port1DataLatched == false);     // should invert
    assert(vdp->dataLatch == 0);                // should be unchanged
    assert(vdp->vdpReg[23] == 0);               // should be '0'

    vdp->port2DataLatched = false;
    vdp->dataLatch = 1;
    vdp->writePaletteRegister(0);
    assert(vdp->port2DataLatched == true);      // should invert
    assert(vdp->dataLatch == 0);                // should contains the latched '0'

    vdp->vdpReg[16] = 15;
    vdp->dataLatch = 2;
    vdp->writePaletteRegister(0);
    assert(vdp->port2DataLatched == false);     // should invert
    assert(vdp->dataLatch == 2);                // should be unchanged
    assert(vdp->vdpReg[16] == 0);               // should be back to '0'
    
    vdp->dataLatch = 2;
    vdp->vdpReg[23] = 4;
    vdp->vdpReg[17] = 23;
    vdp->writeRegisterIndirect(0);
    assert(vdp->dataLatch == 0);                // should be '0'
    assert(vdp->vdpReg[23] == 0);               // should be '0'

    vdp->vdpReg[17] = 0x80+23;
    vdp->writeRegisterIndirect(0);
    assert(vdp->vdpReg[17] == 0x80+23);         // reg17 should not be incremented
    vdp->vdpReg[17] = 23;
    vdp->writeRegisterIndirect(0);
    assert(vdp->vdpReg[17] == 24);               // reg17 SHOULD be incremented
}
