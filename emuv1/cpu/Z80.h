//! Z80.h
#ifndef Z80_H
#define Z80_H

#include <iostream>

#include <string>
#include <sstream>
#include <map>
#include <stdlib.h>

#include "msxtypes.h"
#include "FastDelegate.h"
#include "Emulator.h"
#include "EmulatorTester.h"
#include "devices/SlotSelector.h"
#include "memory/RamBlock.h"
#include "memory/RomBlock.h"

//#define inline  __forceinline
//#define inline_after __attribute__((always_inline))
#define inline_after ;//
// class functies nooit inline ?
// mogelijke speed-up: readmem etc. uit de class halen? 

typedef fastdelegate::FastDelegate2<nw_word, nw_byte> writeDelegate;   // returns void 

class Debug;
class Mapper;
class V9938;
class RP5C01;
class AY38910;
class YM2413;
class SCC;
class MSXAudio;
class DiskInterface;
class I8255;
class Disassembler;
class Kanji;
class SwitchedPorts;
class DebugDevice;
class F4Device;
class Profiler;

/* all of these macros are using in opcodes*.inc so they are easely be adapted
   to used on different platforms and in different configurations */

#define TS(states) emuTime += states

#define reg_af ((reg_a << 8) | reg_f)
#define reg_bc ((reg_b << 8) | reg_c)

#define reg_d (reg_de >> 8)
#define reg_e (reg_de & 255)

#define reg_h (reg_hl >> 8)
#define reg_l (reg_hl & 255)

#define SFLAG 0x80
#define ZFLAG 0x40
#define YFLAG 0x20
#define HFLAG 0x10
#define XFLAG 0x08
#define PFLAG 0x04
#define NFLAG 0x02
#define CFLAG 0x01

/* memory read/write macros */
#define READMEM readMem
#define READMEM16 readMem16
#define WRITEMEM writeMem
#define WRITEMEM16 writeMem16

class Z80 {

friend class EmulatorTester;
friend class Disassembler;
friend class Profiler;
friend class DiskInterface;
friend class Debugger;
friend class SlotSelector;
friend class Media;

// solve this by creating public functions
friend class Mapper;
friend class RamBlock;
friend class RomBlock;
friend class MapperKonami4;
friend class MapperKonami5;
friend class MapperMsxDos2;
friend class MapperFMPAC;
friend class NowindInterface;
friend class SCC;
friend class EmptyPage;
friend class WD279X;

private:

		Z80();

        float opcodeCounter[256];
        float opcodeCounterCB[256];
        float opcodeCounterDD[256];
        float opcodeCounterED[256];
        float opcodeCounterFD[256];
        
        /* new memory layout */
        const nw_byte   *readBlock[8];
        writeDelegate   writeFunc[8];

        SlotSelector    *slotSelector;
        
		/* things that do not actually belong inside the z80 */
		Mapper          *mapper;
		Mapper  		*mapperCartride;
		DiskInterface   *disk;

		/* interne z80 registers */
		nw_word			reg_a;
		nw_word			reg_f;
		nw_word			reg_b;
		nw_word			reg_c;
		
		nw_word		    reg_i;
		nw_word    		reg_r;
		nw_word			reg_de;		// TODO: uitzoeken of DE vaker als 8 bits registers wordt gebruikt
		nw_word			reg_hl;
		nw_word    		reg_pc;
		nw_word    		reg_sp;
		nw_word    		reg_ix;
        nw_word    		reg_iy;
		nw_word			reg_wz;

		nw_word    		shadow_af;
		nw_word   		shadow_b;
		nw_word			shadow_c;
		nw_word    		shadow_de;
		nw_word    		shadow_hl;

		bool            IFF1;
		bool            IFF2;
		nw_word    		interruptMode;
		nw_word			refreshCounter;

		nw_word    		flagSZ[256];
		nw_word    		flagSZsub[256];
		nw_word    		flagSZP[256];
		nw_word    		flagInc[256];
		nw_word    		flagDec[256];

		inline nw_byte	opcodeFetch(nw_word) inline_after;
		void            debugInstuctionCounter();

		void            writeIo(nw_word, nw_byte);
		nw_byte         readIo(nw_word);
 
		V9938           *vdp;
		RP5C01          *realTimeClock;
		AY38910         *psg;
		YM2413			*fmpac;
		SCC				*scc;
		MSXAudio        *msxaudio;
		I8255			*ppi;
		Kanji			*kanji;
		SwitchedPorts	*switchedPorts;
		DebugDevice		*debugDevice;
		F4Device		*f4Device;
        Profiler        *profiler;

		inline nw_byte	readMem(nw_word) inline_after;
		inline nw_word	readMem16(nw_word) inline_after;
		inline void     writeMem(nw_word,nw_byte) inline_after;
		inline void     writeMem16(nw_word,nw_word) inline_after;	
		
public:
        void            setupBdosEnv(const char* filename);
		nw_byte			readMemPublic(nw_word address);
		nw_word			readMem16Public(nw_word address);
		void 			writeMemPublic(nw_word address, nw_byte value);
		void 			writeMem16Public(nw_word address, nw_word value);

        emuTimeType     nextInterrupt;
		unsigned long   cpuFrequency;

		nw_word         start_test_nr;

		static          Z80 * Instance();
						~Z80();
		void            initialize();

		void            reset();
		void            hardReset();
		void            nmiCPU();
		bool			getIFF1();
		void            intCPU(nw_byte);
		void            start(nw_word);

		void            executeInstructions();

		void            saveState();
		void            loadState();

		emuTimeType     emuTime;

		void            hijackBdos();
		unsigned long   bdosCount;
		void            dumpPages();
		void            dumpSlotSelection();
		void            dumpCpuInfo();		
        void            abortEmulator();
		nw_word         getSP();
};

#endif
