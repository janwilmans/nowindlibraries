//! SCC.h
#ifndef SCC_H
#define SCC_H

#include <iostream>
#include <string>

#include "msxtypes.h"
#include "AudioDevice.h"

typedef fastdelegate::FastDelegate2<nw_word, nw_byte> writeDelegate;   // returns void 

class AudioMixer;

/*!
 * The SCC class emulates the Konami-soundchip
 */
class SCC : public MemoryDevice, public AudioDevice {

private:
		SCC();
		
		nw_byte         *memory;				// warning the MemoryDevice::memory variable is shadowed here
		emuTimeType     &emuTime;
		emuTimeType     lastUpdateEmuTime;

 		unsigned long	periodCount1;
 		unsigned long	periodCount2;
 		unsigned long	periodCount3;
 		unsigned long	periodCount4;
 		unsigned long	periodCount5;

		unsigned int    wavePointer1;
		unsigned int    wavePointer2;
		unsigned int    wavePointer3;
		unsigned int    wavePointer4;
		unsigned int    wavePointer5;

		unsigned int	logVolume[16];

		// internal SCC registers
		unsigned int    period1;
		unsigned int    period2;
		unsigned int    period3;
		unsigned int    period4;
		unsigned int    period5;
		
		unsigned int    volume1;
		unsigned int    volume2;
		unsigned int    volume3;
		unsigned int    volume4;
		unsigned int    volume5;
		
		unsigned int    enable;
		unsigned int    deform;

        writeDelegate   lastWriteFunc;
		const nw_byte	*lastReadBlock;
		
public:
    
		static SCC * Instance();

		void       updateBuffer(unsigned int);
		void	   writeRegister(nw_word, nw_byte);
//		nw_byte    readRegister(nw_word);
		
        // AudioDevice interface		
		void startup();
		void sync(unsigned int);

        // MemoryDevice interface		
        void write(nw_word address, nw_byte value);
        void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
        void activate(unsigned int block);
		
		virtual ~SCC();

};

#endif

