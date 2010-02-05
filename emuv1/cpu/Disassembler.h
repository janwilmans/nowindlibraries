//! Disassembler.h
#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <map>
#include <iostream>
#include <string>
#include "msxtypes.h"

class Z80;

static const char mnemonic[256][20] =
{
	"nop",         "ld   bc,MMMM","ld   (bc),a",  "inc  bc", "inc  b",  "dec  b",  "ld   b,NN",  "rlca",
	"ex   af,af'", "add  Y,bc",   "ld   a,(bc)",  "dec  bc", "inc  c",  "dec  c",  "ld   c,NN",  "rrca",
	"djnz EEEE",   "ld   de,MMMM","ld   (de),a",  "inc  de", "inc  d",  "dec  d",  "ld   d,NN",  "rla",
	"jr   EEEE",   "add  Y,de",   "ld   a,(de)",  "dec  de", "inc  e",  "dec  e",  "ld   e,NN",  "rra",
	"jr   nz,EEEE","ld   Y,MMMM", "ld   (MMMM),Y","inc  Y",  "inc  h",  "dec  h",  "ld   h,NN",  "daa",
	"jr   z,EEEE", "add  Y,R",    "ld   Y,(MMMM)","dec  Y",  "inc  l",  "dec  l",  "ld   l,NN",  "cpl",
	"jr   nc,EEEE","ld   sp,MMMM","ld   (MMMM),a","inc  sp", "inc  (X)","dec  (X)","ld   (X),NN","scf",
	"jr   c,EEEE", "add  Y,sp",   "ld   a,(MMMM)","dec  sp", "inc  a",  "dec  a",  "ld   a,NN",  "ccf",

	"ld   b,b", "ld   b,c", "ld   b,d", "ld   b,e", "ld   b,h", "ld   b,l", "ld   b,(X)", "ld   b,a",
	"ld   c,b", "ld   c,c", "ld   c,d", "ld   c,e", "ld   c,h", "ld   c,l", "ld   c,(X)", "ld   c,a",
	"ld   d,b", "ld   d,c", "ld   d,d", "ld   d,e", "ld   d,h", "ld   d,l", "ld   d,(X)", "ld   d,a",
	"ld   e,b", "ld   e,c", "ld   e,d", "ld   e,e", "ld   e,h", "ld   e,l", "ld   e,(X)", "ld   e,a",
	"ld   h,b", "ld   h,c", "ld   h,d", "ld   h,e", "ld   h,h", "ld   h,l", "ld   h,(X)", "ld   h,a",
	"ld   l,b", "ld   l,c", "ld   l,d", "ld   l,e", "ld   l,h", "ld   l,l", "ld   l,(X)", "ld   l,a",
	"ld   (X),b", "ld   (X),c", "ld   (X),d", "ld   (X),e",
	"ld   (X),h", "ld   (X),l", "halt",        "ld   (X),a",
	"ld   a,b", "ld   a,c", "ld   a,d", "ld   a,e", "ld   a,h", "ld   a,l", "ld   a,(X)", "ld   a,a",

	"add  a,b","add  a,c","add  a,d","add  a,e","add  a,h","add  a,l","add  a,(X)","add  a,a",
	"adc  a,b","adc  a,c","adc  a,d","adc  a,e","adc  a,h","adc  a,l","adc  a,(X)","adc  a,a",
	"sub  b",  "sub  c",  "sub  d",  "sub  e",  "sub  h",  "sub  l",  "sub  (X)",  "sub  a",
	"sbc  a,b","sbc  a,c","sbc  a,d","sbc  a,e","sbc  a,h","sbc  a,l","sbc  a,(X)","sbc  a,a",
	"and  b",  "and  c",  "and  d",  "and  e",  "and  h",  "and  l",  "and  (X)",  "and  a",
	"xor  b",  "xor  c",  "xor  d",  "xor  e",  "xor  h",  "xor  l",  "xor  (X)",  "xor  a",
	"or   b",  "or   c",  "or   d",  "or   e",  "or   h",  "or   l",  "or   (X)",  "or   a",
	"cp   b",  "cp   c",  "cp   d",  "cp   e",  "cp   h",  "cp   l",  "cp   (X)",  "cp   a",

	"ret  nz","pop  bc",   "jp   nz,MMMM","jp   MMMM",   "call nz,MMMM","push bc",  "add  a,NN","rst  00",
	"ret  z", "ret",       "jp   z,MMMM", "CB",          "call z,MMMM", "call MMMM","adc  a,NN","rst  08",
	"ret  nc","pop  de",   "jp   nc,MMMM","out  (NN),a", "call nc,MMMM","push de",  "sub  NN",  "rst  10",
	"ret  c", "exx",       "jp   c,MMMM", "in   a,(NN)", "call c,MMMM", "DD",       "sbc  a,NN","rst  18",
	"ret  po","pop  Y",   "jp   po,MMMM","ex   (sp),Y","call po,MMMM","push Y",  "and  NN",  "rst  20",
	"ret  pe","jp   (Y)", "jp   pe,MMMM","ex   de,Y",  "call pe,MMMM","ED",       "xor  NN",  "rst  28",
	"ret  p", "pop  af",   "jp   p,MMMM", "di",          "call p,MMMM", "push af",  "or   NN",  "rst  30",
	"ret  m", "ld   sp,Y","jp   m,MMMM", "ei",          "call m,MMMM", "FD",       "cp   NN",  "rst  38"
};

static const char mnemonicCB[][25] =
{
	"rlc  b","rlc  c","rlc  d","rlc  e","rlc  h","rlc  l","rlc  (X)","rlc  a",
	"rrc  b","rrc  c","rrc  d","rrc  e","rrc  h","rrc  l","rrc  (X)","rrc  a",
	"rl   b","rl   c","rl   d","rl   e","rl   h","rl   l","rl   (X)","rl   a",
	"rr   b","rr   c","rr   d","rr   e","rr   h","rr   l","rr   (X)","rr   a",
	"sla  b","sla  c","sla  d","sla  e","sla  h","sla  l","sla  (X)","sla  a",
	"sra  b","sra  c","sra  d","sra  e","sra  h","sra  l","sra  (X)","sra  a",
	"sll  b","sll  c","sll  d","sll  e","sll  h","sll  l","sll  (X)","sll  a",
	"srl  b","srl  c","srl  d","srl  e","srl  h","srl  l","srl  (X)","srl  a",

	"bit  0,b","bit  0,c","bit  0,d","bit  0,e","bit  0,h","bit  0,l","bit  0,(X)","bit  0,a",
	"bit  1,b","bit  1,c","bit  1,d","bit  1,e","bit  1,h","bit  1,l","bit  1,(X)","bit  1,a",
	"bit  2,b","bit  2,c","bit  2,d","bit  2,e","bit  2,h","bit  2,l","bit  2,(X)","bit  2,a",
	"bit  3,b","bit  3,c","bit  3,d","bit  3,e","bit  3,h","bit  3,l","bit  3,(X)","bit  3,a",
	"bit  4,b","bit  4,c","bit  4,d","bit  4,e","bit  4,h","bit  4,l","bit  4,(X)","bit  4,a",
	"bit  5,b","bit  5,c","bit  5,d","bit  5,e","bit  5,h","bit  5,l","bit  5,(X)","bit  5,a",
	"bit  6,b","bit  6,c","bit  6,d","bit  6,e","bit  6,h","bit  6,l","bit  6,(X)","bit  6,a",
	"bit  7,b","bit  7,c","bit  7,d","bit  7,e","bit  7,h","bit  7,l","bit  7,(X)","bit  7,a",

	"res  0,b","res  0,c","res  0,d","res  0,e","res  0,h","res  0,l","res  0,(X)","res  0,a",
	"res  1,b","res  1,c","res  1,d","res  1,e","res  1,h","res  1,l","res  1,(X)","res  1,a",
	"res  2,b","res  2,c","res  2,d","res  2,e","res  2,h","res  2,l","res  2,(X)","res  2,a",
	"res  3,b","res  3,c","res  3,d","res  3,e","res  3,h","res  3,l","res  3,(X)","res  3,a",
	"res  4,b","res  4,c","res  4,d","res  4,e","res  4,h","res  4,l","res  4,(X)","res  4,a",
	"res  5,b","res  5,c","res  5,d","res  5,e","res  5,h","res  5,l","res  5,(X)","res  5,a",
	"res  6,b","res  6,c","res  6,d","res  6,e","res  6,h","res  6,l","res  6,(X)","res  6,a",
	"res  7,b","res  7,c","res  7,d","res  7,e","res  7,h","res  7,l","res  7,(X)","res  7,a",

	"set  0,b","set  0,c","set  0,d","set  0,e","set  0,h","set  0,l","set  0,(X)","set  0,a",
	"set  1,b","set  1,c","set  1,d","set  1,e","set  1,h","set  1,l","set  1,(X)","set  1,a",
	"set  2,b","set  2,c","set  2,d","set  2,e","set  2,h","set  2,l","set  2,(X)","set  2,a",
	"set  3,b","set  3,c","set  3,d","set  3,e","set  3,h","set  3,l","set  3,(X)","set  3,a",
	"set  4,b","set  4,c","set  4,d","set  4,e","set  4,h","set  4,l","set  4,(X)","set  4,a",
	"set  5,b","set  5,c","set  5,d","set  5,e","set  5,h","set  5,l","set  5,(X)","set  5,a",
	"set  6,b","set  6,c","set  6,d","set  6,e","set  6,h","set  6,l","set  6,(X)","set  6,a",
	"set  7,b","set  7,c","set  7,d","set  7,e","set  7,h","set  7,l","set  7,(X)","set  7,a"
};

static const char mnemonicED[][25] =
{
	"[ed]DISKIO","[ed]HIJACK BDOS","??","??","??","??","??","[ed]dumpCpuInfo",  
    "[ed]dumpSlotSelection","??","??","[ed]enableDisasm","[ed]disableDisasm","??","??","??",
    
	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??",
	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??",
	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??",

	"in   b,(c)","out  (c),b","sbc  Y,bc","ld   (MMMM),bc","neg","retn", "im   0","ld   i,a",
	"in   c,(c)","out  (c),c","adc  Y,bc","ld   bc,(MMMM)","??",  "reti","??",    "ld   r,a",
	"in   d,(c)","out  (c),d","sbc  Y,de","ld   (MMMM),de","??",  "??",  "im   1","ld   a,i",
	"in   e,(c)","out  (c),e","adc  Y,de","ld   de,(MMMM)","??",  "??",  "im   2","ld   a,r",
	"in   h,(c)","out  (c),h","sbc  Y,R","ld   (MMMM),Y","??",  "??",  "??",    "rrd",
	"in   l,(c)","out  (c),l","adc  Y,R","ld   Y,(MMMM)","??",  "??",  "??",    "rld",
	"??",        "??",        "sbc  Y,sp","ld   (MMMM),sp","??",  "??",  "??",    "??",
	"in   a,(c)","out  (c),a","adc  Y,sp","ld   sp,(MMMM)","??",  "??",  "??",    "??",

	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??",
	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??",

	"ldi", "cpi", "ini", "outi","??","??","??","??","ldd", "cpd", "ind", "outd","??","??","??","??",
	"ldir","cpir","inir","otir","??","??","??","??","lddr","cpdr","indr","outr","??","??","??","??",

	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??",
	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??",
	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??",
	"??","??","??","??", "??","??","??","??",  "??","??","??","??", "??","??","??","??"
};

/*!
 * The Disassembler class can disassemble z80-opcodes into a human-readable format
 * it is mainly for debugging purposes.
 */
class Disassembler {

private:

	Disassembler();
	int                size;
	char               mstr[30];
	Z80                *z80;
    
	unsigned long      lineNumber;

	nw_word            startDisassbleAddress;	
	emuTimeType        startDisassbleEmuTime;

	bool 			   startByProgramCounter;
	bool 			   startByEmuTime;
					
	nw_word            instructionsLeftToDisassble;
	nw_word            previewWord1;
	nw_word            previewWord2;
	double             lastEmuTime;

	nw_word            reg_pc;
	nw_word            reg_sp;
	nw_word            register_af;
	nw_word            register_bc;
	nw_word            register_de;
	nw_word            register_hl;
	nw_word            register_ix;
	nw_word            register_iy;

	nw_word            shadow_af;
	nw_word            shadow_bc;
	nw_word            shadow_de;
	nw_word            shadow_hl;
	
	std::map<nw_word, std::string> symbolnames;

public:

	std::ofstream      *logfile;
	bool               disassembling;

	~Disassembler();
	static Disassembler *Instance();
	char *disAsm(nw_word pc, nw_word w1, nw_word w2);
	int getSize();

	void setDisassemblyPC(nw_word pc, unsigned int instructionCount);
	void setDisassemblyEmuTime(emuTimeType emuTime, unsigned int instructionCount);
	void setDisassemblyPreview(emuTimeType emuTime);
	void triggerDisassembler();
	void logRegisterColored(nw_word firstReg, nw_word secordReg);
	std::string recallInstruction();
	
	std::string disAsm(nw_word *pc_pnt, nw_word w1, nw_word w2,bool hotinfo);
	
};

#endif

