//! opcodesDDCB.inc rotate and shift group


// hier geen opcodeFetch, want waarschijnlijk wordt er geen M1 uitgevoerd
// reg_r wordt dus ook niet nog een extra keer verhoogd

reg1 = READMEM(reg_pc++);
reg_wz = (REGIX + (reg1 & 127) - (reg1 & 128)) & 0xffff;		// IX+d
reg2 = READMEM(reg_wz);
opcode = READMEM(reg_pc++);

switch (opcode >> 3) {
case 0x00: RLC(reg2); break;	
case 0x01: RRC(reg2); break;	
case 0x02: RL(reg2); break;	
case 0x03: RR(reg2); break;	
case 0x04: SLA(reg2); break;	
case 0x05: SRA(reg2); break;	
case 0x06: SLL(reg2); break; 	// undocumented
case 0x07: SRL(reg2); break;

#define BITIX(b) reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((reg2&b) ? HFLAG:PFLAG|ZFLAG|HFLAG)
case 0x08: TS(20); BITIX(0x01); break;	// bit 0,(ix+d)
case 0x09: TS(20); BITIX(0x02); break;	// bit 1,(ix+d)
case 0x0a: TS(20); BITIX(0x04); break;	// bit 2,(ix+d)
case 0x0b: TS(20); BITIX(0x08); break;	// bit 3,(ix+d)
case 0x0c: TS(20); BITIX(0x10); break;	// bit 4,(ix+d)
case 0x0d: TS(20); BITIX(0x20); break;	// bit 5,(ix+d)
case 0x0e: TS(20); BITIX(0x40); break;	// bit 6,(ix+d)
case 0x0f: 
	TS(20); 
	reg_f = (reg_f&CFLAG) | ((reg_wz>>8)&0x28) | ((reg2&0x80) ? SFLAG|HFLAG:PFLAG|ZFLAG|HFLAG);
	break;	// bit 7,(ix+d)

case 0x10: reg2 &= 0xfe; break;	// res 0,(ix+d)
case 0x11: reg2 &= 0xfd; break;	// res 1,(ix+d)
case 0x12: reg2 &= 0xfb; break;	// res 2,(ix+d)
case 0x13: reg2 &= 0xf7; break;	// res 3,(ix+d)
case 0x14: reg2 &= 0xef; break;	// res 4,(ix+d)
case 0x15: reg2 &= 0xdf; break;	// res 5,(ix+d)
case 0x16: reg2 &= 0xbf; break;	// res 6,(ix+d)
case 0x17: reg2 &= 0x7f; break;	// res 7,(ix+d)

case 0x18: reg2 |= 0x01; break;	// set 0,(ix+d)
case 0x19: reg2 |= 0x02; break;	// set 1,(ix+d)
case 0x1a: reg2 |= 0x04; break;	// set 2,(ix+d)
case 0x1b: reg2 |= 0x08; break;	// set 3,(ix+d)
case 0x1c: reg2 |= 0x10; break;	// set 4,(ix+d)
case 0x1d: reg2 |= 0x20; break;	// set 5,(ix+d)
case 0x1e: reg2 |= 0x40; break;	// set 6,(ix+d)
case 0x1f: reg2 |= 0x80; break;	// set 7,(ix+d)
default: assert(false);
}

if ((opcode & 0xc0) != 0x40) {

	//assert((opcode & 7)==6); // TODO: testen daarna kan deze assert eruit!
	TS(23);
	WRITEMEM(reg_wz, reg2);
	switch (opcode & 7) {
	case 0x00: reg_b = reg2; break;							// reg_b
	case 0x01: reg_c = reg2; break;							// reg_c
	case 0x02: reg_de = LREG(reg_de)|(reg2<<8); break;		// reg_d
	case 0x03: reg_de = HREG(reg_de)|reg2; break;			// reg_e
	case 0x04: reg_hl = LREG(reg_hl)|(reg2<<8); break;		// reg_h
	case 0x05: reg_hl = HREG(reg_hl)|reg2; break;			// reg_l
	case 0x06: break; // documented instruction
	case 0x07: reg_a = reg2; break;							// reg_a

	default: assert(false);
	}
}
