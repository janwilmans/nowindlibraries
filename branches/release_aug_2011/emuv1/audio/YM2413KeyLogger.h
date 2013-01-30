//! YM2413.h
#ifndef YM2413_H
#define YM2413_H

#include <iostream>
#include <string>
#include <list>

#include "SDL.h"
#include "SDL/SDL_audio.h"
#include "msxtypes.h"
//#include "AudioDevice.h"

//TODO: implement the FM-sound generation and AudioDevice interface

#define FMPAC_SOUND_OFF

/*! 
 * The YM2413 class emulates the MSX-MUSIC, currently it only logs the music that 
 * should have been played
 */
class YM2413 {
    
static const nw_byte USE_ENVELOPE = 16;
static const unsigned int soundBufferSize = 16000;
static const unsigned int sampleRate = 44100;

private:

		YM2413();
		emuTimeType     &emuTime;
		double          lastClockPulseEmutime;
		double          emutimeElapsed;
		double          lastCallBackEmutime;		

		unsigned int	currentRegister;
		unsigned int	reg[0x40];

		struct chanStruct {
    		unsigned int 	freq;
   			unsigned int	oct;
			unsigned int	key;
			unsigned int	sustain;
			unsigned int	volume;
			unsigned int	stroke;

		} chan[10]; 

		chanStruct lastChan[10]; 

		std::string chanNote[10]; 
		std::string lastNote[10]; 

		unsigned int drum[5];
  		unsigned int drumStroke;
		unsigned int AYbeat;

		std::fstream 	*mbmStream;

		typedef struct {
			unsigned short	length;
			unsigned char	id;
			unsigned char	voiceData[16*9];
			unsigned char	instrumentList[16];
			unsigned char	instrumentVolList[32];
			unsigned char	channelChipSettings[10];
			unsigned char	startTempo;
			unsigned char	sustainMsxaudio;

			char			trackName[41];
			unsigned char	startInstrumentsMsxAudio[9];
			unsigned char	startInstrumentsMsxMusic[9];
			unsigned char	msxMusicOriginalInstrumentData[6*8];
			unsigned char	msxMusicOriginalProgNummer[6];
			char			sampleKitName[8];
			
			unsigned char	drumSetupMsxMusicPsg[15];
			unsigned char	drumVolumesMsxMusic[3];
			unsigned char	drumFrequenciesMsxMusic[20];
			unsigned char	startDetune[9];
			unsigned char	loopPosition;
			
		} mbmHeaderStruct;

		std::list<char *> noteLines;
  		nw_byte * lastLine;
	
public:
		static YM2413 * Instance();
		~YM2413();

		void writeAddress(nw_byte value);
		void writeData(nw_byte value);
		std::string getNote(unsigned int);
		int getNoteNr(unsigned int freq);

		bool noteMatch(unsigned int freq, unsigned int fixedFreq);

		void    clockPulse();

		Uint8   soundBuffer[soundBufferSize];
		
		unsigned int    soundBufferIndex;

		void dumpChannelsBeat();
		void dumpChannels();
		void dump9Channels();
		void dump6ChannelsAndRhythm();
};

#endif

