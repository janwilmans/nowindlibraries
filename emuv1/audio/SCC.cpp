//! SCC.cc

#include "stdio.h"
#include "cpu/Z80.h"
#include "AudioMixer.h"
#include "Debug.h"
#include "SCC.h"
#include "Emulator.h"

using namespace std;
//using namespace fastdelegate;

#define NUM_SOUNDS 2

SCC * SCC::Instance() {

		/* implies singleton class */
		static SCC deInstantie;
		return &deInstantie;
}

SCC::SCC() : emuTime(Z80::Instance()->emuTime) {

	DBERR("SCC constructor...\n");
    idString = "SCC";
    
    deviceName = "SND_SCC";
	
	// TODO: kan dit misschien (bij een reset) beter in initialize()? 
    enable = 0;
    deform = 0;
    
    memory = new nw_byte[8192];
    memset(memory, 0xff, 8192);
    
	DBERR("SCC constructor...finished\n");
}

SCC::~SCC() {
		DBERR("SCC destructor\n");
		delete [] memory; // this is not the MemoryDevice:memory variable (which is read-only)
		DBERR("SCC destroyed.\n");
}

void SCC::startup() {

    initialize();
    DBERR("SCC startup\n");

    audioBuffer = AudioMixer::Instance()->getBuffer(5);
    audioBufferIndex = 2*audioBufferSize;
    
    //DBERR("SCC got audioBufferSize: " << dec << audioBufferSize << endl);
    //DBERR("SCC got audioSampleRate: " << dec << audioSampleRate << endl);
}

void SCC::sync(unsigned int) {

	emuTimeType timeSpan = emuTime - lastUpdateEmuTime;
	unsigned int samples = (unsigned int)(((float)timeSpan * (float)audioSampleRate) / (float)3579545);
    
	updateBuffer(samples);
}

// TODO: SCC kan ook gelezen worden, maar niet in het hele bereik!!!
// Bovendien is het gemirrord.

#define PERIODLOW(period, wavePointer)		\
	period = (period & 0x0f00) | value;		\
	if (deform & 0x20) wavePointer = 0	

#define PERIODHIGH(period, wavePointer)					\
	period = (period & 0x00ff) | ((value & 15) << 8);	\
	if (deform & 0x20) wavePointer = 0
	
void SCC::write(nw_word address, nw_byte value) {
    
    assert(address >= 0x8000 && address < 0xa000);

	emuTimeType timeSpan = emuTime - lastUpdateEmuTime;
	unsigned int samples = (unsigned int)(((float)timeSpan * (float)audioSampleRate) / (float)3579545);
    updateBuffer(samples);
	
    // scc zelf zit tussen 0x9800 t/m 9fff
	// TODO: mirroring!
	
	switch (address) {
	case 0x9880: case 0x9890: PERIODLOW (period1, wavePointer1); break;
	case 0x9881: case 0x9891: PERIODHIGH(period1, wavePointer1); break;
	case 0x9882: case 0x9892: PERIODLOW (period2, wavePointer2); break;
	case 0x9883: case 0x9893: PERIODHIGH(period2, wavePointer2); break;
	case 0x9884: case 0x9894: PERIODLOW (period3, wavePointer3); break;
	case 0x9885: case 0x9895: PERIODHIGH(period3, wavePointer3); break;
	case 0x9886: case 0x9896: PERIODLOW (period4, wavePointer4); break;
	case 0x9887: case 0x9897: PERIODHIGH(period4, wavePointer4); break;
	case 0x9888: case 0x9898: PERIODLOW (period5, wavePointer5); break;
	case 0x9889: case 0x9899: PERIODHIGH(period5, wavePointer5); break;

	case 0x988a: case 0x989a: volume1 = (value & 15) << 2; break;
	case 0x988b: case 0x989b: volume2 = (value & 15) << 2; break;
	case 0x988c: case 0x989c: volume3 = (value & 15) << 2; break;
	case 0x988d: case 0x989d: volume4 = (value & 15) << 2; break;
	case 0x988e: case 0x989e: volume5 = (value & 15) << 2; break;
	
	case 0x988f: case 0x989f: enable = value; break;
/*
    case 0x8f: case 0x9f:
    	diff = channelEnable ^ value;
    	channelEnable = value;
    	// ZOU DIT IN WERKELIJKHEID GEBEUREN???
    	// nu roteren de channels overigens gewoon door, ook als ze uitstaan.
    	// Het heeft nu dus geen zin om de pointers op 0 te zetten. (wel als er 
    	// tijdens het afspelen eerst gecheck wordt of een channel aanstaat)
    	// hoe is dit in werkelijkheid?
    	if (diff & 0x01) wavePointer[0] = 0;
    	if (diff & 0x02) wavePointer[1] = 0;
    	if (diff & 0x04) wavePointer[2] = 0;
    	if (diff & 0x08) wavePointer[3] = 0;
    	if (diff & 0x10) wavePointer[4] = 0;    	
    	break;
*/  
    default:
		if (address < 0x9000) break;
		if (address < 0x9800) {
			if ((value & 0x3f) != 0x3f) {
				// activate romMapper
				Z80::Instance()->writeFunc[4] = lastWriteFunc;
				Z80::Instance()->readBlock[4] = lastReadBlock;
			}
			break;
		}
		if (address < 0x9880) {
			memory[address - 0x8000] = value;
			break;
		}
		if (address >= 0x98e0 && address < 0x9900) {
			deform = value;
			memory[address - 0x8000] = value; // eigenlijk de hele range....
			DBERR("DEFORMATION write: %x\n", value);
		}
	}
}

void SCC::updateBuffer(unsigned int samples) {

	AudioMixer::Lock();
	unsigned long AYticks = (Z80::Instance()->cpuFrequency/audioSampleRate);
 
	bool enable1 = (enable & 0x01) && (period1 !=0) ? true:false;
	bool enable2 = (enable & 0x02) && (period2 !=0) ? true:false;
	bool enable3 = (enable & 0x04) && (period3 !=0) ? true:false;
	bool enable4 = (enable & 0x08) && (period4 !=0) ? true:false;
	bool enable5 = (enable & 0x10) && (period5 !=0) ? true:false;

	for (unsigned int i=0;i<samples;i++) {
	
        signed int value = 0;
		
		if (enable1) {
			periodCount1 += AYticks;
			if (periodCount1 > period1) {
                wavePointer1 += (periodCount1 / period1);
                wavePointer1 &= 31;
                periodCount1 %= period1;
			}
			value += memory[0x9800 - 0x8000 + wavePointer1 + 0] * volume1;
		}
		if (enable2) {
			periodCount2 += AYticks;
			if (periodCount2 > period2) {
                wavePointer2 += (periodCount2 / period2);
                wavePointer2 &= 31;
                periodCount2 %= period2;
			}
			value += memory[0x9800 - 0x8000 + wavePointer2 + 32] * volume2;
		}
		if (enable3) {
			periodCount3 += AYticks;
			if (periodCount3 > period3) {
                wavePointer3 += (periodCount3 / period3);
                wavePointer3 &= 31;
                periodCount3 %= period3;
			}
			value += memory[0x9800 - 0x8000 + wavePointer3 + 64] * volume3;
		}
		if (enable4) {
			periodCount4 += AYticks;
			if (periodCount4 > period4) {
                wavePointer4 += (periodCount4 / period4);
                wavePointer4 &= 31;
                periodCount4 %= period4;
			}
			value += memory[0x9800 - 0x8000 + wavePointer4 + 96] * volume4;
		}
		if (enable5) {
			periodCount5 += AYticks;
			if (periodCount5 > period5) {
                wavePointer5 += (periodCount5 / period5);
                wavePointer5 &= 31;
                periodCount5 %= period5;
			}
			value += memory[0x9800 - 0x8000 + wavePointer5 + 96] * volume5;
		}
		
        //if (value > 16383) value = 16383;
        //else if (value < -16384) value = -16384;
        //value += 16384;
		        
        assert(audioBufferIndex <= maxBufferSize);
		if (audioBufferIndex == maxBufferSize) audioBufferIndex = 0;
        audioBuffer[audioBufferIndex++] = value;
//        DEBUGERROR(value<<endl);
	}
	lastUpdateEmuTime += (unsigned int)((float)samples * (float)3579545 / (float)audioSampleRate); // nauwkeuriger
	AudioMixer::Unlock();
}


void SCC::install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address) {
    
    DBERR("SCC::install\n");
	unsigned int startBlocks = address / BLOCKSIZE;
	offset = address;
	
    DBERR("  (%u-%u) in block: %u\n", mainSlot, subSlot, startBlocks);
	layout[mainSlot][subSlot][startBlocks].bind(this, &SCC::activate);
}

void SCC::activate(unsigned int block) {

    //DBERR("SCC::activate for block " << block << endl);
	// TODO: nasty hack... kan vast mooier.
    lastWriteFunc = Z80::Instance()->writeFunc[4];
    lastReadBlock = Z80::Instance()->readBlock[4];
    
	Z80::Instance()->readBlock[block] = memory;
    Z80::Instance()->writeFunc[block].bind(this, &SCC::write);
}
