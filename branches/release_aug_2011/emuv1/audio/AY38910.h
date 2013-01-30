//! AY38910.h
#ifndef AY38910_H
#define AY38910_H

#include <iostream>
#include <string>

#include "msxtypes.h"
#include "AudioDevice.h"

class AudioMixer;

#define toneEnableA 0x01
#define toneEnableB 0x02
#define toneEnableC 0x04

#define noiseEnableA 0x08
#define noiseEnableB 0x10
#define noiseEnableC 0x20

#define ENVHOLD 255

static const nw_byte regMask[16] = {
	0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0x1F, 0xFF,
	0x1F, 0x1F, 0x1F, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF
};

/*!
 * The AY38910 class is the PSG emulation AudioDevice
 */
class AY38910 : public AudioDevice {

enum { AFINE, ACOARSE, BFINE, BCOARSE, CFINE, CCOARSE, NOISE, ENABLE,
	   AAMPL, BAMPL, CAMPL, ENVFINE, ENVCOARSE, ENVSHAPE, PORTA, PORTB };

static const nw_byte USE_ENVELOPE = 16;

private:
		std::fstream	* csvFile;

		AY38910();
		nw_byte reg[16];  
		nw_byte regSelect;

		emuTimeType     &emuTime;
		emuTimeType		lastUpdateEmuTime;

		unsigned int    bufferOffsetCopy;
		unsigned int    bufferLevel;
		
		unsigned int   	periodA;
		unsigned int   	periodB;
		unsigned int   	periodC;
		unsigned int   	periodNoise;
		unsigned int   	periodEnvelope;

		unsigned int	countA;
		unsigned int   	countB;
		unsigned int	countC;
		unsigned int	countNoise;
		unsigned int	countEnvelope;
		unsigned int	envelope;
		unsigned int	envelopeState;
		unsigned int	volumeE;
		
		unsigned int    outA;
		unsigned int	outB;
		unsigned int	outC;
		unsigned int 	outNoise;
		unsigned int	noiseA;
		unsigned int	noiseB;
		unsigned int	noiseC;
		
		unsigned int	volumeA;
		unsigned int	volumeB;
		unsigned int	volumeC;
		unsigned int	logVolume[16];
		
		unsigned int	pseudoRnd;
		unsigned int    overflowCounter;
		
		inline Uint32   makeWave();
		void			precalcVolume();

		unsigned int    samplesInBuffer;
        unsigned int    lastSync;

public:
		static AY38910 * Instance();

		void 	writePort0(nw_byte value);
		void 	writePort1(nw_byte value);
		nw_byte readPort2();

		void updateBuffer(unsigned int);

        // AudioDevice interface		
		void 	startup();
		void 	sync(unsigned int);
        void    audioDone();
		~AY38910();
			
};

#endif

