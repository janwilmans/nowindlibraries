//! YM2413.h
#ifndef YM2413_H
#define YM2413_H

#include <iostream>
#include <string>

#include "msxtypes.h"
#include "AudioDevice.h"

class AudioMixer;

/*! 
 * The YM2413 class emulates the MSX-MUSIC
 */
class YM2413 : public AudioDevice { 

private:

		YM2413();
		emuTimeType 	&emuTime;
		emuTimeType		lastUpdateEmuTime;
		nw_byte			address;
		
		// YM2413 registers
		bool			rhythmMode;
		unsigned int	rhythm;
		unsigned int	FNumber[9];
		double			octave[9];
		bool			key[9];
		bool			sustain[9];
		unsigned int	volume[9];
		unsigned int	instrument[9];
		unsigned int	multiCarrier[9];		

		void			generateTables();
		void			updateBuffer(unsigned int);
		
		unsigned int	phase[9];
		unsigned int	phaseM[9];
		signed int		sinusTable[1024];
		double			modulationIndex[64];
		
		void			updateUserInstrument();
		unsigned int	periodC;
		double			keyScaleLevel[4][8][16];
	
public:
		static YM2413 * Instance();
		~YM2413();
		void			reset();

        // AudioDevice interface		
		void 		startup();
		void 		sync(unsigned int);

		void 		writeAddress(nw_byte value);
		void 		writeData(nw_byte value);
};

#endif

