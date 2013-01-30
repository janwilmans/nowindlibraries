//! MSXAudio.h
#ifndef MSXAUDIO_H
#define MSXAUDIO_H

#include "msxtypes.h"
#include "AudioDevice.h"

class AudioMixer;

class MSXAudio : public AudioDevice {

private:

		MSXAudio();
		nw_byte           registerAddress;
		nw_byte           externalMemory[256 * 1024];
		nw_byte           statusRegister;     // IRQ, FT1, FT2, EOS, BUFRDY, 1, 1, PCMBUSY
		nw_byte           statusMask;
		
        nw_byte           timer1;
        nw_byte           timer2;
        emuTimeType       interruptTime1;
        emuTimeType       interruptTime2;

		nw_word           startAddress;
		nw_word           stopAddress;
        bool              record;
        bool              memoryAccess;

        void              writeADPCM(nw_byte);
		
// FM synthesis
        nw_byte           frequencyMultiplier1[9];
        nw_byte           frequencyMultiplier2[9];
        nw_byte           fNumber[9];
        bool              keyOn[9];
		
public:
		static MSXAudio * Instance();
		~MSXAudio();
		
		emuTimeType&    emuTime;
        emuTimeType     lastUpdateEmuTime;
        signed int bytesGenerated;

		void 	writeAddress(nw_byte);
		void 	writeData(nw_byte);
		nw_byte readStatus();
		nw_byte readRegister();

		void    updateBuffer(unsigned int);

        // AudioDevice interface		
		void 	startup();
		void 	sync(unsigned int);
			
};

#endif

