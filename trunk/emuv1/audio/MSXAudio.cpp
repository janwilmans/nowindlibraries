#include "stdio.h"
#include "Debug.h"
#include "AudioMixer.h"
#include "Emulator.h"
#include "cpu/Z80.h"
#include "MSXAudio.h"

#include <math.h>

using namespace std;

MSXAudio * MSXAudio::Instance() {

		/* implies singleton class */
		static MSXAudio deInstantie;
		return &deInstantie;
}
  
MSXAudio::MSXAudio() : emuTime(Z80::Instance()->emuTime) {

	DBERR("MSXAudio constructor...\n");

    deviceName = "SND_MSXAUDIO";

    registerAddress = 0; // TODO: check!
    statusRegister = 0x06;  // OPL2??? OPL3 = 0x00
    statusMask = 255;
    
    startAddress = 0;
    stopAddress = 0;
    
    record = false; // TODO check
    memoryAccess = false; // TODO check

/* TODO
       reset timer1
       reset timer2
       reset keyboard out
       and more..... marked with * in pdf
*/

    keyOn[0] = false;
	DBERR("MSXAudio constructor...finished\n");
}

MSXAudio::~MSXAudio() {

		// destructor
		DBERR("MSXAudio destroyed.\n");
}

void MSXAudio::writeAddress(nw_byte value) {

     registerAddress = value;
}

void MSXAudio::writeData(nw_byte value) {
     
    DBERR("MSXAudio: reg[0x%0x] = 0x%0x\n", registerAddress, value);

    switch (registerAddress) {
    case 0x00: break;
    case 0x01: break;   // LSI TEST
    case 0x02: timer1 = value; break;
    case 0x03: timer2 = value; break;
    case 0x04:
        if (value & 0x80) {
            // reset all flags
            statusRegister = 0x06;
        } else {
            statusMask = (value & 0x78) ^ 255; // MASKT1, MASKT2, MASKEOS, MASKBUFRDY
//            if (value & 0x01) interruptTime1 = emuTime + ((256 - timer1) * 80 * 3579545); 
//            if (value & 0x02) interruptTime2 = emuTime + ((256 - timer2) * 320 * 3579545);
        }
        break;
    case 0x05: break; // keyboard in (read only)
    case 0x06: break; // keyboard out (not supported)
    case 0x07:
        record = false;
        memoryAccess = false;
        
        if (value & 0x01) {
            // andere bits ook!
            if (value & 0x40) record = true;
            if (value & 0x20) memoryAccess = true;
            

        } else {
            // reset ???
            
        }
        break;
    case 0x09: startAddress = (startAddress & 0xff00) | value; break;
    case 0x0a: startAddress = (startAddress & 0x00ff) | (value << 8); break;
    case 0x0b: stopAddress = (stopAddress & 0xff00) | value; break;
    case 0x0c: stopAddress = (stopAddress & 0x00ff) | (value << 8); break;
    case 0x0d: break; // prescale low
    case 0x0e: break; // prescale high
    case 0x0f: writeADPCM(value); break;
    
    case 0x18: break; // I/O control
    case 0x19: break; // I/O data     
    case 0x1a: break; // PCM data
    
        // AM, VIB, EGT, KSR
    case 0x20: frequencyMultiplier1[0] = value & 15; break;
    case 0x21: frequencyMultiplier1[1] = value & 15; break;
    case 0x22: frequencyMultiplier1[2] = value & 15; break;
    case 0x23: frequencyMultiplier2[0] = value & 15; break;
    case 0x24: frequencyMultiplier1[1] = value & 15; break;
    case 0x25: frequencyMultiplier1[2] = value & 15; break;

    case 0xa0: fNumber[0] = (fNumber[0] & 0x0300) | value; break;

    case 0xb0: 
        fNumber[0] = (fNumber[0] & 0x00ff) | ((value & 2) << 8);
        keyOn[0] = (value & 0x20) == 0x20 ? true:false;
        break;
    
     default:
         DBERR("unsupported MSX Audio register: 0x%0x\n", registerAddress);
     }
}

nw_byte MSXAudio::readStatus() {
        
    DBERR("MSXAudio readStatus\n");
    return (statusRegister & statusMask);
}

nw_byte MSXAudio::readRegister() {
        
     nw_byte value;
     
     switch (registerAddress) {
     case 0x05: value = 0xff; break; // keyboard in
     case 0x0f: DBERR("read ADPCM data!\n"); break;
     case 0x19: DBERR("read I/O data!\n"); break;
     case 0x1a: DBERR("read PCM data!\n"); break;
     default:
         DBERR("WARNING: register is write only!\n");
         value = 0xff; // TODO check!
     }
     return value;
}

void MSXAudio::writeADPCM(nw_byte value) {

    if (!record) DBERR("ADPCM write (0x%02x) gedaan, maar read geselecteerd! (TODO!!!)\n", value);
    
    if (memoryAccess) {
//        memoryPointer++ = value;
        // memoryPointer &= 0xffff ??????
//        status |= BUFFERREADY;
        // if (memoryPointer == stopAddress) status |= EOS;
    } else {
        DBERR("deze data moet naar soundBuffer\n");
    }
}

void MSXAudio::startup() {

    initialize();
    DBERR("MSXAudio startup\n");

    audioBuffer = AudioMixer::Instance()->getBuffer(1);
    audioBufferIndex = (BUFFER_DEPTH*audioBufferSize)/2;      // beginpunt halverwege buffer
    
    DBERR("MSXAudio got audioBufferSize: %u\n", audioBufferSize);
    DBERR("MSXAudio got audioSampleRate: %u\n", audioSampleRate);
}

void MSXAudio::sync(unsigned int) {

	emuTimeType timeSpan = emuTime - lastUpdateEmuTime;
	unsigned int samples = (unsigned int)(((float)timeSpan * (float)audioSampleRate) / (float)3579545);
    updateBuffer(samples);

	bytesGenerated -= audioBufferSize;
}

void MSXAudio::updateBuffer(unsigned int samples) {

	AudioMixer::Lock();
//	unsigned int sampleTime = Z80::Instance()->cpuFrequency / audioSampleRate;

	// modulator wave
//	double multiM = multiplyFactor[instrumentData[inst][0] & 15];
//	double freqM = (FNumber[0] * octave[0] * 50000 * multiM) / (double)262144;
//	unsigned int periodM = 3579545 / freqM;

//	if (periodM == 0) periodM = 1;
	
//	double mi = modulationIndex[instrumentData[0][2] & 0x3f];
//	DBERR("db: " << int(instrumentData[0][2] & 0x1f) << endl);
//	DBERR("mi: " << mi << endl);
	
//	DBERR("modulation Index: " << mi << endl);
//	double volume = 16384/2;
signed int value = 0;

	for (unsigned int i=0;i<samples;i++) {
	
//	    float f = i;
        value = 0;    //(sin(f/samples))*16384;
		
        assert(audioBufferIndex <= maxBufferSize);
		if (audioBufferIndex == maxBufferSize) audioBufferIndex = 0;
        audioBuffer[audioBufferIndex++] = value;
	}
	lastUpdateEmuTime += (unsigned int)((float)samples * (float)3579545 / (float)audioSampleRate); // nauwkeuriger
	AudioMixer::Unlock();
}
