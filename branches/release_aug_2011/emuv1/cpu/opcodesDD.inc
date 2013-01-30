//! opcodesDD.inc for IX register
// included in Z80.cc

// TODO: rare constructie met reg_wz in IXPD, maar lijkt wel te werken
#define IXPD (reg_wz = ((REGIX + (signed char)READMEM(reg_pc++)) & 0xffff))
#define REGIXH (REGIX >> 8)
#define REGIXL (REGIX & 255)

// 8-bit load group
case 0x46: TS(19); reg_b = READMEM(IXPD); break;		            // ld b,(ix+d)
case 0x4E: TS(19); reg_c = READMEM(IXPD); break;   		            // ld c,(ix+d)
case 0x56: TS(19); reg_de = LREG(reg_de)|(READMEM(IXPD)<<8); break; // ld d,(ix+d)
case 0x5E: TS(19); reg_de = HREG(reg_de)|READMEM(IXPD); break;      // ld e,(ix+d)
case 0x66: TS(19); reg_hl = LREG(reg_hl)|(READMEM(IXPD)<<8); break; // ld h,(ix+d)
case 0x6E: TS(19); reg_hl = HREG(reg_hl)|READMEM(IXPD); break;      // ld l,(ix+d)
case 0x7E: TS(19); reg_a = READMEM(IXPD); break;                    // ld a,(ix+d)

case 0x70: TS(19); WRITEMEM(IXPD, reg_b); break;        // ld (ix+d),b
case 0x71: TS(19); WRITEMEM(IXPD, reg_c); break;        // ld (ix+d),c
case 0x72: TS(19); WRITEMEM(IXPD, reg_d); break;        // ld (ix+d),d
case 0x73: TS(19); WRITEMEM(IXPD, reg_e); break;        // ld (ix+d),e
case 0x74: TS(19); WRITEMEM(IXPD, reg_h); break;        // ld (ix+d),h
case 0x75: TS(19); WRITEMEM(IXPD, reg_l); break;        // ld (ix+d),l
case 0x77: TS(19); WRITEMEM(IXPD, reg_a); break;        // ld (ix+d),a

case 0x36: TS(19); reg2 = IXPD; WRITEMEM(reg2, READMEM(reg_pc++)); break;   // ld (ix+d),n


// 16-bit load group
case 0x21: TS(14); REGIX = READMEM16(reg_pc); reg_pc += 2; break;       		// ld ix,nn
case 0x22: TS(20); reg_wz = READMEM16(reg_pc); WRITEMEM16(reg_wz++, REGIX); reg_pc += 2; break;	// ld (nn),ix
case 0x2A: TS(20); reg_wz = READMEM16(reg_pc); REGIX = READMEM16(reg_wz++); reg_pc += 2; break;	// ld ix,(nn)
case 0xF9: TS(10); reg_sp = REGIX; break;              				// ld sp,ix

case 0xE1: TS(14); REGIX = READMEM16(reg_sp); reg_sp +=2; break;    // pop ix
case 0xE5: TS(15); reg_sp -= 2; WRITEMEM16(reg_sp, REGIX); break;   // push ix

case 0xE3:      // ex (sp),ix
        
		// TODO: test reg_wz!
		TS(23);
        reg_wz = READMEM16(reg_sp);
        WRITEMEM16(reg_sp, REGIX);
        REGIX = reg_wz;
        break;  


// 8-bit arithmetic group
case 0x86: TS(19); reg2 = READMEM(IXPD); ADDA(reg2); break;		// add a,(ix+d)
case 0x8E: TS(19); reg2 = READMEM(IXPD); ADCA(reg2); break;		// adc a,(ix+d)
case 0x96: TS(19); reg2 = READMEM(IXPD); SUB(reg2); break;		// sub (ix+d)
case 0x9E: TS(19); reg2 = READMEM(IXPD); SBC(reg2); break;		// sbc (ix+d)

case 0xA6: TS(19); reg2 = READMEM(IXPD); AND(reg2); break;      // and (ix+d)
case 0xAE: TS(19); reg2 = READMEM(IXPD); XOR(reg2); break;      // xor (ix+d)
case 0xB6: TS(19); reg2 = READMEM(IXPD); OR(reg2); break;       // or (ix+d)
case 0xBE: TS(19); reg2 = READMEM(IXPD); CP(reg2); break;       // cp (ix+d)

case 0x34: TS(23); reg1 = IXPD; reg2 = READMEM(reg1); INC((++reg2)&255); WRITEMEM(reg1, reg2); break; // inc (ix+d)
case 0x35: TS(23); reg1 = IXPD; reg2 = READMEM(reg1); DEC((--reg2)&255); WRITEMEM(reg1, reg2); break; // dec (ix+d)


// 16-bit arithmetic group

#define ADDIX(r) \
        reg_wz = REGIX + 1; \
		reg1 = REGIX + r; \
        reg_f = (reg_f & (SFLAG|ZFLAG|PFLAG)) | (reg1>>16) | \
                ((reg1>>8)&(YFLAG|XFLAG)) | (((reg1^REGIX^r)>>8)&HFLAG); \
        REGIX = reg1 & 0xFFFF; 

case 0x09: TS(15); ADDIX(reg_bc); break;		// add ix,bc
case 0x19: TS(15); ADDIX(reg_de); break;		// add ix,de
case 0x29: TS(15); ADDIX(REGIX); break;			// add ix,ix
case 0x39: TS(15); ADDIX(reg_sp); break;		// add ix,sp

case 0x23: TS(10); ++REGIX &= 0xFFFF; break;	// inc ix
case 0x2B: TS(10); --REGIX &= 0xFFFF; break;	// dec ix

case 0xE9: 										// jp (ix) 
	TS(8);
    reg_pc = REGIX; 
#ifdef STACKTRACK_ON
    if (Debug::Instance()->STACK_TRACING)
    {
        DBERR(" JP IX -> 0x%04x\n", reg_pc);
    }
#endif
    break;


// CB-opcodes group

case 0xCB: {
        # include "opcodesDDCB.inc"
		}
		break;


// undocumented instructions DD

case 0x44: TS(8); reg_b = REGIXH; break;					// ld b,ixh
case 0x45: TS(8); reg_b = REGIXL; break;					// ld b,ixl
case 0x4C: TS(8); reg_c = REGIXH; break;					// ld c,ixh
case 0x4D: TS(8); reg_c = REGIXL; break;					// ld c,ixl
case 0x54: TS(8); reg_de = LREG(reg_de)|HREG(REGIX); break;	// ld d,ixh
case 0x55: TS(8); reg_de = LREG(reg_de)|(REGIXL<<8); break;	// ld d,ixl
case 0x5C: TS(8); reg_de = HREG(reg_de)|REGIXH; break;		// ld e,ixh
case 0x5D: TS(8); reg_de = HREG(reg_de)|REGIXL; break;		// ld e,ixl
case 0x7C: TS(8); reg_a = REGIXH; break;					// ld a,ixh
case 0x7D: TS(8); reg_a = REGIXL; break;					// ld a,ixl

case 0x60: TS(8); REGIX = LREG(REGIX) | (reg_b << 8); break;       // ld ixh,b
case 0x61: TS(8); REGIX = LREG(REGIX) | (reg_c << 8); break;       // ld ixh,c
case 0x62: TS(8); REGIX = LREG(REGIX) | HREG(reg_de); break;       // ld ixh,d
case 0x63: TS(8); REGIX = LREG(REGIX) | (reg_e << 8); break;       // ld ixh,e
case 0x64: TS(8); break;                                           // ld ixh,ixh
case 0x65: TS(8); REGIX = LREG(REGIX) | (REGIXL << 8); break;      // ld ixh,ixl
case 0x67: TS(8); REGIX = LREG(REGIX) | (reg_a << 8); break;       // ld ixh,a

case 0x68: TS(8); REGIX = HREG(REGIX) | reg_b; break;       // ld ixl,b
case 0x69: TS(8); REGIX = HREG(REGIX) | reg_c; break;       // ld ixl,c
case 0x6A: TS(8); REGIX = HREG(REGIX) | reg_d; break;       // ld ixl,d
case 0x6B: TS(8); REGIX = HREG(REGIX) | reg_e; break;       // ld ixl,e
case 0x6C: TS(8); REGIX = HREG(REGIX) | REGIXH; break;      // ld ixl,ixh
case 0x6D: TS(8); break;                                    // ld ixl,ixl
case 0x6F: TS(8); REGIX = HREG(REGIX) | reg_a; break;       // ld ixl,a

case 0x84: TS(8); reg2 = REGIXH; ADDA(reg2); break;         // add a,ixh
case 0x85: TS(8); reg2 = REGIXL; ADDA(reg2); break;         // add a,ixl
case 0x8C: TS(8); reg2 = REGIXH; ADCA(reg2); break;         // adc a,ixh
case 0x8D: TS(8); reg2 = REGIXL; ADCA(reg2); break;         // adc a,ixl
case 0x94: TS(8); reg2 = REGIXH; SUB(reg2); break;          // sub ixh
case 0x95: TS(8); reg2 = REGIXL; SUB(reg2); break;          // sub ixl
case 0x9C: TS(8); reg2 = REGIXH; SBC(reg2); break;          // sbc ixh
case 0x9D: TS(8); reg2 = REGIXL; SBC(reg2); break;          // sbc ixl

case 0xA4: TS(8); reg2 = REGIXH; AND(reg2); break;          // and ixh
case 0xA5: TS(8); reg2 = REGIXL; AND(reg2); break;          // and ixl
case 0xAC: TS(8); reg2 = REGIXH; XOR(reg2); break;          // xor ixh
case 0xAD: TS(8); reg2 = REGIXL; XOR(reg2); break;          // xor ixl
case 0xB4: TS(8); reg2 = REGIXH; OR(reg2); break;           // or ixh
case 0xB5: TS(8); reg2 = REGIXL; OR(reg2); break;           // or ixl  
case 0xBC: TS(8); reg2 = REGIXH; CP(reg2); break;           // cp ixh
case 0xBD: TS(8); reg2 = REGIXL; CP(reg2); break;           // cp ixl

case 0x24: TS(8); reg1 = MOVLO(REGIX+0x100)&255; INC(reg1); REGIX = LREG(REGIX)|(reg1<<8); break;   // inc ixh
case 0x2C: TS(8); reg1 = LREG(REGIX+1); INC(reg1); REGIX = HREG(REGIX)|reg1; break;                 // inc ixl
case 0x25: TS(8); reg1 = MOVLO(REGIX-0x100)&255; DEC(reg1); REGIX = LREG(REGIX)|(reg1<<8); break;   // dec ixh
case 0x2D: TS(8); reg1 = LREG(REGIX-1); DEC(reg1); REGIX = HREG(REGIX)|reg1; break;                 // dec ixl

case 0x26: TS(8); REGIX = LREG(REGIX) | (READMEM(reg_pc++)<<8); break;     // ld ixh,n
case 0x2E: TS(8); REGIX = HREG(REGIX) | READMEM(reg_pc++); break;          // ld ixl,n

case 0x40: TS(8); break;	// ld b,b

default:
	// Indien de DD xx niet gevonden is, moet de instructie zonder DD worden uitgevoerd
	// (in tegenstelling wat er bij de ED groep gebeurt dus!)
	// TODO: oplossing hiervoor bedenken.
	
	//tijdelijk, want zexall maakt gebruik van DD instructies die geen HL in zich hebben:
	reg_pc--;
	
	//DBERR("laatste opcode: DD %02X\n", opcode);
	//assert(false);
	break;
	
#undef IXPD
#undef REGIXH
#undef REGIXL
