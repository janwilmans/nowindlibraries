//! opcodes.inc
// included in Z80.cc

// always include () around r to make precedence explicit
#define HREG(r) ((r) & 0xFF00)
#define LREG(r) ((r) & 0x00FF)
#define MOVHI(r) (((r)&255)<<8)
#define MOVLO(r) ((r)>>8)

// 8-bit load group
case 0x40: TS(4); break;     				      // ld b,b
case 0x41: TS(4); reg_b = reg_c; break;           // ld b,c
case 0x42: TS(4); reg_b = MOVLO(reg_de); break;   // ld b,d
case 0x43: TS(4); reg_b = LREG(reg_de); break;    // ld b,e
case 0x44: TS(4); reg_b = MOVLO(reg_hl); break;   // ld b,h
case 0x45: TS(4); reg_b = LREG(reg_hl); break;    // ld b,l
case 0x47: TS(4); reg_b = reg_a; break;           // ld b,a

case 0x48: TS(4); reg_c = reg_b; break;           // ld c,b
case 0x49: TS(4); break;                          // ld c,c
case 0x4A: TS(4); reg_c = MOVLO(reg_de); break;   // ld c,d
case 0x4B: TS(4); reg_c = LREG(reg_de); break;    // ld c,e
case 0x4C: TS(4); reg_c = MOVLO(reg_hl); break;   // ld c,h
case 0x4D: TS(4); reg_c = LREG(reg_hl); break;    // ld c,l
case 0x4F: TS(4); reg_c = reg_a; break;           // ld c,a

case 0x50: TS(4); reg_de = LREG(reg_de)|(reg_b<<8); break;           // ld d,b
case 0x51: TS(4); reg_de = LREG(reg_de)|(reg_c<<8); break;           // ld d,c
case 0x52: {                                                         // ld d,d 
        // blueMSX debuginfo
        TS(4);
        if (readMem(reg_pc) == 0x18) {
            int len = readMem(reg_pc+1);
            char text[255];
            for (int i=0;i<len;i++) text[i] = readMem(reg_pc+i+2);
            text[len] = 0;
            DBERR("[LDD,D ^ 0x%08X] %s\n", Z80::Instance()->emuTime, text);
        }
    }
    break;
case 0x53: TS(4); reg_de = LREG(reg_de)|MOVHI(reg_de); break;        // ld d,e
case 0x54: TS(4); reg_de = LREG(reg_de)|HREG(reg_hl); break;         // ld d,h
case 0x55: TS(4); reg_de = LREG(reg_de)|MOVHI(reg_hl); break;        // ld d,l
case 0x57: TS(4); reg_de = LREG(reg_de)|(reg_a<<8); break;           // ld d,a

case 0x58: TS(4); reg_de = HREG(reg_de)|reg_b; break;                // ld e,b
case 0x59: TS(4); reg_de = HREG(reg_de)|reg_c; break;                // ld e,c
case 0x5A: TS(4); reg_de = HREG(reg_de)|MOVLO(reg_de); break;        // ld e,d
case 0x5B: TS(4); break;                                             // ld e,e
case 0x5C: TS(4); reg_de = HREG(reg_de)|MOVLO(reg_hl); break;        // ld e,h
case 0x5D: TS(4); reg_de = HREG(reg_de)|LREG(reg_hl); break;         // ld e,l
case 0x5F: TS(4); reg_de = HREG(reg_de)|reg_a; break;                // ld e,a

case 0x60: TS(4); reg_hl = LREG(reg_hl)|(reg_b<<8); break;           // ld h,b
case 0x61: TS(4); reg_hl = LREG(reg_hl)|(reg_c<<8); break;           // ld h,c
case 0x62: TS(4); reg_hl = LREG(reg_hl)|HREG(reg_de); break;         // ld h,d
case 0x63: TS(4); reg_hl = LREG(reg_hl)|MOVHI(reg_de); break;        // ld h,e
case 0x64: TS(4); break;                       						 // ld h,h
case 0x65: TS(4); reg_hl = LREG(reg_hl)|MOVHI(reg_hl); break;        // ld h,l
case 0x67: TS(4); reg_hl = LREG(reg_hl)|(reg_a<<8); break;           // ld h,a

case 0x68: TS(4); reg_hl = HREG(reg_hl)|reg_b; break;                // ld l,b
case 0x69: TS(4); reg_hl = HREG(reg_hl)|reg_c; break;                // ld l,c
case 0x6A: TS(4); reg_hl = HREG(reg_hl)|MOVLO(reg_de); break;        // ld l,d
case 0x6B: TS(4); reg_hl = HREG(reg_hl)|LREG(reg_de); break;         // ld l,e
case 0x6C: TS(4); reg_hl = HREG(reg_hl)|MOVLO(reg_hl); break;        // ld l,h
case 0x6D: TS(4); break;                       						 // ld l,l
case 0x6F: TS(4); reg_hl = HREG(reg_hl)|reg_a; break;                // ld l,a
        
case 0x78: TS(4); reg_a = reg_b; break;               // ld a,b
case 0x79: TS(4); reg_a = reg_c; break;               // ld a,c
case 0x7A: TS(4); reg_a = MOVLO(reg_de); break;       // ld a,d
case 0x7B: TS(4); reg_a = LREG(reg_de); break;        // ld a,e
case 0x7C: TS(4); reg_a = MOVLO(reg_hl); break;       // ld a,h
case 0x7D: TS(4); reg_a = LREG(reg_hl); break;        // ld a,l
case 0x7F: TS(4); break;                              // ld a,a

case 0x06: TS(7); reg_b = READMEM(reg_pc++); break;                    // ld b,n
case 0x0E: TS(7); reg_c = READMEM(reg_pc++); break;                    // ld c,n
case 0x16: TS(7); reg_de = LREG(reg_de)|(READMEM(reg_pc++)<<8); break; // ld d,n
case 0x1E: TS(7); reg_de = HREG(reg_de)|READMEM(reg_pc++); break;      // ld e,n
case 0x26: TS(7); reg_hl = LREG(reg_hl)|(READMEM(reg_pc++)<<8); break; // ld h,n
case 0x2E: TS(7); reg_hl = HREG(reg_hl)|READMEM(reg_pc++); break;      // ld l,n
case 0x3E: TS(14); reg_a = READMEM(reg_pc++); break;           		   // ld a,n

case 0x46: TS(7); reg_b = READMEM(reg_hl); break;                      // ld b,(hl)
case 0x4E: TS(7); reg_c = READMEM(reg_hl); break;                      // ld c,(hl)
case 0x56: TS(7); reg_de = LREG(reg_de)|(READMEM(reg_hl)<<8); break;   // ld d,(hl)
case 0x5E: TS(7); reg_de = HREG(reg_de)|READMEM(reg_hl); break;        // ld e,(hl)
case 0x66: TS(7); reg_hl = LREG(reg_hl)|(READMEM(reg_hl)<<8); break;   // ld h,(hl)
case 0x6E: TS(7); reg_hl = HREG(reg_hl)|READMEM(reg_hl); break;        // ld l,(hl)
case 0x7E: TS(7); reg_a = READMEM(reg_hl); break;                      // ld a,(hl)

case 0x70: TS(7); WRITEMEM(reg_hl, reg_b); break;      // ld (hl),b
case 0x71: TS(7); WRITEMEM(reg_hl, reg_c); break;      // ld (hl),c
case 0x72: TS(7); WRITEMEM(reg_hl, reg_d); break;      // ld (hl),d
case 0x73: TS(7); WRITEMEM(reg_hl, reg_e); break;      // ld (hl),e
case 0x74: TS(7); WRITEMEM(reg_hl, reg_h); break;      // ld (hl),h
case 0x75: TS(7); WRITEMEM(reg_hl, reg_l); break;      // ld (hl),l
case 0x77: TS(7); WRITEMEM(reg_hl, reg_a); break;      // ld (hl),a

case 0x36: TS(10); WRITEMEM(reg_hl, READMEM(reg_pc++)); break;  // ld (hl),n

case 0x0A: TS(7); reg_wz = reg_bc + 1; reg_a = READMEM(reg_bc); break;      // ld a,(bc)
case 0x1A: TS(7); reg_wz = reg_de + 1; reg_a = READMEM(reg_de); break;      // ld a,(de)
case 0x3A:	// ld a,(nn)
		TS(13);
		reg_wz = READMEM16(reg_pc);
		reg_a = READMEM(reg_wz++);
		reg_pc += 2;
		break;

case 0x02: TS(7); reg_wz = reg_a<<8; WRITEMEM(reg_bc, reg_a); break;      					 // ld (bc),a
case 0x12: TS(7); reg_wz = reg_a<<8; WRITEMEM(reg_de, reg_a); break;      					 // ld (de),a
case 0x32: TS(13); reg_wz = reg_a<<8; WRITEMEM(READMEM16(reg_pc), reg_a); reg_pc +=2; break; // ld (nn),a

// 16-bit load group
case 0x01: // ld bc,nn
		TS(10); 
		reg_c = READMEM16(reg_pc);
		reg_b = reg_c >> 8;
		reg_c &= 255;
  		reg_pc += 2;
    	break;
case 0x11: TS(10); reg_de = READMEM16(reg_pc); reg_pc += 2; break;      // ld de,nn
case 0x21: TS(10); reg_hl = READMEM16(reg_pc); reg_pc += 2; break;      // ld hl,nn
case 0x31: TS(10); 
	reg_sp = READMEM16(reg_pc); reg_pc += 2;
	break;      // ld sp,nn     

case 0x2A: TS(16); reg_wz = READMEM16(reg_pc); reg_hl = READMEM16(reg_wz++); reg_pc +=2; break;    // ld hl,(nn)
case 0x22: TS(16); reg_wz = READMEM16(reg_pc); WRITEMEM16(reg_wz++, reg_hl); reg_pc +=2; break;    // ld (nn),hl

case 0xF9: TS(6); reg_sp = reg_hl; break;      // ld sp,hl

case 0xC5: TS(11); reg_sp -= 2; WRITEMEM16(reg_sp, reg_bc); break;      // push bc
case 0xD5: TS(11); reg_sp -= 2; WRITEMEM16(reg_sp, reg_de); break;      // push de
case 0xE5: TS(11); reg_sp -= 2; WRITEMEM16(reg_sp, reg_hl); break;      // push hl
case 0xF5: TS(11); reg_sp -= 2; WRITEMEM16(reg_sp, reg_af); break;      // push af

case 0xC1:  // pop bc
		TS(10);
  		reg_c = READMEM16(reg_sp);
    	reg_b = MOVLO(reg_c);
    	reg_c &= 255;
     	reg_sp +=2;
     	break;
case 0xD1: TS(10); reg_de = READMEM16(reg_sp); reg_sp +=2; break;       // pop de
case 0xE1: TS(10); reg_hl = READMEM16(reg_sp); reg_sp +=2; break;       // pop hl
case 0xF1: // pop af
		TS(10); 
		reg_f = READMEM16(reg_sp);
  		reg_a = MOVLO(reg_f);
    	reg_f &= 255; 
		reg_sp +=2;
  		break;


// exchange, blocktransfer, and search group
case 0xEB: TS(4); reg1 = reg_de; reg_de = reg_hl; reg_hl = reg1; break;            // ex de,hl
case 0x08: // ex af,af'
		TS(4);
  		reg1 = reg_af;
    	reg_a = shadow_af>>8;
    	reg_f = shadow_af&255;
     	shadow_af = reg1;
      	break;
case 0xD9:      // exx
        TS(4); 
        reg1 = reg_b; reg2 = reg_c; 
        reg_b = shadow_b; reg_c = shadow_c; shadow_b = reg1; shadow_c = reg2;
        reg1 = reg_de; reg_de = shadow_de; shadow_de = reg1;
        reg1 = reg_hl; reg_hl = shadow_hl; shadow_hl = reg1;
        break;
case 0xE3:      // ex (sp),hl

		// TODO: test reg_wz!
        TS(19); 
        reg_wz = READMEM16(reg_sp);
        WRITEMEM16(reg_sp, reg_hl);
        reg_hl = reg_wz;
        break;


// 8-bit arithmetic group

/*
// 3830 ms
#define ADDA(r) \
        reg1 = reg_a + r; \
        reg_f = flagSZ[reg1&255] | (reg1>>8) | (reg_a^r^reg1)&HFLAG | (~(reg_a^r)&(reg_a^reg1)&0x80)>>5; \
        reg_a = reg1&255               

*/

// 2980 ms
#define ADDA(r) \
        reg1 = reg_a + r; \
        reg_f = reg_a ^ r; \
		reg_f = (reg1 >> 8) | (reg_f^reg1)&HFLAG | (~(reg_f)&(reg_a^reg1)&0x80)>>5; \
		reg_a = reg1 & 255; \
        reg_f |= flagSZ[reg_a]         


case 0x80: TS(4); ADDA(reg_b); break;               // add a,b
case 0x81: TS(4); ADDA(reg_c); break;               // add a,c
case 0x82: TS(4); reg2 = reg_d; ADDA(reg2); break;  // add a,d
case 0x83: TS(4); reg2 = reg_e; ADDA(reg2); break;  // add a,e
case 0x84: TS(4); reg2 = reg_h; ADDA(reg2); break;  // add a,h
case 0x85: TS(4); reg2 = reg_l; ADDA(reg2); break;  // add a,l
case 0x87: TS(4); ADDA(reg_a); break;               // add a,a

case 0xC6: TS(7); reg2 = READMEM(reg_pc++); ADDA(reg2); break;       // add a,n
case 0x86: TS(7); reg2 = READMEM(reg_hl); ADDA (reg2); break;        // add a,(hl)


#define ADCA(r) \
        reg1 = reg_a + r + (reg_f&CFLAG); \
        reg_f = flagSZ[reg1&255] | (reg1>>8) | (reg_a^r^reg1)&HFLAG | (~(reg_a^r)&(reg_a^reg1)&0x80)>>5; \
        reg_a = reg1&255
/*
#define ADCA(r) \
        reg1 = reg_a + r + (reg_f&CFLAG); \
        reg_f = (reg1 & (SFLAG|XFLAG|YFLAG)) | (reg1>>8) | (reg_a^r^reg1)&HFLAG | (~(reg_a^r)&(reg_a^reg1)&0x80)>>5; \
		reg_a = reg1 & 255; \
		if (reg_a == 0) reg_f |= ZFLAG
*/
		
case 0x88: TS(4); ADCA(reg_b); break;  // adc a,b
case 0x89: TS(4); ADCA(reg_c); break;  // adc a,c
case 0x8A: TS(4); reg2 = reg_d; ADCA(reg2); break;  // adc a,d
case 0x8B: TS(4); reg2 = reg_e; ADCA(reg2); break;  // adc a,e
case 0x8C: TS(4); reg2 = reg_h; ADCA(reg2); break;  // adc a,h
case 0x8D: TS(4); reg2 = reg_l; ADCA(reg2); break;  // adc a,l
case 0x8F: TS(4); ADCA(reg_a); break;  // adc a,a

case 0xCE: TS(7); reg2 = READMEM(reg_pc++); ADCA(reg2); break;       // adc a,n
case 0x8E: TS(7); reg2 = READMEM(reg_hl); ADCA(reg2); break;         // adc a,(hl)


#define SUB(r) \
        reg1 = reg_a - r; \
        reg_f = flagSZsub[reg1&255] | ((reg1>>8)&CFLAG) | (reg_a^r^reg1)&HFLAG | \
                ((reg_a^r)&(reg_a^reg1)&0x80)>>5; \
        reg_a = reg1&255

/*
#define SUB(r) \
        reg1 = reg_a - r; \
        reg_f = (reg1 & (SFLAG|XFLAG|YFLAG)) |((reg1>>8)&CFLAG) | ((reg_a^r^reg1)&HFLAG) | \
                (((reg_a^r)&(reg_a^reg1)&0x80)>>5) | NFLAG; \
        reg_a = reg1&255; \
        if (reg_a == 0) reg_f |= ZFLAG
*/
        
case 0x90: TS(4); SUB(reg_b); break;   // sub b
case 0x91: TS(4); SUB(reg_c); break;   // sub c
case 0x92: TS(4); reg2 = reg_d; SUB(reg2); break;   // sub d
case 0x93: TS(4); reg2 = reg_e; SUB(reg2); break;   // sub e
case 0x94: TS(4); reg2 = reg_h; SUB(reg2); break;   // sub h
case 0x95: TS(4); reg2 = reg_l; SUB(reg2); break;   // sub l
case 0x97: // sub a
		TS(4);
  		reg_a ^= reg_a;
    	reg_f = 0x42;
     	break;

case 0xD6: TS(7); reg2 = READMEM(reg_pc++); SUB(reg2); break;        // sub n
case 0x96: TS(7); reg2 = READMEM(reg_hl); SUB(reg2); break;          // sub (hl)


#define SBC(r) \
        reg1 = reg_a - r - (reg_f&CFLAG); \
        reg_f = flagSZsub[reg1&255] | ((reg1>>8)&CFLAG) | (reg_a^r^reg1)&HFLAG | \
                ((reg_a^r)&(reg_a^reg1)&0x80)>>5; \
        reg_a = reg1&255

/*
#define SBC(r) \
        reg1 = reg_a - r - (reg_f&CFLAG); \
        reg_f = (reg1 & (SFLAG|XFLAG|YFLAG)) | ((reg1>>8)&CFLAG) | ((reg_a^r^reg1)&HFLAG) | \
                (((reg_a^r)&(reg_a^reg1)&0x80)>>5) | NFLAG; \
        reg_a = reg1&255;\
        if (reg_a == 0) reg_f |= ZFLAG
*/

case 0x98: TS(4); SBC(reg_b); break;   // sbc b
case 0x99: TS(4); SBC(reg_c); break;   // sbc c
case 0x9A: TS(4); reg2 = reg_d; SBC(reg2); break;   // sbc d
case 0x9B: TS(4); reg2 = reg_e; SBC(reg2); break;   // sbc e
case 0x9C: TS(4); reg2 = reg_h; SBC(reg2); break;   // sbc h
case 0x9D: TS(4); reg2 = reg_l; SBC(reg2); break;   // sbc l
case 0x9F: // sbc a
		TS(4);	
  		if (reg_f&CFLAG) { reg_a = 255; reg_f = 0xBB; }
		else { reg_a ^= reg_a; reg_f = 0x42; }        
    	break;

case 0xDE: TS(7); reg2 = READMEM(reg_pc++); SBC(reg2); break;        // sbc n
case 0x9E: TS(7); reg2 = READMEM(reg_hl); SBC(reg2); break;          // sbc (hl)


#define AND(r)  reg_a &= r; reg_f = flagSZP[reg_a] | HFLAG
#define OR(r)   reg_a |= r; reg_f = flagSZP[reg_a]
#define XOR(r)  reg_a ^= r; reg_f = flagSZP[reg_a]

case 0xA0: TS(4); AND(reg_b); break;                   // and b
case 0xA1: TS(4); AND(reg_c); break;                   // and c
case 0xA2: TS(4); AND(reg_d); break;                   // and d
case 0xA3: TS(4); AND(reg_e); break;                   // and e
case 0xA4: TS(4); AND(reg_h); break;                   // and h
case 0xA5: TS(4); AND(reg_l); break;                   // and l
case 0xA7: TS(4); reg_f = flagSZP[reg_a]|HFLAG; break; // and a

case 0xE6: TS(7); AND(READMEM(reg_pc++)); break;       // and n
case 0xA6: TS(7); AND(READMEM(reg_hl)); break;         // and (hl)

case 0xA8: TS(4); XOR(reg_b); break;                   // xor b
case 0xA9: TS(4); XOR(reg_c); break;                   // xor c
case 0xAA: TS(4); XOR(reg_d); break;                   // xor d
case 0xAB: TS(4); XOR(reg_e); break;                   // xor e
case 0xAC: TS(4); XOR(reg_h); break;                   // xor h
case 0xAD: TS(4); XOR(reg_l); break;                   // xor l
case 0xAF: TS(4); reg_a = 0x00; reg_f = 0x44; break;   // xor a

case 0xEE: TS(7); XOR(READMEM(reg_pc++)); break;       // xor n
case 0xAE: TS(7); XOR(READMEM(reg_hl)); break;         // xor (hl)

case 0xB0: TS(4); OR(reg_b); break;                    // or b
case 0xB1: TS(4); OR(reg_c); break;                    // or c
case 0xB2: TS(4); OR(reg_d); break;                    // or d
case 0xB3: TS(4); OR(reg_e); break;                    // or e
case 0xB4: TS(4); OR(reg_h); break;                    // or h
case 0xB5: TS(4); OR(reg_l); break;                    // or l
case 0xB7: TS(4); reg_f = flagSZP[reg_a]; break;       // or a

case 0xF6: TS(7); OR(READMEM(reg_pc++)); break;        // or n
case 0xB6: TS(7); OR(READMEM(reg_hl)); break;          // or (hl)


#define CP(r) \
        reg1 = reg_a - r; \
        reg_f = (flagSZsub[reg1&255]&~(YFLAG|XFLAG)) | ((reg1>>8)&CFLAG) | (reg_a^r^reg1)&HFLAG | \
                ((reg_a^r)&(reg_a^reg1)&0x80)>>5 | (r&(YFLAG|XFLAG))

case 0xB8: TS(4); CP(reg_b); break;            						// cp b
case 0xB9: TS(4); CP(reg_c); break;  				          		// cp c
case 0xBA: TS(4); reg2 = reg_d; CP(reg2); break;            		// cp d
case 0xBB: TS(4); reg2 = reg_e; CP(reg2); break;            		// cp e
case 0xBC: TS(4); reg2 = reg_h; CP(reg2); break;            		// cp h
case 0xBD: TS(4); reg2 = reg_l; CP(reg2); break;            		// cp l
case 0xBF: TS(4); reg_f = 0x42 | (reg_a&(YFLAG|XFLAG));	break;		// cp a

case 0xFE: TS(7); reg2 = READMEM(reg_pc++); CP(reg2); break;        // cp n
case 0xBE: TS(7); reg2 = READMEM(reg_hl); CP(reg2); break;          // cp (hl)

#define INC(r) reg_f = flagInc[r] | (reg_f&CFLAG)     
#define DEC(r) reg_f = flagDec[r] | (reg_f&CFLAG)
        
case 0x04: TS(4); ++reg_b &= 255; INC(reg_b); break;                            // inc b
case 0x0C: TS(4); ++reg_c &= 255; INC(reg_c); break;                            // inc c
case 0x14: TS(4); reg_de = (reg_de+0x100)&0xFFFF; INC(reg_d); break;            // inc d
case 0x1C: TS(4); reg_de = HREG(reg_de)|LREG(reg_de+1); INC(reg_e); break;      // inc e
case 0x24: TS(4); reg_hl = (reg_hl+0x100)&0xFFFF; INC(reg_h); break;    	    // inc h
case 0x2C: TS(4); reg_hl = HREG(reg_hl)|LREG(reg_hl+1); INC(reg_l); break;      // inc l
case 0x3C: TS(4); ++reg_a &= 255; INC(reg_a); break;                            // inc a

case 0x34: TS(11); reg1 = READMEM(reg_hl); ++reg1 &= 255; INC(reg1); WRITEMEM(reg_hl, reg1); break;  // inc (hl)

case 0x05: TS(4); --reg_b &= 255; DEC(reg_b); break;                            // dec b
case 0x0D: TS(4); --reg_c &= 255; DEC(reg_c); break;                            // dec c
case 0x15: TS(4); reg_de = LREG(reg_de)|HREG(reg_de-0x100); DEC(reg_d); break;  // dec d
case 0x1D: TS(4); reg_de = HREG(reg_de)|LREG(reg_de-1); DEC(reg_e); break;      // dec e
case 0x25: TS(4); reg_hl = LREG(reg_hl)|HREG(reg_hl-0x100); DEC(reg_h); break;  // dec h
case 0x2D: TS(4); reg_hl = HREG(reg_hl)|LREG(reg_hl-1); DEC(reg_l); break;      // dec l
case 0x3D: TS(4); --reg_a &= 255; DEC(reg_a); break;                            // dec a

case 0x35: TS(11); reg1 = READMEM(reg_hl); --reg1 &= 255; DEC(reg1); WRITEMEM(reg_hl, reg1); break;  // dec (hl)


// general-purpose arithmetic and cpu control groups

case 0x27:                                                              // daa
        TS(4); 
        reg1 = reg_a;
        if (reg_f & NFLAG) {
            reg2 = reg_f & (CFLAG|NFLAG|HFLAG);
            if ((reg_f&CFLAG)||(reg_a>0x99)) reg1 -= 0x160;
            if ((reg_f&HFLAG)||((reg_a&0x0F)>9)) {
                if ((reg_a&0x0F)>5) reg2 &= ~HFLAG;
                reg1 = (reg1&0xFF00)|((reg1-6)&0xFF);
            }
        } else {
            reg2 = (reg_f&CFLAG) | (((reg_a&0x0F)>9)?HFLAG:0);
            if ((reg2|reg_f)&HFLAG) reg1 += 6;
            if ((reg_f&CFLAG) || ((reg1&0x1F0)>0x90)) reg1 += 0x60;
        }
        reg_a = reg1&255;
		reg_f = flagSZP[reg_a] | reg2 | ((reg1>>8)&CFLAG);
		break;

case 0x2F:	// cpl
		TS(4);
		reg_a ^= 255;
		reg_f = (reg_f & 0xc5) | (reg_a & (XFLAG|YFLAG)) | HFLAG | NFLAG;
		break; 
case 0x37:	// scf
		TS(4);
		reg_f = (reg_f&0xc4) | (reg_a & (YFLAG|XFLAG)) | CFLAG;
		break;
case 0x3F:	// ccf
		TS(4);
		reg_f = (reg_f&0xc4) | (reg_a & (YFLAG|XFLAG)) | ((reg_f&CFLAG) ? HFLAG:CFLAG);
		break;

case 0x00: TS(4); break;       // nop
case 0x76:  // halt (TODO: still burns host cpu cycles...)
        TS(4); 
        // DBERR("HALT !\n");
        // Debug::Instance()->memDump(cpuPageRead,0xc000);
        // dumpRegisters();
        // exit(0);
        reg_pc--; // pc incremented at NMI or INT
        break;
        
case 0xF3: // di
        TS(4);
		IFF1 = false;
		IFF2 = false;
		Emulator::Instance()->setCPUInterrupt(false);
		
#ifdef STACKTRACK_ON
        if (Debug::Instance()->STACK_TRACING)
        {
	        DBERR("DI\n");
	    }
#endif
		break;    
case 0xFB: // ei
		TS(4); 
		IFF1 = true;
		IFF2 = true;
			  
		// IFF1 is set immediately, but the next interrupt is scheduled at emuTime + 1
		// This way the interrupt is enabled after the instruction following EI
		Emulator::Instance()->setCPUInterrupt(true);
		   
#ifdef STACKTRACK_ON
        if (Debug::Instance()->STACK_TRACING)
        {
    	    DBERR("EI\n");
    	}
#endif
		break;     

// 16-bit arithmetic group

#define ADDHL(r) \
		reg_wz = reg_hl + 1; \
		reg1 = reg_hl + r; \
        reg_f = (reg_f & (SFLAG|ZFLAG|PFLAG)) | (reg1>>16) \
                | ((reg1>>8)&(YFLAG|XFLAG)) | (((reg1^reg_hl^r)>>8)&HFLAG); \
        reg_hl = reg1 & 0xFFFF
        
case 0x09: TS(11); ADDHL(reg_bc); break;        // add hl,bc
case 0x19: TS(11); ADDHL(reg_de); break;        // add hl,de
case 0x29: TS(11); ADDHL(reg_hl); break;        // add hl,hl
case 0x39: TS(11); ADDHL(reg_sp); break;        // add hl,sp

case 0x03: // inc bc
		TS(6);
  		reg_c |= reg_b<<8;
  		reg_b = (++reg_c>>8)&255;
  		reg_c &= 255;
        break;
case 0x13: TS(6); ++reg_de &= 0xFFFF; break;     // inc de
case 0x23: TS(6); ++reg_hl &= 0xFFFF; break;     // inc hl
case 0x33: TS(6); ++reg_sp &= 0xFFFF; break;     // inc sp

case 0x0B: // dec bc
		TS(6);
  		reg_c |= reg_b<<8;
  		reg_b = (--reg_c>>8)&255;
  		reg_c &= 255; 
    	break;
case 0x1B: TS(6); --reg_de &= 0xFFFF; break;     // dec de
case 0x2B: TS(6); --reg_hl &= 0xFFFF; break;     // dec hl
case 0x3B: TS(6); --reg_sp &= 0xFFFF; break;     // dec sp


// rotate and shift group

case 0x07:      // RLCA
        TS(4); 
        reg_f = (reg_f & (SFLAG|ZFLAG|PFLAG)) | (reg_a>>7);
        reg_a = ((reg_a<<1)&255) | (reg_f&CFLAG);
        reg_f |= reg_a&(YFLAG|XFLAG);
        break;
case 0x17:      // RLA
        TS(4); 
        reg_a = (reg_a<<1) | (reg_f&CFLAG);
        reg_f = (reg_f & (SFLAG|ZFLAG|PFLAG)) | (reg_a>>8) | (reg_a&(YFLAG|XFLAG));
        reg_a &= 255;
        break;
case 0x0F:      // RRCA
        TS(4); 
        reg_f = (reg_f & (SFLAG|ZFLAG|PFLAG)) | (reg_a & CFLAG);
        reg_a = (reg_a>>1) | ((reg_f&CFLAG)<<7);
        reg_f |= reg_a&(YFLAG|XFLAG);
        break;
case 0x1F:      // RRA
        TS(4); 
        reg1 = (reg_a>>1) | (reg_f<<7);
        reg_f = (reg_f & (SFLAG|ZFLAG|PFLAG))| (reg_a & CFLAG) | (reg1&(YFLAG|XFLAG));
        reg_a = reg1 & 255;
        break;


// jump group

#define JPNN    { reg_pc = reg_wz = READMEM16(reg_pc); }
#define JPNEXT  { reg_wz = READMEM16(reg_pc); reg_pc += 2; }

case 0xC3: TS(10); JPNN; break;                                   // jp nn
case 0xC2: TS(10); if (reg_f&ZFLAG) JPNEXT else JPNN break;       // jp nz,nn
case 0xCA: TS(10); if (reg_f&ZFLAG) JPNN else JPNEXT break;       // jp z,nn
case 0xD2: TS(10); if (reg_f&CFLAG) JPNEXT else JPNN break;       // jp cz,nn
case 0xDA: TS(10); if (reg_f&CFLAG) JPNN else JPNEXT break;       // jp c,nn
case 0xE2: TS(10); if (reg_f&PFLAG) JPNEXT else JPNN break;       // jp po,nn
case 0xEA: TS(10); if (reg_f&PFLAG) JPNN else JPNEXT break;       // jp pe,nn
case 0xF2: TS(10); if (reg_f&SFLAG) JPNEXT else JPNN break;       // jp p,nn
case 0xFA: TS(10); if (reg_f&SFLAG) JPNN else JPNEXT break;       // jp m,nn

#define JRE     { TS(12); reg1 = READMEM(reg_pc); reg_pc = reg_wz = (reg_pc + (reg1&127) - (reg1&128) + 1) & 0xffff; }
#define JRNEXT  { TS(7); reg_pc++; }

case 0x18: JRE; break;                                      // jr e
case 0x38: if (reg_f&CFLAG) JRE else JRNEXT break;          // jr c,e
case 0x30: if (reg_f&CFLAG) JRNEXT else JRE break;          // jr nc,e
case 0x28: if (reg_f&ZFLAG) JRE else JRNEXT break;          // jr z,e
case 0x20: if (reg_f&ZFLAG) JRNEXT else JRE break;          // jr nz,e

case 0xE9: // jp (hl)
		TS(4);
		reg_pc = reg_hl; 
#ifdef STACKTRACK_ON
        if (Debug::Instance()->STACK_TRACING)
        {
            DBERR(" JP HL -> 0x%04x\n", reg_pc);
        }
#endif
		break;

case 0x10: // djnz e
    --reg_b &= 255;
    if (reg_b) {
        reg_pc += (signed char)READMEM(reg_pc)+1;
        reg_wz = reg_pc;
        TS(13);
    } else {
        reg_pc++;
        TS(8);
    }
    break;

#ifdef STACKTRACK_ON
    #define CALLNN \
    { \
    TS(17); \
    if (Debug::Instance()->STACK_TRACING) { Debug::Instance()->fill(reg_sp); } \
    reg_sp -= 2; reg_sp &= 0xffff; \
    WRITEMEM16(reg_sp, reg_pc+2); \
    if (Debug::Instance()->STACK_TRACING) { DBERR("0x%08x CALL: from 0x%04x", emuTime, reg_pc-1); } \
    reg_pc = reg_wz = READMEM16(reg_pc); \
    if (Debug::Instance()->STACK_TRACING) { DBERR(" -> 0x%04x\n", reg_pc); } \
    }
#endif

#ifndef STACKTRACK_ON
    #define CALLNN { TS(17); reg_sp -= 2; reg_sp &= 0xffff; WRITEMEM16(reg_sp, reg_pc+2); reg_pc = reg_wz = READMEM16(reg_pc); }
#endif

#define CNEXT { TS(10); reg_wz = READMEM16(reg_pc); reg_pc += 2; }

case 0xCD: CALLNN; break;                            // call nn

case 0xC4: if (reg_f&ZFLAG) CNEXT else CALLNN break; // call nz,nn
case 0xCC: if (reg_f&ZFLAG) CALLNN else CNEXT break; // call z,nn 
case 0xD4: if (reg_f&CFLAG) CNEXT else CALLNN break; // call nc,nn
case 0xDC: if (reg_f&CFLAG) CALLNN else CNEXT break; // call c,nn 
case 0xE4: if (reg_f&PFLAG) CNEXT else CALLNN break; // call po,nn
case 0xEC: if (reg_f&PFLAG) CALLNN else CNEXT break; // call pe,nn
case 0xF4: if (reg_f&SFLAG) CNEXT else CALLNN break; // call p,nn
case 0xFC: if (reg_f&SFLAG) CALLNN else CNEXT break; // call m,nn

#ifdef STACKTRACK_ON
    #define RET reg_pc = reg_wz = READMEM16(reg_sp); reg_sp += 2; reg_sp &= 0xffff; \
	if (Debug::Instance()->STACK_TRACING) { Debug::Instance()->fill(reg_sp); DBERR("RET: to 0x%04x\n", reg_pc); }
#else
    #ifdef FULL_SPEED_ON
        #define RET reg_pc = reg_wz = READMEM16(reg_sp); reg_sp += 2; reg_sp &= 0xffff;
    #else
        // Quit when SP points to an empty slot
        #define RET reg_pc = reg_wz = READMEM16(reg_sp); \
        if (reg_pc == 0xffff) { DBERR("****\nSTACK ERROR\n****\n"); assert(false);} else {reg_sp += 2; reg_sp &= 0xffff;};
    #endif
#endif

case 0xC9: TS(10); RET; break;  // ret

case 0xC0: if (reg_f&ZFLAG) TS(5); else { TS(11); RET; } break;   // ret nz (T=11)	
case 0xD0: if (reg_f&CFLAG) TS(5); else { TS(11); RET; } break;   // ret nc
case 0xE0: if (reg_f&PFLAG) TS(5); else { TS(11); RET; } break;   // ret po
case 0xF0: if (reg_f&SFLAG) TS(5); else { TS(11); RET; } break;   // ret p

case 0xC8: if (reg_f&ZFLAG) { TS(11); RET; } else TS(5); break;   // ret z
case 0xD8: if (reg_f&CFLAG) { TS(11); RET; } else TS(5); break;   // ret c
case 0xE8: if (reg_f&PFLAG) { TS(11); RET; } else TS(5); break;   // ret pe
case 0xF8: if (reg_f&SFLAG) { TS(11); RET; } else TS(5); break;   // ret m

// TODO: reg_sp &= 0xffff; moet op meer plekken 
#define RST(r) reg_wz = 0; reg_sp -= 2; reg_sp &= 0xffff; WRITEMEM16(reg_sp, reg_pc); reg_pc = r

case 0xC7: TS(11); RST(0x0000); break;  // rst 0x00
case 0xCF: TS(11); RST(0x0008); break;  // rst 0x08
case 0xD7: TS(11); RST(0x0010); break;  // rst 0x10
case 0xDF: TS(11); RST(0x0018); break;  // rst 0x18
case 0xE7: TS(11); RST(0x0020); break;  // rst 0x20
case 0xEF: TS(11); RST(0x0028); break;  // rst 0x28
case 0xF7: TS(11); RST(0x0030); break;  // rst 0x30
case 0xFF: TS(11); RST(0x0038); break;  // rst 0x38


// input and output group

case 0xDB: // in a,(n)
		TS(9);
		reg_wz = READMEM(reg_pc++);
		reg_a = readIo(reg_wz);
		reg_wz += (reg_a<<8) + 1;
		TS(2);
		break;
		
case 0xD3: TS(9); writeIo(READMEM(reg_pc++), reg_a); TS(2); break; // out (n),a
