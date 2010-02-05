//! opcodesCB.inc
// included in Z80.cc

#define RLC(r) reg_f = r>>7; r = ((r<<1)|reg_f)&255; reg_f |= flagSZP[r]
#define RRC(r) reg_f = r&CFLAG; r = (r>>1)|(reg_f<<7); reg_f |= flagSZP[r]
#define RL(r) reg1 = ((r<<1)|(reg_f&CFLAG))&255; reg_f = (r>>7)|flagSZP[reg1]; r = reg1
#define RR(r) reg1 = ((r>>1)|(reg_f<<7))&255; reg_f = (r&CFLAG)|flagSZP[reg1]; r = reg1

case 0x00: TS(8); RLC(reg_b); break;   // rlc b
case 0x01: TS(8); RLC(reg_c); break;   // rlc c
case 0x02: TS(8); reg2 = reg_d; RLC(reg2); reg_de = LREG(reg_de)|(reg2<<8); break;   // rlc d
case 0x03: TS(8); reg2 = reg_e; RLC(reg2); reg_de = HREG(reg_de)|reg2; break;        // rlc e
case 0x04: TS(8); reg2 = reg_h; RLC(reg2); reg_hl = LREG(reg_hl)|(reg2<<8); break;   // rlc h
case 0x05: TS(8); reg2 = reg_l; RLC(reg2); reg_hl = HREG(reg_hl)|reg2; break;        // rlc l
case 0x06: TS(15); reg2 = READMEM(reg_hl); RLC(reg2); WRITEMEM(reg_hl, reg2); break; // rlc (hl)
case 0x07: TS(8); RLC(reg_a); break;   // rlc a

case 0x08: TS(8); RRC(reg_b); break;   // rrc b
case 0x09: TS(8); RRC(reg_c); break;   // rrc c
case 0x0A: TS(8); reg2 = reg_d; RRC(reg2); reg_de = LREG(reg_de)|(reg2<<8); break;   // rrc d
case 0x0B: TS(8); reg2 = reg_e; RRC(reg2); reg_de = HREG(reg_de)|reg2; break;        // rrc e
case 0x0C: TS(8); reg2 = reg_h; RRC(reg2); reg_hl = LREG(reg_hl)|(reg2<<8); break;   // rrc h
case 0x0D: TS(8); reg2 = reg_l; RRC(reg2); reg_hl = HREG(reg_hl)|reg2; break;        // rrc l
case 0x0E: TS(15); reg2 = READMEM(reg_hl); RRC(reg2); WRITEMEM(reg_hl, reg2); break; // rrc (hl)
case 0x0F: TS(8); RRC(reg_a); break;   // rrc a
        
case 0x10: TS(8); RL(reg_b); break;    // rl b
case 0x11: TS(8); RL(reg_c); break;    // rl c
case 0x12: TS(8); reg2 = reg_d; RL(reg2); reg_de = LREG(reg_de)|(reg2<<8); break;   // rl d
case 0x13: TS(8); reg2 = reg_e; RL(reg2); reg_de = HREG(reg_de)|reg2; break;        // rl e
case 0x14: TS(8); reg2 = reg_h; RL(reg2); reg_hl = LREG(reg_hl)|(reg2<<8); break;   // rl h
case 0x15: TS(8); reg2 = reg_l; RL(reg2); reg_hl = HREG(reg_hl)|reg2; break;        // rl l
case 0x16: TS(15); reg2 = READMEM(reg_hl); RL(reg2); WRITEMEM(reg_hl, reg2); break; // rl (hl)
case 0x17: TS(8); RL(reg_a); break;    // rl a

case 0x18: TS(8); RR(reg_b); break;    // rr b
case 0x19: TS(8); RR(reg_c); break;    // rr c
case 0x1A: TS(8); reg2 = reg_d; RR(reg2); reg_de = LREG(reg_de)|(reg2<<8); break;   // rr d
case 0x1B: TS(8); reg2 = reg_e; RR(reg2); reg_de = HREG(reg_de)|reg2; break;        // rr e
case 0x1C: TS(8); reg2 = reg_h; RR(reg2); reg_hl = LREG(reg_hl)|(reg2<<8); break;   // rr h
case 0x1D: TS(8); reg2 = reg_l; RR(reg2); reg_hl = HREG(reg_hl)|reg2; break;        // rr l
case 0x1E: TS(15); reg2 = READMEM(reg_hl); RR(reg2); WRITEMEM(reg_hl, reg2); break; // rr (hl)
case 0x1F: TS(8); RR(reg_a); break;    // rr a


#define SLA(r) reg_f = (r>>7); r = (r<<1)&255; reg_f |= flagSZP[r]
#define SRA(r) reg1 = r&CFLAG; r = (r>>1)|(r&0x80); reg_f = reg1|flagSZP[r]
#define SRL(r) reg_f = (r&CFLAG)|flagSZP[r>>1]; r = r>>1
#define SLL(r) reg_f = r>>7; r = ((r<<1)|0x01)&255; reg_f |= flagSZP[r]

case 0x20: TS(8); SLA(reg_b); break;   // sla b
case 0x21: TS(8); SLA(reg_c); break;   // sla c
case 0x22: TS(8); reg2 = reg_d; SLA(reg2); reg_de = LREG(reg_de)|(reg2<<8); break;   // sla d
case 0x23: TS(8); reg2 = reg_e; SLA(reg2); reg_de = HREG(reg_de)|reg2; break;        // sla e
case 0x24: TS(8); reg2 = reg_h; SLA(reg2); reg_hl = LREG(reg_hl)|(reg2<<8); break;   // sla h
case 0x25: TS(8); reg2 = reg_l; SLA(reg2); reg_hl = HREG(reg_hl)|reg2; break;        // sla l
case 0x26: TS(15); reg2 = READMEM(reg_hl); SLA(reg2); WRITEMEM(reg_hl, reg2); break; // sla (hl) 
case 0x27: TS(8); SLA(reg_a); break;   // sla a

case 0x28: TS(8); SRA(reg_b); break;   // sra b
case 0x29: TS(8); SRA(reg_c); break;   // sra c
case 0x2A: TS(8); reg2 = reg_d; SRA(reg2); reg_de = LREG(reg_de)|(reg2<<8); break;   // sra d
case 0x2B: TS(8); reg2 = reg_e; SRA(reg2); reg_de = HREG(reg_de)|reg2; break;        // sra e
case 0x2C: TS(8); reg2 = reg_h; SRA(reg2); reg_hl = LREG(reg_hl)|(reg2<<8); break;   // sra h
case 0x2D: TS(8); reg2 = reg_l; SRA(reg2); reg_hl = HREG(reg_hl)|reg2; break;        // sra l
case 0x2E: TS(15); reg2 = READMEM(reg_hl); SRA(reg2); WRITEMEM(reg_hl, reg2); break; // sra (hl)
case 0x2F: TS(8); SRA(reg_a); break;   // sra a

// SLL is undocumented (different behaviour on R800?)
case 0x30: TS(8); SLL(reg_b); break;   // sll b
case 0x31: TS(8); SLL(reg_c); break;   // sll c
case 0x32: TS(8); reg2 = reg_d; SLL(reg2); reg_de = LREG(reg_de)|(reg2<<8); break;   // sll d
case 0x33: TS(8); reg2 = reg_e; SLL(reg2); reg_de = HREG(reg_de)|reg2; break;        // sll e
case 0x34: TS(8); reg2 = reg_h; SLL(reg2); reg_hl = LREG(reg_hl)|(reg2<<8); break;   // sll h
case 0x35: TS(8); reg2 = reg_l; SLL(reg2); reg_hl = HREG(reg_hl)|reg2; break;        // sll l
case 0x36: TS(15); reg2 = READMEM(reg_hl); SLL(reg2); WRITEMEM(reg_hl, reg2); break; // sll (hl)
case 0x37: TS(8); SLL(reg_a); break;   // sll a

case 0x38: TS(8); SRL(reg_b); break;   // srl b
case 0x39: TS(8); SRL(reg_c); break;   // srl c
case 0x3A: TS(8); reg2 = reg_d; SRL(reg2); reg_de = LREG(reg_de)|(reg2<<8); break;   // srl d
case 0x3B: TS(8); reg2 = reg_e; SRL(reg2); reg_de = HREG(reg_de)|reg2; break;        // srl e
case 0x3C: TS(8); reg2 = reg_h; SRL(reg2); reg_hl = LREG(reg_hl)|(reg2<<8); break;   // srl h
case 0x3D: TS(8); reg2 = reg_l; SRL(reg2); reg_hl = HREG(reg_hl)|reg2; break;        // srl l
case 0x3E: TS(15); reg2 = READMEM(reg_hl); SRL(reg2); WRITEMEM(reg_hl, reg2); break; // srl (hl)
case 0x3F: TS(8); SRL(reg_a); break;   // srl a

// Checked on real Z80
#define BIT0(r) reg_f = (reg_f&CFLAG) | (r & 0x28) | ((r&0x01) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BIT1(r) reg_f = (reg_f&CFLAG) | (r & 0x28) | ((r&0x02) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BIT2(r) reg_f = (reg_f&CFLAG) | (r & 0x28) | ((r&0x04) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BIT3(r) reg_f = (reg_f&CFLAG) | (r & 0x28) | ((r&0x08) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BIT4(r) reg_f = (reg_f&CFLAG) | (r & 0x28) | ((r&0x10) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BIT5(r) reg_f = (reg_f&CFLAG) | (r & 0x28) | ((r&0x20) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BIT6(r) reg_f = (reg_f&CFLAG) | (r & 0x28) | ((r&0x40) ? HFLAG:PFLAG|ZFLAG|HFLAG)                
#define BIT7(r) reg_f = (reg_f&CFLAG) | (r & (SFLAG|0x28)) | ((r&0x80) ? HFLAG:PFLAG|ZFLAG|HFLAG)
     
// TODO: check reg_wz!
#define BITHL0(r) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((r&0x01) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BITHL1(r) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((r&0x02) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BITHL2(r) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((r&0x04) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BITHL3(r) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((r&0x08) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BITHL4(r) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((r&0x10) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BITHL5(r) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((r&0x20) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BITHL6(r) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((r&0x40) ? HFLAG:PFLAG|ZFLAG|HFLAG)
#define BITHL7(r) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((r&0x80) ? SFLAG|HFLAG:PFLAG|ZFLAG|HFLAG)

case 0x40: TS(8); BIT0(reg_b); break;           	// bit 0,b
case 0x41: TS(8); BIT0(reg_c); break;           	// bit 0,c
case 0x42: TS(8); BIT0(reg_d); break;           	// bit 0,d
case 0x43: TS(8); BIT0(reg_e); break;           	// bit 0,e
case 0x44: TS(8); BIT0(reg_h); break;           	// bit 0,h
case 0x45: TS(8); BIT0(reg_l); break;           	// bit 0,l
case 0x46: TS(12); BITHL0(READMEM(reg_hl)); break;  // bit 0,(hl)
case 0x47: TS(8); BIT0(reg_a); break;           	// bit 0,a

case 0x48: TS(8); BIT1(reg_b); break;           	// bit 1,b
case 0x49: TS(8); BIT1(reg_c); break;           	// bit 1,c
case 0x4A: TS(8); BIT1(reg_d); break;           	// bit 1,d
case 0x4B: TS(8); BIT1(reg_e); break;           	// bit 1,e
case 0x4C: TS(8); BIT1(reg_h); break;           	// bit 1,h
case 0x4D: TS(8); BIT1(reg_l); break;           	// bit 1,l
case 0x4E: TS(12); BITHL1(READMEM(reg_hl)); break;  // bit 1,(hl)
case 0x4F: TS(8); BIT1(reg_a); break;           	// bit 1,a

case 0x50: TS(8); BIT2(reg_b); break;           	// bit 2,b
case 0x51: TS(8); BIT2(reg_c); break;           	// bit 2,c
case 0x52: TS(8); BIT2(reg_d); break;           	// bit 2,d
case 0x53: TS(8); BIT2(reg_e); break;           	// bit 2,e
case 0x54: TS(8); BIT2(reg_h); break;           	// bit 2,h
case 0x55: TS(8); BIT2(reg_l); break;           	// bit 2,l
case 0x56: TS(12); BITHL2(READMEM(reg_hl)); break;  // bit 2,(hl)
case 0x57: TS(8); BIT2(reg_a); break;           	// bit 2,a

case 0x58: TS(8); BIT3(reg_b); break;           	// bit 3,b
case 0x59: TS(8); BIT3(reg_c); break;           	// bit 3,c
case 0x5A: TS(8); BIT3(reg_d); break;           	// bit 3,d
case 0x5B: TS(8); BIT3(reg_e); break;           	// bit 3,e
case 0x5C: TS(8); BIT3(reg_h); break;           	// bit 3,h
case 0x5D: TS(8); BIT3(reg_l); break;           	// bit 3,l
case 0x5E: TS(12); BITHL3(READMEM(reg_hl)); break;  // bit 3,(hl)
case 0x5F: TS(8); BIT3(reg_a); break;           	// bit 3,a

case 0x60: TS(8); BIT4(reg_b); break;           	// bit 4,b
case 0x61: TS(8); BIT4(reg_c); break;           	// bit 4,c
case 0x62: TS(8); BIT4(reg_d); break;           	// bit 4,d
case 0x63: TS(8); BIT4(reg_e); break;           	// bit 4,e
case 0x64: TS(8); BIT4(reg_h); break;           	// bit 4,h
case 0x65: TS(8); BIT4(reg_l); break;           	// bit 4,l
case 0x66: TS(12); BITHL4(READMEM(reg_hl)); break;  // bit 4,(hl)
case 0x67: TS(8); BIT4(reg_a); break;           	// bit 4,a

case 0x68: TS(8); BIT5(reg_b); break;           	// bit 5,b
case 0x69: TS(8); BIT5(reg_c); break;           	// bit 5,c
case 0x6A: TS(8); BIT5(reg_d); break;           	// bit 5,d
case 0x6B: TS(8); BIT5(reg_e); break;           	// bit 5,e
case 0x6C: TS(8); BIT5(reg_h); break;           	// bit 5,h
case 0x6D: TS(8); BIT5(reg_l); break;           	// bit 5,l
case 0x6E: TS(12); BITHL5(READMEM(reg_hl)); break;  // bit 5,(hl)
case 0x6F: TS(8); BIT5(reg_a); break;           	// bit 5,a

case 0x70: TS(8); BIT6(reg_b); break;           	// bit 6,b
case 0x71: TS(8); BIT6(reg_c); break;           	// bit 6,c
case 0x72: TS(8); BIT6(reg_d); break;           	// bit 6,d
case 0x73: TS(8); BIT6(reg_e); break;           	// bit 6,e
case 0x74: TS(8); BIT6(reg_h); break;           	// bit 6,h
case 0x75: TS(8); BIT6(reg_l); break;           	// bit 6,l
case 0x76: TS(12); BITHL6(READMEM(reg_hl)); break;  // bit 6,(hl)
case 0x77: TS(8); BIT6(reg_a); break;           	// bit 6,a

case 0x78: TS(8); BIT7(reg_b); break;          		// bit 7,b
case 0x79: TS(8); BIT7(reg_c); break;          		// bit 7,c
case 0x7A: TS(8); BIT7(reg_d); break;          		// bit 7,d
case 0x7B: TS(8); BIT7(reg_e); break;          		// bit 7,e
case 0x7C: TS(8); BIT7(reg_h); break;          		// bit 7,h
case 0x7D: TS(8); BIT7(reg_l); break;          		// bit 7,l
case 0x7E: TS(12); BITHL7(READMEM(reg_hl)); break;  // bit 7,(hl)
case 0x7F: TS(8); BIT7(reg_a); break;          		// bit 7,a

case 0x80: TS(8); reg_b &= 0xFE; break;           // res 0,b
case 0x81: TS(8); reg_c &= 0xFE; break;           // res 0,c
case 0x82: TS(8); reg_de &= 0xFEFF; break;        // res 0,d
case 0x83: TS(8); reg_de &= 0xFFFE; break;        // res 0,e
case 0x84: TS(8); reg_hl &= 0xFEFF; break;        // res 0,h
case 0x85: TS(8); reg_hl &= 0xFFFE; break;        // res 0,l
case 0x86: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) & 0xFE); break;     // res 0,(hl)
case 0x87: TS(8); reg_a &= 0xFE; break;           // res 0,a

case 0x88: TS(8); reg_b &= 0xFD; break;           // res 1,b
case 0x89: TS(8); reg_c &= 0xFD; break;           // res 1,c
case 0x8A: TS(8); reg_de &= 0xFDFF; break;        // res 1,d
case 0x8B: TS(8); reg_de &= 0xFFFD; break;        // res 1,e
case 0x8C: TS(8); reg_hl &= 0xFDFF; break;        // res 1,h
case 0x8D: TS(8); reg_hl &= 0xFFFD; break;        // res 1,l
case 0x8E: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) & 0xFD); break;     // res 1,(hl)
case 0x8F: TS(8); reg_a &= 0xFD; break;           // res 1,a

case 0x90: TS(8); reg_b &= 0xFB; break;           // res 2,b
case 0x91: TS(8); reg_c &= 0xFB; break;           // res 2,c
case 0x92: TS(8); reg_de &= 0xFBFF; break;        // res 2,d
case 0x93: TS(8); reg_de &= 0xFFFB; break;        // res 2,e
case 0x94: TS(8); reg_hl &= 0xFBFF; break;        // res 2,h
case 0x95: TS(8); reg_hl &= 0xFFFB; break;        // res 2,l
case 0x96: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) & 0xFB); break;     // res 2,(hl)
case 0x97: TS(8); reg_a &= 0xFB; break;           // res 2,a

case 0x98: TS(8); reg_b &= 0xF7; break;           // res 3,b
case 0x99: TS(8); reg_c &= 0xF7; break;           // res 3,c
case 0x9A: TS(8); reg_de &= 0xF7FF; break;        // res 3,d
case 0x9B: TS(8); reg_de &= 0xFFF7; break;        // res 3,e
case 0x9C: TS(8); reg_hl &= 0xF7FF; break;        // res 3,h
case 0x9D: TS(8); reg_hl &= 0xFFF7; break;        // res 3,l
case 0x9E: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) & 0xF7); break;     // res 3,(hl)
case 0x9F: TS(8); reg_a &= 0xF7; break;           // res 3,a

case 0xA0: TS(8); reg_b &= 0xEF; break;           // res 4,b
case 0xA1: TS(8); reg_c &= 0xEF; break;           // res 4,c
case 0xA2: TS(8); reg_de &= 0xEFFF; break;        // res 4,d
case 0xA3: TS(8); reg_de &= 0xFFEF; break;        // res 4,e
case 0xA4: TS(8); reg_hl &= 0xEFFF; break;        // res 4,h
case 0xA5: TS(8); reg_hl &= 0xFFEF; break;        // res 4,l
case 0xA6: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) & 0xEF); break;     // res 4,(hl)
case 0xA7: TS(8); reg_a &= 0xEF; break;           // res 4,a

case 0xA8: TS(8); reg_b &= 0xDF; break;           // res 5,b
case 0xA9: TS(8); reg_c &= 0xDF; break;           // res 5,c
case 0xAA: TS(8); reg_de &= 0xDFFF; break;        // res 5,d
case 0xAB: TS(8); reg_de &= 0xFFDF; break;        // res 5,e
case 0xAC: TS(8); reg_hl &= 0xDFFF; break;        // res 5,h
case 0xAD: TS(8); reg_hl &= 0xFFDF; break;        // res 5,l
case 0xAE: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) & 0xDF); break;     // res 5,(hl)
case 0xAF: TS(8); reg_a &= 0xDF; break;           // res 5,a

case 0xB0: TS(8); reg_b &= 0xBF; break;           // res 6,b
case 0xB1: TS(8); reg_c &= 0xBF; break;           // res 6,c
case 0xB2: TS(8); reg_de &= 0xBFFF; break;        // res 6,d
case 0xB3: TS(8); reg_de &= 0xFFBF; break;        // res 6,e
case 0xB4: TS(8); reg_hl &= 0xBFFF; break;        // res 6,h
case 0xB5: TS(8); reg_hl &= 0xFFBF; break;        // res 6,l
case 0xB6: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) & 0xBF); break;     // res 6,(hl)
case 0xB7: TS(8); reg_a &= 0xBF; break;           // res 6,a

case 0xB8: TS(8); reg_b &= 0x7F; break;           // res 7,b
case 0xB9: TS(8); reg_c &= 0x7F; break;           // res 7,c
case 0xBA: TS(8); reg_de &= 0x7FFF; break;        // res 7,d
case 0xBB: TS(8); reg_de &= 0xFF7F; break;        // res 7,e
case 0xBC: TS(8); reg_hl &= 0x7FFF; break;        // res 7,h
case 0xBD: TS(8); reg_hl &= 0xFF7F; break;        // res 7,l
case 0xBE: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) & 0x7F); break;     // res 7,(hl)
case 0xBF: TS(8); reg_a &= 0x7F; break;           // res 7,a

case 0xC0: TS(8); reg_b |= 0x01; break;           // set 0,b
case 0xC1: TS(8); reg_c |= 0x01; break;           // set 0,c
case 0xC2: TS(8); reg_de |= 0x0100; break;        // set 0,d
case 0xC3: TS(8); reg_de |= 0x0001; break;        // set 0,e
case 0xC4: TS(8); reg_hl |= 0x0100; break;        // set 0,h
case 0xC5: TS(8); reg_hl |= 0x0001; break;        // set 0,l
case 0xC6: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) | 0x01); break;     // set 0,(hl)
case 0xC7: TS(8); reg_a |= 0x01; break;           // set 0,a

case 0xC8: TS(8); reg_b |= 0x02; break;           // set 1,b
case 0xC9: TS(8); reg_c |= 0x02; break;           // set 1,c
case 0xCA: TS(8); reg_de |= 0x0200; break;        // set 1,d
case 0xCB: TS(8); reg_de |= 0x0002; break;        // set 1,e
case 0xCC: TS(8); reg_hl |= 0x0200; break;        // set 1,h
case 0xCD: TS(8); reg_hl |= 0x0002; break;        // set 1,l
case 0xCE: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) | 0x02); break;     // set 1,(hl)
case 0xCF: TS(8); reg_a |= 0x02; break;           // set 1,a

case 0xD0: TS(8); reg_b |= 0x04; break;           // set 2,b
case 0xD1: TS(8); reg_c |= 0x04; break;           // set 2,c
case 0xD2: TS(8); reg_de |= 0x0400; break;        // set 2,d
case 0xD3: TS(8); reg_de |= 0x0004; break;        // set 2,e
case 0xD4: TS(8); reg_hl |= 0x0400; break;        // set 2,h
case 0xD5: TS(8); reg_hl |= 0x0004; break;        // set 2,l
case 0xD6: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) | 0x04); break;     // set 2,(hl)
case 0xD7: TS(8); reg_a |= 0x04; break;           // set 2,a

case 0xD8: TS(8); reg_b |= 0x08; break;           // set 3,b
case 0xD9: TS(8); reg_c |= 0x08; break;           // set 3,c
case 0xDA: TS(8); reg_de |= 0x0800; break;        // set 3,d
case 0xDB: TS(8); reg_de |= 0x0008; break;        // set 3,e
case 0xDC: TS(8); reg_hl |= 0x0800; break;        // set 3,h
case 0xDD: TS(8); reg_hl |= 0x0008; break;        // set 3,l
case 0xDE: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) | 0x08); break;     // set 3,(hl)
case 0xDF: TS(8); reg_a |= 0x08; break;           // set 3,a

case 0xE0: TS(8); reg_b |= 0x10; break;           // set 4,b
case 0xE1: TS(8); reg_c |= 0x10; break;           // set 4,c
case 0xE2: TS(8); reg_de |= 0x1000; break;        // set 4,d
case 0xE3: TS(8); reg_de |= 0x0010; break;        // set 4,e
case 0xE4: TS(8); reg_hl |= 0x1000; break;        // set 4,h
case 0xE5: TS(8); reg_hl |= 0x0010; break;        // set 4,l
case 0xE6: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) | 0x10); break;     // set 4,(hl)
case 0xE7: TS(8); reg_a |= 0x10; break;           // set 4,a

case 0xE8: TS(8); reg_b |= 0x20; break;           // set 5,b
case 0xE9: TS(8); reg_c |= 0x20; break;           // set 5,c
case 0xEA: TS(8); reg_de |= 0x2000; break;        // set 5,d
case 0xEB: TS(8); reg_de |= 0x0020; break;        // set 5,e
case 0xEC: TS(8); reg_hl |= 0x2000; break;        // set 5,h
case 0xED: TS(8); reg_hl |= 0x0020; break;        // set 5,l
case 0xEE: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) | 0x20); break;     // set 5,(hl)
case 0xEF: TS(8); reg_a |= 0x20; break;           // set 5,a

case 0xF0: TS(8); reg_b |= 0x40; break;           // set 6,b
case 0xF1: TS(8); reg_c |= 0x40; break;           // set 6,c
case 0xF2: TS(8); reg_de |= 0x4000; break;        // set 6,d
case 0xF3: TS(8); reg_de |= 0x0040; break;        // set 6,e
case 0xF4: TS(8); reg_hl |= 0x4000; break;        // set 6,h
case 0xF5: TS(8); reg_hl |= 0x0040; break;        // set 6,l
case 0xF6: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) | 0x40); break;     // set 6,(hl)
case 0xF7: TS(8); reg_a |= 0x40; break;           // set 6,a

case 0xF8: TS(8); reg_b |= 0x80; break;           // set 7,b
case 0xF9: TS(8); reg_c |= 0x80; break;           // set 7,c
case 0xFA: TS(8); reg_de |= 0x8000; break;        // set 7,d
case 0xFB: TS(8); reg_de |= 0x0080; break;        // set 7,e
case 0xFC: TS(8); reg_hl |= 0x8000; break;        // set 7,h
case 0xFD: TS(8); reg_hl |= 0x0080; break;        // set 7,l
case 0xFE: TS(15); WRITEMEM(reg_hl, READMEM(reg_hl) | 0x80); break;     // set 7,(hl)
case 0xFF: TS(8); reg_a |= 0x80; break;           // set 7,a

default:
	assert(false); // opcode > 255 ?!
	break;
