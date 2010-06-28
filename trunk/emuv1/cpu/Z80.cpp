// Z80.cc

#include <iostream>
#include <fstream>
#include "cpu/Z80.h"

using namespace std;
using namespace fastdelegate;

#include "Config.h"
#include "stdio.h"
#include "Debug.h"
#include "memory/Mapper.h"
#include "video/V9938.h"
#include "devices/RP5C01.h"
#include "audio/AY38910.h"
#include "audio/YM2413.h"
#include "audio/MSXAudio.h"
#include "DiskInterface.h"
#include "devices/I8255.h"
#include "msxtypes.h"
#include "cpu/Disassembler.h"
#include "devices/Kanji.h"
#include "devices/SwitchedPorts.h"
#include "devices/DebugDevice.h"
#include "devices/F4Device.h"
#include "Media.h"
#include "cpu/Profiler.h"


Z80 * Z80::Instance() {

		/* implies singleton class */

		static Z80 deInstantie;
		return &deInstantie;
}

Z80::Z80() {

        DBERR("Z80 constructor...\n");
		/* set the CPU frequency to MSX's normal 3,57 Mhz */
        cpuFrequency = 3579545;
			
		/* initialize flag tables
		 * instructions use these so the flags do not have to be calculated 
		 * the undocumented (we call them: X and Y) flags are calculated here too.
   		 */
		for (int i=0;i<256;++i) {

			flagSZ[i] = (i & (SFLAG|XFLAG|YFLAG)) | ((i==0) ? ZFLAG:0);
			flagSZsub[i] = flagSZ[i] | NFLAG;
			nw_byte p=1;
			for (int j=0;j<8;++j) p^=((i>>j)&1);
			flagSZP[i] = flagSZ[i] | (p<<2);
			flagInc[i] = flagSZ[i] | ((i==0x80) ? PFLAG:0) | ((i ^ (i-1)) & HFLAG);
			flagDec[i] = flagSZsub[i] | ((i==0x7F) ? PFLAG:0) | ((i ^ (i+1)) & HFLAG);
		}

		bdosCount = 0;
		Debug::Instance()->RUNTIME_INSTRUCTIONS_ON = false;
        DBERR("Z80 constructor finished.\n");
}

void Z80::initialize() {

		/* get some local pointers to other hardware around the z80 */
		vdp = V9938::Instance();	
		psg = AY38910::Instance();
		fmpac = YM2413::Instance();
        msxaudio = MSXAudio::Instance();
        disk = DiskInterface::Instance();
		switchedPorts = SwitchedPorts::Instance();

		kanji = new Kanji();
		debugDevice = new DebugDevice();
		realTimeClock = new RP5C01();
		f4Device = new F4Device();
		ppi = new I8255();

		profiler = new Profiler();

		slotSelector = SlotSelector::Instance();
		slotSelector->configure(Config::Instance()->msxVersion);

		//CheckSanity();
		DBERR("Cpu initialized...\n");
}

Z80::~Z80() {
		DBERR("z80 destruction in progress...\n");
		/* opruimen van aangemaakte objecten */

		delete f4Device;
		delete kanji;
		delete debugDevice;
		delete realTimeClock;
		delete ppi;
        delete profiler;
		
		/* the memory for loaded roms: Mainrom, Subrom, Diskrom, Cartridges etc. should be freed */
		DBERR("z80 destroyed.\n");
}

void Z80::hardReset() {

		reset();
		mapper->wipe();
}

void Z80::reset() {

		DBERR("start of Z80 RESET\n");

   	    /* rom inschakelen */
		ppi->reset();
    	writeIo(0xa8,0);                
        writeMem(0xffff,0);             

		IFF1 = IFF2 = false;  // disable interrupts
		interruptMode = 0;
		
		reg_pc = 0x0000;
		reg_a = 0xFF; 
		reg_f = 0xFF;
		reg_b = 0xFF;
		reg_c = 0xFF;
		reg_de = 0xFFFF;
		reg_hl = 0xFFFF;
		reg_ix = 0xFFFF;
		reg_iy = 0xFFFF;
		reg_sp = 0xFFFF;
		shadow_af = shadow_de = shadow_hl = 0xFFFF;
		shadow_b = shadow_c = 0xFF;
		
        reg_i = 0;
        reg_r = 0;
		refreshCounter = 0;

        DBERR(" *- RESET *-\n");
}

void Z80::nmiCPU() {

		// aantal klokpulsen???
		// IFF2 = IFF1;                         /* gebeurt niet volgens Sean Young, moet de NMI routine zelf doen */
		IFF1 = false;

		// cpu weer wakker maken na een HALT
		if (readMem(reg_pc)==0x76) reg_pc++;

		reg_sp -= 2;
		writeMem16(reg_sp, reg_pc); //TODO: volgens de documentatie wordt er maar een M1 uitgevoerd???
		reg_pc = 0x0066;
}

bool Z80::getIFF1() {
	 return IFF1;
}



void Z80::intCPU(nw_byte interruptVectorOnDataBus) {

		//DBERR("Z80::intCPU...\n");
		
		if (!IFF1) { 
  			DBERR("  ints are disabled.. -> return (DIT MAG NIET MEER VOORKOMEN)\n");
      		assert(false);
		}
		
        // When an interrupt is accepted, the Z80 clears IFF1 and IFF2    
		IFF1 = false;
		IFF2 = false;
		
		Emulator::Instance()->setCPUInterrupt(false);
		Emulator::Instance()->scheduleVDPInterrupt(false, 4);

#ifndef FULL_SPEED_ON
		assert(reg_pc < 0x10000);
#endif

		// cpu weer wakker maken na een HALT
		if (unlikely(readMem(reg_pc)==0x76)) reg_pc++;

		// Interrupt Acknowledge Cycle
		// The Z80 generates a special (NOT AN EXTRA) M1 cycle and adds two wait states to
		// this cycle. Futhermore, the MSX-engine adds another wait state to this M1 cycle.

        refreshCounter++;
		reg_sp -= 2;
		writeMem16(reg_sp, reg_pc);
		
		switch (interruptMode) {
		case 0:
			emuTime += 14;
		
			// the instruction can be any 1 byte instruction, usually this will be rst.
			// TODO: hoeveel states...
			switch (interruptVectorOnDataBus) {
			case 0xc7: reg_pc = 0x0000; break;
			case 0xcf: reg_pc = 0x0008; break;
			case 0xd7: reg_pc = 0x0010; break;
			case 0xdf: reg_pc = 0x0018; break;
			case 0xe7: reg_pc = 0x0020; break;
			case 0xef: reg_pc = 0x0028; break;
			case 0xf7: reg_pc = 0x0030; break;
			case 0xff: reg_pc = 0x0038; break;
			default:
				reg_pc = 0x0038; // just in case...
				DBERR("Alleen RST-instr. van een interrupt-device worden ondersteund!\n");
			};
			break;
		case 1:
			// RST $38 will be executed, so the total number of states is: 
	 		// 11 (rst $38) + 2 (wait states in special M1) + 1 (MSX-engine) = 14
			// TODO: dit is onze (stellig) aanname, maar klopt dit ook werkelijk? De redenatie
			// komt overeen met die van Sean met als toevoeging 1 extra state door de engine.
			emuTime += 14;
			reg_pc = 0x38;
			break;
		case 2:
			emuTime += 20;
			reg_pc = readMem16(interruptVectorOnDataBus|(reg_i<<8));
			break;
		default:
			// this should NOT happen!
			assert(false);
		}
}

inline nw_byte Z80::opcodeFetch(nw_word address) {

	// Meer info over extra M1 wait-state:
	// http://www.funet.fi/pub/msx/mirrors/hanso/service_manuals/yamahacx5myis503ts.pdf
	
	// M1 wait state (msx engine)
	emuTime++;
	refreshCounter++;

#ifndef FULL_SPEED_ON
	if (slotSelector->isIllegalAddress(address)) {
        DBERR("===============================================\n");
        DBERR("== PC: 0x%04X Executing code in empty page!\n", address);
        DBERR("== Memory dumped to crashdump.bin!\n");
        DBERR("===============================================\n");
        dumpCpuInfo();
    	dumpSlotSelection();
  		Debug::Instance()->memFileDump(readBlock,"crashdump.bin");
        assert(false);
    }
#endif
//    if (reg_pc == 0xc01e) DBERR("EXEC BOOTSECTOR!\n");

    nw_byte value = readBlock[address >> 13][address&0x1FFF];
    //DBERR("0x%04X  0x%02X\n", address, value);
    return value;
}

void Z80::abortEmulator() {

    dumpCpuInfo();
 	dumpSlotSelection();
	Debug::Instance()->memFileDump(readBlock,"abortdump.bin");
    assert(false);
}

// execute instructions until the "nextInterrupt" time, nextInterrupt is a member variable so
// DI/EI can effect it! (via emulator::setCPUInterrupt())
// 
// the amount of cycles actually executed can vary (with max. the cycles-1 of the biggest instruction)
// because an instruction is always completely executed.

// todo: optimize this method by copying the emutime into a local variable, this will require
// passing the actual emutime at every readMem/writeMem readIO/writeIO
// put the localemutime back into global emutime at exit of executeInstructions()
void Z80::executeInstructions() {

		//CheckSanity();
        // TODO: 1 instruction is always executed before the next interrupt occurs,
        // this is sometimes usefull, but not accurate.. why did we do that ?
        // see also Emulator::setCPUInterrupt

		do {
    		nw_word reg1 = 0;
    		nw_word reg2 = 0;
    		nw_word opcode = 0;
    		
    		//if (reg_pc == 0x4a33 && readMem(0x40ff)==1) Debug::Instance()->RUNTIME_INSTRUCTIONS_ON = true;
    		opcode = opcodeFetch(reg_pc);
    
    if (Z80::Instance()->emuTime > 0x00E84BE0)
    {
        //Debug::Instance()->RUNTIME_INSTRUCTIONS_ON = true;
    }
    
    #ifdef INSTRUCTIONS_ON
    		if (Debug::Instance()->RUNTIME_INSTRUCTIONS_ON) {
    			string disasm = string(Disassembler::Instance()->disAsm(reg_pc, readMem16(reg_pc), readMem16(reg_pc+2)));
    			DBERR("%04X %-15s ", reg_pc, disasm.c_str());
    		}
    #endif
    
    		++reg_pc;
            //profiler->count(opcode, readMem(reg_pc));

    		// execute opcode
			switch (opcode) {
     
    		#include "opcodes.inc"
    
    		case 0xcb:
    				switch(opcodeFetch(reg_pc++)) {
    				#include "opcodesCB.inc"
    				}
    				break;
    		case 0xed:
					opcode = opcodeFetch(reg_pc++);
    				switch(opcode) {
        			#include "opcodesED.inc"
    				}
    				break;
    		case 0xdd:
			case 0xfd:
					reg2 = 0;
					do {
						reg2++;
						reg1 = opcode;
						opcode = opcodeFetch(reg_pc++);
						assert(opcode != 0xed); // TODO: deze situatie kan voorkomen, er moet dan naar opcodeED gesprongen worden (dd en fd worden dan dus genegeerd)
						assert(reg2 == 1); // TODO: eruit halen als het een keer is voorgekomen! reg2 dan ook dus
					} while ((opcode == 0xdd) || (opcode == 0xfd));
										
					switch (reg1) {
					case 0xdd:
    					switch(opcode) {
						#define REGIX reg_ix
						#include "opcodesDD.inc"		
						}
						break;
	    			case 0xfd:
	        			switch(opcode) {
						#undef REGIX
	    				#define REGIX reg_iy
	    				#include "opcodesDD.inc"
	    				}
	    				break;
	    			default: assert(false);
					}
					break;
	    	default:
    			assert(false); // opcode > 255 ?!
    			break;
    		}

    #ifndef FULL_SPEED_ON
    
    		assert (reg_a<256);
    		assert (reg_f<256);
    		assert (reg_b<256);
    		assert (reg_c<256);
    		assert (reg_de<0x10000);
    		assert (reg_hl<0x10000);
    		assert (reg_ix<0x10000);
    		assert (reg_iy<0x10000);
    		assert (reg_pc<0x10000);
    		assert (reg_sp<0x10000);

//    		assert (reg_sp != 0x0fffe);		// duidt op een stack overflow, vaak wordt dat veroorzaakt door een eerder probleem
//    		assert (reg_sp != 1);  			// komt voor in zexall!
//    		assert (reg_sp != 2);           // jan: dat kan ook voorkomen als instructies worden getest die sp gebruiken,
                                            // als je maar zorgt dat je sp niet gebruikt op dat moment, gaat dat wel goed.
    
    		Disassembler::Instance()->triggerDisassembler();
    #endif
    
    #ifdef INSTRUCTIONS_ON
    		if (Debug::Instance()->RUNTIME_INSTRUCTIONS_ON) {
    			dumpCpuInfo();
    		}
    #endif
    } while(emuTime < nextInterrupt);      // end of while-not-next-interrupt
}

/*! \brief reads an 8 bit value from memory
 *
 *  \address 16-bit addres to read from
 *  \return the value read
 *
 *  with readMem(nw_word) it's possible to read the 
 *  current sub-slot selection from address 0xffff
 * 
 *  this method can be inlined for speed
 */
inline nw_byte Z80::readMem(nw_word address) {

    assert(address < 0x10000);
/*

    if ((slotSelector->isIllegalAddress(address)) && (address!=0xffff)) {
        DBERR("WARNING: reading empty page (addr: 0x%04x)!\n", address); 
    }
TODO: voor alle pages, en geen foutmelding geven wanneer het slot niet expanded is
    if (address == 0xfcc5) {
        if ((readBlock[address >> 13][address&0x1FFF] & 0xff) != (slotSelector->getSubSlotSelection(0) & 0xff)) {
            DBERR ("--MAJOR WARNING: SLTTBL invalid!\n");
            DBERR ("ram: 0x%02x, ss: 0x%02x\n", readBlock[address >> 13][address&0x1FFF] & 0xff, (slotSelector->getSubSlotSelection(0) & 0xff));
        }
    }
    if (address == 0xfcc8) {
        if ((readBlock[address >> 13][address&0x1FFF] & 0xff) != (slotSelector->getSubSlotSelection(3) & 0xff)) {
            DBERR ("--MAJOR WARNING: SLTTBL invalid!\n");
            DBERR ("ram: 0x%02x, ss: 0x%02x\n", readBlock[address >> 13][address&0x1FFF] & 0xff, (slotSelector->getSubSlotSelection(3) & 0xff));
        }
    }
*/
    if ((address == 0xffff) && slotSelector->possibleSubslotRead()) {
        return (~slotSelector->getSubSlotSelection(3)) & 0xff;
    } 

    // for usbInterface testing...
    if (slotSelector->usbInterface != 0) {
        if (slotSelector->usbInterface->isEnabled(address)) {
            return slotSelector->usbInterface->read(address);
        }
    }

    // for WD279X testing...
    if (slotSelector->wd279x != 0) {
        if (slotSelector->wd279x->isEnabled(address)) {
            return slotSelector->wd279x->read(address);
        }
    }    
    
    return readBlock[address >> 13][address&0x1FFF] & 0xff;
}


/*! \brief writes a 16 bit value to memory 
 *
 *  \address 16-bit address to write to
 *  \value 16-bit value that should be written
 *
 *  This method may at some time in future also include a check for writing to address 0xFFFF 
 *  that is used for selecting the sub-slot
 * 
 * this method can be inlined for speed
 */
inline nw_word Z80::readMem16(nw_word address) {

		assert(address < 0x10000);

#ifndef FULL_SPEED_ON
	/* hier ook rekening houden met FFFF */
    if (address == 0xfffe || address == 0xffff) {
	   DEBUGERROR ("16 bits reading of address FFFF not implemented !!\n");
//	   assert(false);
	}
#endif	

// Het volgende is alleen voor de 16 bit read van de nowind interface.
// De read bevindt zich op 0x2000 - 0x3fff en is gemirrord in page 2 (0x8000 - 0x9fff)
    if (((address >= 0x2000) && (address < 0x4000)) || ((address >= 0x8000) && (address < 0xa000))) {
		nw_word addressHigh = (address+1) & 0xffff;
		return readMem(address) | (readMem(addressHigh) << 8);
    }

	nw_word addressHigh = (address+1) & 0xffff;
	return readBlock[address >> 13][address&0x1FFF] |
		   (readBlock[addressHigh >> 13][addressHigh&0x1FFF] << 8);
}


/*! \brief writes a 8 bit value to memory (cpu space)
 *
 *  \address 16-bit addres to write to
 *  \value 8-bit value that should be written
 *
 *  This method also includes a check for writing to address 0xFFFF 
 *  that is used for selecting the sub-slot
 */
inline void Z80::writeMem(nw_word address, nw_byte value) {

    assert(value < 0x100);
	assert(address < 0x10000);
	
//    DBERR("Z80::writeMem 0x%04X = 0x%02X\n", address, value);
/*
    if (address == 0xfcc8) {
        DBERR("fcc8: %u ss: %u pc:0x%4x", ((value>>2)&3), slotSelector->getSelectedSubSlot(1), reg_pc);
        if (((value>>2)&3) != slotSelector->getSelectedSubSlot(1)) {
            DBERR(" paniek!!!!!!!!");
        }
        DBERR("\n");
    }
*/

//    if (address == 0xfcc4+4) {
//        DBERR("paniek! val: 0x%x\n", value);
//        if (value == 0xa8) Debug::Instance()->RUNTIME_INSTRUCTIONS_ON = true;
//        dumpCpuInfo();
//    }

    if ((address == 0xffff) && slotSelector->possibleSubslotRead()) {
		// only affect subslot-selection if the currently selected slot of page 3 is expanded!
        slotSelector->setSubSlot(value);
        return;
    }
	writeFunc[address >> 13](address, value);
/*
	if (address == 0xc0ff) {
                DBERR("val: 0x%02x\n", value);
                if (value == 0xff) return;
                if (value == 0x00) return;
                if (value == 0x89) return;
                writeIo(0xa8,255);
                writeMem(0xffff, 0xaa);
                abortEmulator();
    }
*/
}

/*! \brief writes a 16 bit value to memory (cpu space)
 *
 *  \address 16-bit addres to write to
 *  \value 16-bit value that should be written
 *
 * cpu space refers to the fact the this routine does not write to memory 
 * directly but as a real z80 would, namely accessing 64 KB at one time
 * through 4 pages of 16kb 
 *
 */
inline void Z80::writeMem16(nw_word address, nw_word value) {

    assert(value < 0x10000);
    
    
    
    
	// this will happen when sp is decremented and the result is below 0
    assert(address < 0x10000);
	nw_word addressHigh = (address+1) & 0xffff;
	writeFunc[address >> 13](address, value & 0xff);
	writeFunc[addressHigh >> 13](addressHigh, value >> 8);	
}

nw_byte	Z80::readMemPublic(nw_word address) { return readMem(address); }
nw_word	Z80::readMem16Public(nw_word address) { return readMem16(address); }
void 	Z80::writeMemPublic(nw_word address, nw_byte value) { writeMem(address, value); }	
void 	Z80::writeMem16Public(nw_word address, nw_word value) { writeMem16(address, value); }

void Z80::writeIo(nw_word port, nw_byte value) {

	assert(port < 0x100);
	assert(value < 0x100);

	switch (port) {
	case 0x2e: debugDevice->setModeRegister(value); break;
	case 0x2f: debugDevice->writeData(value); break;
		
	case 0x40: switchedPorts->writeDeviceID(value); break;
	
	case 0x41: case 0x42: case 0x43: case 0x44: 
	case 0x45: case 0x46: case 0x47: case 0x48: 
	case 0x49: case 0x4a: case 0x4b: case 0x4c: 
	case 0x4d: case 0x4e: case 0x4f: 
  		switchedPorts->writePort(port, value);
    	break;

//	case 0x7c: fmpac->writeAddress(value); break;
//	case 0x7d: fmpac->writeData(value); break;
	case 0x91: break; //DBERR("write to PRINTER PORT not implemented: " << (nw_word) value << endl); break;

	case 0x98: vdp->writeVram(value); break;
	case 0x99: vdp->writeControlRegister(value); break;
	case 0x9A: vdp->writePaletteRegister(value); break;
	case 0x9B: vdp->writeRegisterIndirect(value); break;

	case 0xa0: psg->writePort0(value); break;
	case 0xa1: psg->writePort1(value); break;
	case 0xa7: break; // pause en R800 LED op turbo-R
	
	case 0xa8: ppi->writePortA(value); break;
	case 0xa9: ppi->writePortB(value); break;
	case 0xaa: ppi->writePortC(value); break;
	case 0xab: ppi->writeControlRegister(value); break;

	case 0xb4: realTimeClock->writePortB4(value); break;
	case 0xb5: realTimeClock->writePortB5(value); break;
	
	case 0xbb: break; // Sanyo light pen

    // msx-audio en moonsound 
//    case 0xc0: msxaudio->writeAddress(value); break;
//    case 0xc1: msxaudio->writeData(value); break;
    case 0xc2: case 0xc3:
    case 0xc4: case 0xc5: 
    case 0xc6: case 0xc7: break;

	case 0xd8: kanji->writeAddressLow(value, 0); break;		// KANJI JIS1
	case 0xd9: kanji->writeAddressHigh(value, 0); break;
	case 0xda: kanji->writeAddressLow(value, 1); break;		// KANJI JIS2
	case 0xdb: kanji->writeAddressHigh(value, 1); break;
	
	case 0xdc: DBERR("writeIO (0xdc) 0x%02X (Kanji ROM, 24 dots)\n", value); break;		
	case 0xdd: DBERR("writeIO (0xdd) 0x%02X (Kanji ROM, 24 dots)\n", value); break;

	case 0xf4: f4Device->writeRegister(value); break;
// 	 	case 0xf5: system control (used to enable kanji)

	case 0xf7: DBERR("writeIO (0xf7) 0x%02X (A/V controller)\n", value); break;
	case 0xf8: DBERR("writeIO (0xf8) 0x%02X (Optional PAL A/V controller)\n", value); break;
	
	case 0xfc: mapper->setPage(0,value); break;
	case 0xfd: mapper->setPage(1,value); break;
	case 0xfe: mapper->setPage(2,value); break;
	case 0xff: mapper->setPage(3,value); break;
    
	default:
		DBERR("writeIO (0x%02X) %u not implemented\n", port, value);
		break;
	}

} 

nw_byte Z80::readIo(nw_word port) {

    assert(port < 0x100);
    
	switch (port) {
	case 0x40: return switchedPorts->readDeviceID(); break;
	
	case 0x41: case 0x42: case 0x43: case 0x44: 
	case 0x45: case 0x46: case 0x47: case 0x48: 
	case 0x49: case 0x4a: case 0x4b: case 0x4c: 
	case 0x4d: case 0x4e: case 0x4f: 
  		switchedPorts->readPort(port);
    	break;
  	
  	case 0x90: break; // printer port
	case 0x98: return vdp->readVram(); break;
	case 0x99: return vdp->readStatusRegister(); break;
	case 0xa2: return psg->readPort2(); break;
	
	//case 0xa7: pause switch on turbo-r (only in R800-mode???)
	
	case 0xa8: return ppi->readPortA(); break;
	case 0xa9: return ppi->readPortB(); break;    /* ppi == I8255, readPortB -> Keyboard::readKeyboardLine(); */
	case 0xaa: return ppi->readPortC(); break;
	case 0xab: return ppi->readControlRegister(); break;
	
	case 0xb5: return realTimeClock->readPortB5(); break;

    // msx-audio en moonsound 
//    case 0xc0: return msxaudio->readStatus(); break;
//    case 0xc1: return msxaudio->readRegister(); break;
    case 0xc2: case 0xc3:
    case 0xc4: case 0xc5: 
    case 0xc6: case 0xc7: break;

	case 0xd9: return kanji->readKanjiRom(0); break;	// KANJI JIS1
	case 0xdb: return kanji->readKanjiRom(1); break;	// KANJI JIS2

	case 0xf4: return f4Device->readRegister(); break;
	case 0xf7: DBERR("readIO (0xf7) (A/V controller)\n"); break;
	case 0xf8: DBERR("readIO (0xf8) (Optional PAL A/V controller)\n"); break;
	
	case 0xfc: return mapper->getPage(0); break;
	case 0xfd: return mapper->getPage(1); break;
	case 0xfe: return mapper->getPage(2); break;
	case 0xff: return mapper->getPage(3); break;

	case 0xba: return 255; // Sanyo light pen
	case 0xdd: DBERR("readIO 0xdd (Kanji 24-dots)\n"); break;

	default:
	    DBERR("readIO (0x%02X) not implemented!\n", port);
	}
	return 0xff;
}

void Z80::dumpCpuInfo() {

	DBERR(" AF:%04X BC:%04X DE:%04X HL:%04X",reg_af,reg_bc,reg_de,reg_hl);
    DBERR(" IX:%04X IY:%04X SP:%04X(%04X) F:",reg_ix,reg_iy,reg_sp,0x1234);//readMem16(reg_sp));

	if (reg_f & SFLAG) DBERR("s"); else DBERR(".");
	if (reg_f & ZFLAG) DBERR("z"); else DBERR(".");
	if (reg_f & 0x20) DBERR("1"); else DBERR("0");
	if (reg_f & HFLAG) DBERR("h"); else DBERR(".");
	if (reg_f & 0x08) DBERR("1"); else DBERR("0");
	if (reg_f & PFLAG) DBERR("p"); else DBERR(".");
	if (reg_f & NFLAG) DBERR("n"); else DBERR(".");
	if (reg_f & CFLAG) DBERR("c"); else DBERR(".");

    int slot = ppi->readPortA();
    for (int i=0;i<4;i++) {
        int mainSlot = (slot>>(i*2))&3;
        Uint8 subSlot = slotSelector->getSelectedSubSlot(i);
        DBERR(" %u", mainSlot);
        if (slotSelector->isExpanded(mainSlot)) {
            DBERR("%u", subSlot);
        } else {
            DBERR("x");
        }
    }
    DBERR("\n");
}

/* can return invalid information if the PPI is not yet initialized */
void Z80::dumpSlotSelection() {

    int slot = ppi->readPortA();
    DBERR("Slot selection:\n");
    for (int i=0;i<4;i++) {
        int mainSlot = (slot>>(i*2))&3;
        Uint8 subSlot = slotSelector->getSelectedSubSlot(i);
        DBERR(" page%u (%u", i, mainSlot);
        if (slotSelector->isExpanded(mainSlot)) {
            DBERR("-%u", subSlot);
        }
        DBERR(") %s\n", slotSelector->getDeviceName(i).c_str());
    }
}

void Z80::dumpPages() {

	string filename("dumppages.bin");

	/* delete the existing? file */
	ofstream ofs_delete(filename.c_str(),ios::trunc);
	ofs_delete.close();

	ofstream ofs(filename.c_str(),ios::binary);
	if (ofs.fail()) {
		DBERR("Error opening file %s!\n", filename.c_str());
	}

    for (unsigned int b=0;b<8;b++) {
        DBERR("readBlock[%u]: %u\n", b, readBlock[b]);
        ofs.write((char *)readBlock[b],8*1024);
    }
	ofs.close();
}

void Z80::saveState() {
	mapper->saveState();
}

void Z80::loadState() {
	mapper->loadState();
}

nw_word Z80::getSP() {
    return reg_sp;   
}

void Z80::hijackBdos()
{
    long seconds = SDL_GetTicks() / 1000;

    switch (reg_c)
    {
    case 2:
        // BDOS function 2 (C_WRITE) - Console output
        // C=2, E=ASCII character
        DBERR("%c", reg_e);
        break;
    case 9:
    {
        // BDOS function 9 (C_WRITESTR) - Output string
        // C=9, DE=address of string
        nw_word de = reg_de;
        while (1)
        {
            nw_byte c = READMEM(de++);
            if (c == 0x0A)
            {
                DBERR(" (time: %us)\n", seconds);
                continue;
            }
            if (c == 0x24) break;
            DBERR("%c", c);
        }
        break;
    }
    default:
        // no nothing
        break;
    }
}
void Z80::setupBdosEnv(const char* filename)
{
    writeIo(0xFC, 3);
    writeIo(0xFD, 2);
    writeIo(0xFE, 1);
    writeIo(0xFF, 0);
    
    writeIo(0xa8, 0xff);
    WRITEMEM(0xffff, 0xaa);

    // load file at 0x100
    ifstream romfile(filename, ios::binary);
    bool failed = romfile.fail();
    assert(!failed);

    romfile.seekg(0, ios::end);
    Uint32 fileSize = romfile.tellg();
    DBERR("loaded %s (%u bytes)\n", filename, fileSize);

    char* temp = new char[fileSize];
    romfile.seekg(0);
    romfile.read(temp, fileSize);
    romfile.close();

    // dont read files into memory directly
    // memory locations are not guaranteed to be '1 byte' locations
    for (Uint16 i = 0;i < fileSize;i++)
    {
        nw_byte value = temp[i] & 0xff;    // todo: find out why this fails without the & 0xff!!
        WRITEMEM(0x100+i, value);
    }
    delete temp;

    // patch bdos call 0x0005
    WRITEMEM(0x0005, 0xED);
    WRITEMEM(0x0006, 0x0E);
    WRITEMEM(0x0007, 0xC9);
    
    // set initial SP 
    reg_sp = 0xc800;
    
    // set pc
    reg_pc = 0x0100;
}
