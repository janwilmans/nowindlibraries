//! opcodesED.inc
// included in Z80.cc

// extra eigen instructie voor DISK-IO
case 0x00:
    {
        nw_word reg_pc_was = reg_pc-2;
        DBERR("DISK_ACCESS_SHIT: 0x%04x\n", reg_pc_was);
        if ((reg_pc_was == 0x4010 || reg_pc_was == 0x4013 || reg_pc_was == 0x4016 || reg_pc_was == 0x4019 || reg_pc_was == 0x401c || reg_pc_was == 0x401f)) {
            disk->diskIO();
        }
    }
    break;
    	
case 0x01:
    {
        DBERR("0x%04X: ", reg_hl);
        int len = readMem(reg_pc++);
        for (int i=0;i<len;i++) DBERR("0x%02x ", readMem(reg_hl+i));
        DBERR("\n");
    }
    break;

case 0x02:
    {
        nw_word addr = readMem16(reg_pc);
        DBERR("0x%04X: ", addr);
        reg_pc += 2;
        int len = readMem(reg_pc++);
        for (int i=0;i<len;i++) DBERR("0x%02x ", readMem(addr+i));
        DBERR("\n");
    }
    break;

case 0x05:
	{
		char tmp[]="memdumpx";
		tmp[7] = readMem(reg_pc++) + 48;
		Debug::Instance()->memFileDump(readBlock,tmp);
	}
	break;
	
case 0x06: 
    DBERR("Deprecated, use blueMSX style debug info!\n");
    assert(false);
    break;

case 0x07: 
	DBERR("PC:%04X ", reg_pc-1);
    dumpCpuInfo();
    break;

case 0x08:
    dumpSlotSelection();
    break;

case 0x09:
	Debug::Instance()->memFileDump(readBlock,"memdump1");
	break;

case 0x0a:
	Debug::Instance()->memFileDump(readBlock,"memdump2");
    assert(false);
    DBERR("AAAAASSSSSSSSSSSSEEEEEEEEEEEERRRRRRRRRRRTTTTTTTTTTT\n");
	break;

case 0x0b:
    Debug::Instance()->INSTRUCTION_TRACING = true;
    break;

case 0x0c:
    Debug::Instance()->INSTRUCTION_TRACING = false;
    break;
    
case 0x0d:{
	Uint32 address = reg_de;
	Uint8 count = readMem(reg_pc++);
    char c;
	while (((c = readMem(address)) != 0) || (count == 0)) {
		DBERR("%c", c);
        address++;
        count--;
	}
	DBERR("\n");
    break;
}
case 0x0e:{
	hijackBdos();
    break;
}

// 8-bit load group
case 0x57:      // ld a,i
{
		TS(9);
  		emuTimeType n = Emulator::Instance()->getNextCPUInterruptTime();
  		if (emuTime >= n) {
			IFF2 = false;
		}
        reg_a = reg_i;
        reg_f = flagSZ[reg_a] | (reg_f&CFLAG) | (IFF2 ? PFLAG:0);
}
        break;
case 0x5F:      // ld a,r
{
  		TS(9);
  		emuTimeType n = Emulator::Instance()->getNextCPUInterruptTime();
  		if (emuTime >= n) {
			IFF2 = false;
		}
        reg_a = (reg_r & 0x80) | (refreshCounter & 0x7f);
		reg_f = flagSZ[reg_a] | (reg_f&CFLAG) | (IFF2 ? PFLAG:0);
}
        break;
case 0x47: TS(9); reg_i = reg_a; break;        // ld i,a
case 0x4F: TS(9); reg_r = refreshCounter = reg_a; break;        // ld r,a

// 16-bit load group
case 0x4B: // ld bc,(nn)
		TS(20);
		reg_wz = READMEM16(reg_pc);
  		reg_c = READMEM16(reg_wz++);
    	reg_b = reg_c>>8;
    	reg_c &= 255;
     	reg_pc += 2;
     	break;
case 0x5B: TS(20); reg_wz = READMEM16(reg_pc); reg_de = READMEM16(reg_wz++); reg_pc += 2; break;   // ld de,(nn)
case 0x6B: TS(20); reg_wz = READMEM16(reg_pc); reg_hl = READMEM16(reg_wz++); reg_pc += 2; break;   // ld hl,(nn)

case 0x7B: TS(20); // ld sp,(nn)
	reg_sp = READMEM16(READMEM16(reg_pc)); reg_pc += 2; 
//	DBERR("ld sp,(nn) sp = 0x%04X\n", reg_sp);
	break;   

case 0x43: TS(20); reg_wz = READMEM16(reg_pc); WRITEMEM16(reg_wz++, reg_bc); reg_pc += 2; break;   // ld (nn),bc
case 0x53: TS(20); reg_wz = READMEM16(reg_pc); WRITEMEM16(reg_wz++, reg_de); reg_pc += 2; break;   // ld (nn),de
case 0x63: TS(20); reg_wz = READMEM16(reg_pc); WRITEMEM16(reg_wz++, reg_hl); reg_pc += 2; break;   // ld (nn),hl
case 0x73: TS(20); WRITEMEM16(READMEM16(reg_pc), reg_sp); reg_pc += 2; break;   // ld (nn),sp


// exchange, blocktransfer, and search group

#define BLOCKLD \
		reg_c |= reg_b<<8; 				\
		reg_b = (--reg_c>>8)&255; 		\
		reg1 = READMEM(reg_hl);			\
        WRITEMEM(reg_de, reg1);			\
        reg1 += reg_a;					\
        reg_f = (reg_f & (SFLAG|ZFLAG|CFLAG)) | (reg1&XFLAG) | ((reg1<<4)&YFLAG)

case 0xA0:      // LDI
		TS(16);
        BLOCKLD;
        ++reg_hl &= 0xFFFF;
        ++reg_de &= 0xFFFF;
        reg_f |= (reg_c == 0 ? 0:PFLAG);
   		reg_c &= 255;
        break;
case 0xB0:      // LDIR
		BLOCKLD;
        ++reg_hl &= 0xFFFF;
        ++reg_de &= 0xFFFF;
        if (reg_c == 0) {
			TS(16);
		} else {
            reg_pc -= 2;
            reg_f |= PFLAG;
            reg_c &= 255;
            TS(21);
        }
        break;
case 0xA8:      // LDD
		TS(16);
        BLOCKLD;
        --reg_hl &= 0xFFFF;
        --reg_de &= 0xFFFF;
        reg_f |= (reg_c == 0 ? 0:PFLAG);
   		reg_c &= 255;
        break;
case 0xB8:      // LDDR
        BLOCKLD;
        --reg_hl &= 0xFFFF;
        --reg_de &= 0xFFFF;
        if (reg_c == 0) {
			TS(16);
		} else {
            reg_pc -= 2;
            reg_f |= PFLAG;
            reg_c &= 255;
            TS(21);
        }
        break;

// TODO: kan dit niet eenvoudiger? Geldt ook voor CP misschien.
#define SCPF \
		reg1 = READMEM(reg_hl);						\
		reg_c |= reg_b<<8; 							\
		reg_b = (--reg_c>>8)&255; 					\
        reg2 = reg_a - reg1;						\
        reg_f = (reg2 & SFLAG) | ((reg2 == 0) ? ZFLAG|NFLAG:NFLAG) | (reg_f&CFLAG) \
                | ((reg_a^reg1^reg2)&HFLAG) | (reg_c == 0 ? 0:PFLAG); \
        reg2 -= (reg_f&HFLAG)>>4; 					\
        reg_f |= (reg2&XFLAG) | ((reg2&0x02)<<4);	\
   		reg_c &= 255
   		
case 0xA1:      // CPI
		TS(16);
        SCPF;
        ++reg_hl &= 0xFFFF;
        break;
case 0xB1:      // CPIR
        SCPF; 
        if ((reg1 != reg_a) && (reg_bc != 0)) {
            reg_pc -= 2;
            TS(21);
        } else {
            TS(16);
        }
        ++reg_hl &= 0xFFFF;
        break;
case 0xA9:      // CPD
		TS(16);
        SCPF; 
        --reg_hl &= 0xFFFF;
        break;
case 0xB9:      // CPDR
        SCPF;
        if ((reg1 != reg_a) && (reg_bc != 0)) {
            reg_pc -=2;
            TS(21);
        } else {
            TS(16);
        }
        --reg_hl &= 0xFFFF;
        break;

// general-purpose arithmetic and cpu control groups

case 0x4c:	// neg
case 0x54:
case 0x5c:
case 0x64:
case 0x6c:
case 0x74:
case 0x7c:	//assert(false); // TODO: test maken, daarna kan de assert weg!
case 0x44: 	
		TS(8);
        reg1 = (0 - reg_a) & 255;
        reg_f = flagSZsub[reg1] | (reg_a^reg1)&HFLAG | \
                ((reg_a!=0)?CFLAG:0) | ((reg_a==0x80)?PFLAG:0);
        reg_a = reg1;
        break;
        
case 0x4e:
case 0x66:
case 0x6e: assert(false); // TODO: test maken, daarna kan de assert weg!
case 0x46: TS(8); interruptMode = 0; break;		// IM 0

case 0x76: assert(false); // TODO: test maken, daarna kan de assert weg!
case 0x56: TS(8); interruptMode = 1; break;		// IM 1

case 0x7e: assert(false); // TODO: test maken, daarna kan de assert weg!
case 0x5e: TS(8); interruptMode = 2; break;		// IM 2


// 16-bit arithmetic group

#define ADCHL(r) \
        reg_wz = reg_hl + 1; \
		reg1 = reg_hl + r + (reg_f&CFLAG); \
        reg_f = (reg1>>16) | (((reg_hl^r^reg1)>>8)&HFLAG) | \
                (~(reg_hl^r)&(reg_hl^reg1)&0x8000)>>13 | ((reg1>>8)&(YFLAG|XFLAG)) | \
                ((reg1>>8)&SFLAG) | ((reg1==0)?ZFLAG:0); \
        reg_hl = reg1 & 0xFFFF; 

case 0x4A: TS(15); ADCHL(reg_bc); break;        //adc hl,bc
case 0x5A: TS(15); ADCHL(reg_de); break;        //adc hl,de
case 0x6A: TS(15); ADCHL(reg_hl); break;        //adc hl,hl
case 0x7A: TS(15); ADCHL(reg_sp); break;        //adc hl,sp


#define SBCHL(r) \
        reg_wz = reg_hl + 1; \
        reg1 = reg_hl - r - (reg_f&CFLAG); \
        reg_f = ((reg1>>16)&CFLAG) | NFLAG | (((reg_hl^r^reg1)>>8)&HFLAG) | \
                ((reg_hl^r)&(reg_hl^reg1)&0x8000)>>13 | ((reg1>>8)&(YFLAG|XFLAG)) | \
                ((reg1>>8)&SFLAG) | ((reg1==0)?ZFLAG:0); \
        reg_hl = reg1 & 0xFFFF; \
    
case 0x42: TS(15); SBCHL(reg_bc); break;        //sbc hl,bc
case 0x52: TS(15); SBCHL(reg_de); break;        //sbc hl,de
case 0x62:										//sbc hl,hl
		TS(15);
		if (reg_f & CFLAG) { 
			reg_f = 0xbb; reg_hl = 0xffff;
		} else {
			reg_f = 0x42; reg_hl = 0;
		}
		break;	
case 0x72: TS(15); SBCHL(reg_sp); break;        //sbc hl,sp

// rotate and shift group

case 0x6F:      // RLD

		// TODO: test reg_wz! (reg_wz & 0xffff lijkt me niet nodig)
		TS(18);
        reg1 = (reg_a & 0x0F) | (READMEM(reg_hl)<<4);
        reg_a = (reg_a & 0xF0) | (reg1>>8);
        WRITEMEM(reg_hl, reg1&0xFF);
        reg_f = flagSZP[reg_a]|(reg_f&CFLAG);
        reg_wz = reg_hl + 1;
		break;
case 0x67:      // RRD
		
		// TODO: test reg_wz! (reg_wz & 0xffff lijkt me niet nodig)
		TS(18);
        reg1 = READMEM(reg_hl) | ((reg_a & 0x0F)<<8);
        reg_a = (reg_a & 0xF0) | (reg1 & 0x0F);
        WRITEMEM(reg_hl, (reg1>>4)&0xFF);
        reg_f = flagSZP[reg_a]|(reg_f&CFLAG);
        reg_wz = reg_hl + 1;
        break;


// call and return group
// TODO: sean zeg dat hier ook iff1 = iff2
case 0x4D: TS(14); reg_pc = reg_wz = READMEM16(reg_sp); reg_sp += 2; break;      		// RETI

case 0x55:
case 0x5d:
case 0x65:
case 0x6d:
case 0x75:
case 0x7d: assert(false); // TODO: test maken, daarna kan de assert weg!
case 0x45: TS(14); reg_pc = reg_wz = READMEM16(reg_sp); reg_sp += 2; IFF1 = IFF2; break; // RETN


// input and output group

#define INB(r) reg_f = (reg_f&CFLAG)|flagSZP[r]; reg_wz = reg_bc + 1; TS(2)

case 0x40: TS(10); reg_b = readIo(reg_c); INB(reg_b); break;                          // in b,(c)
case 0x48: TS(10); reg_c = readIo(reg_c); INB(reg_c); break;                          // in c,(c)
case 0x50: TS(10); reg_de = LREG(reg_de)|(readIo(reg_c)<<8);INB(reg_d); break;        // in d,(c)
case 0x58: TS(10); reg_de = HREG(reg_de)|readIo(reg_c);INB(reg_e); break;             // in e,(c)
case 0x60: TS(10); reg_hl = LREG(reg_hl)|(readIo(reg_c)<<8); INB(reg_h); break;       // in h,(c)
case 0x68: TS(10); reg_hl = HREG(reg_hl)|readIo(reg_c); INB(reg_l); break;            // in l,(c)
case 0x70: TS(10); INB(readIo(reg_c)); break; 									      // in (hl),(c)
case 0x78: TS(10); reg_a = readIo(reg_c); INB(reg_a); break;					      // in a,(c)

case 0x41: TS(10); reg_wz = reg_bc + 1; writeIo(reg_c, reg_b); TS(2); break;        // out (c),b
case 0x49: TS(10); reg_wz = reg_bc + 1; writeIo(reg_c, reg_c); TS(2); break;        // out (c),c
case 0x51: TS(10); reg_wz = reg_bc + 1; writeIo(reg_c, reg_d); TS(2); break;        // out (c),d
case 0x59: TS(10); reg_wz = reg_bc + 1; writeIo(reg_c, reg_e); TS(2); break;        // out (c),e
case 0x61: TS(10); reg_wz = reg_bc + 1; writeIo(reg_c, reg_h); TS(2); break;        // out (c),h
case 0x69: TS(10); reg_wz = reg_bc + 1; writeIo(reg_c, reg_l); TS(2); break;        // out (c),l
case 0x71: TS(10); reg_wz = reg_bc + 1; writeIo(reg_c, 0); TS(2); break;            // out (c),0
case 0x79: TS(10); reg_wz = reg_bc + 1; writeIo(reg_c, reg_a); TS(2); break;        // out (c),a

// TODO: alle block I/O functies moeten gechecked worden op het aantal states.
// dus rekening houden met het aantal states voor en na de werkelijke I/O actie 
// en de M1 cycle die door opcodeFetch wordt toegevoegd.

#define BLOCKIN \
		reg2 = readIo(reg_c); \
		WRITEMEM(reg_hl, reg2); \
		--reg_b &= 255

#define BLOCKIN_FLAGS \
        reg_f = flagSZ[reg_b] | ((reg2>>6)&NFLAG) | (flagSZP[(reg1&7)^reg_b]&PFLAG); \
        if (reg1>255) reg_f |= (HFLAG|CFLAG)

#define BLOCK_REPEAT 		\
		if (reg_b) { 		\
			TS(21);			\
			reg_pc -= 2;	\
		} else {			\
			TS(16);			\
		}


case 0xA2:      // INI
#ifdef OUT_INSTRUCTIONS_ON
        DBERR("I: INI\n");
#endif
 		TS(16);
 		BLOCKIN;
        ++reg_hl &= 0xFFFF;
        reg1 = reg2 + ((reg_c+1)&0xFF);
		BLOCKIN_FLAGS;
        break;
case 0xAA:      // IND
#ifdef OUT_INSTRUCTIONS_ON
        DBERR("I: IND\n");
#endif
		TS(16);
		BLOCKIN;
        --reg_hl &= 0xFFFF;
        reg1 = reg2 + ((reg_c-1)&0xFF);
		BLOCKIN_FLAGS;
        break;
case 0xB2:      // INIR
#ifdef OUT_INSTRUCTIONS_ON
        DBERR("I: INIR\n");
#endif
		BLOCKIN;
        ++reg_hl &= 0xFFFF;
        reg1 = reg2 + ((reg_c+1)&0xFF);
		BLOCKIN_FLAGS;
		BLOCK_REPEAT;
        break;
case 0xBA:      // INDR
#ifdef OUT_INSTRUCTIONS_ON
        DBERR("I: INDR\n");
#endif
		BLOCKIN;
        --reg_hl &= 0xFFFF;
        reg1 = reg2 + ((reg_c-1)&0xFF);
		BLOCKIN_FLAGS;
        BLOCK_REPEAT;
        break;


#define BLOCKOUT \
        writeIo(reg_c, reg2); \
        --reg_b &= 255; \
        reg_f = flagSZ[reg_b] | ((reg2>>6)&NFLAG); \
		reg2 += reg_l; \
        if (reg2>255) reg_f |= (HFLAG|CFLAG); \
		reg_f |= flagSZP[(reg2&7)^reg_b]&PFLAG        /* should 'b' of before the decrement be used? */
		                                              /* shouldn't this be: reg_f |= ((reg2&7)^reg_b)&PFLAG 
                                                       *
                                                       * is parity the P-bit or the "real" partity of ((reg2&7)^reg_b) */  
                                                       

case 0xA3:      // OUTI
#ifdef OUT_INSTRUCTIONS_ON
        DBERR("I: OUTI\n");
#endif
		TS(16);
        reg2 = READMEM(reg_hl);
        ++reg_hl &= 0xFFFF;
        BLOCKOUT;
        break;
case 0xB3:      // OTIR
#ifdef OUT_INSTRUCTIONS_ON
        DBERR("I: OTIR\n");
#endif
        reg2 = READMEM(reg_hl);
        ++reg_hl &= 0xFFFF;
        BLOCKOUT;
        BLOCK_REPEAT;
        break;
case 0xAB:      // OUTD
#ifdef OUT_INSTRUCTIONS_ON
        DBERR("I: OUTD\n");
#endif
        TS(16);
        reg2 = READMEM(reg_hl);
        --reg_hl &= 0xFFFF;
        BLOCKOUT;
        break;
case 0xBB:      // OTDR
#ifdef OUT_INSTRUCTIONS_ON
        DBERR("I: OTDR\n");
#endif
        reg2 = READMEM(reg_hl);
        --reg_hl &= 0xFFFF;
        BLOCKOUT;
        BLOCK_REPEAT;
        break;

// undocumented instructions (nop)
		
case 0x77: case 0x7f:
	
case 0xa4: case 0xb4:
case 0xa5: case 0xb5:
case 0xa6: case 0xb6:
case 0xa7: case 0xb7:

case 0xac: case 0xbc:
case 0xad: case 0xbd:
case 0xae: case 0xbe:
case 0xaf: case 0xbf: break;

// TODO: wat gebeurt er met opcode: $ed $ed, waarschijnlijk NOP
case 0xed: assert(false); break;

default:
	if ((opcode >= 0x80) && (opcode < 0xa0)) break;
	opcode &= 0xc0;
	if ((opcode == 0x00) || (opcode == 0xc0)) break;
	assert(false); // trap opcodes that are forgotten or > 255
	break;
