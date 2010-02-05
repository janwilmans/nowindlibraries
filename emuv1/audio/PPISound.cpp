//! PPISound.cc

#include "stdio.h"
#include "cpu/Z80.h"
#include "AudioMixer.h"
#include "Debug.h"
#include "PPISound.h"
#include "Emulator.h"

using namespace std;

PPISound * PPISound::Instance() {

		/* implies singleton class */
		static PPISound deInstantie;
		return &deInstantie;
}

PPISound::PPISound() : emuTime(Z80::Instance()->emuTime) {

	DBERR("PPISound constructor...\n");
    
    deviceName = "SND_PPISound";
    portIsHigh = false;
	
	filterIn = 0;
	filterOut = 0;
    
	DBERR("PPISound constructor...finished\n");
}

PPISound::~PPISound() {
	DBERR("PPISound destructor\n");

	// destructor
	DBERR("PPISound destroyed.\n");
}

void PPISound::startup() {

    initialize();
    DBERR("PPISound startup\n");

    audioBuffer = AudioMixer::Instance()->getBuffer(1);
    audioBufferIndex = 2*audioBufferSize;
    
    DBERR("PPISound got audioBufferSize: %u\n", audioBufferSize);
    DBERR("PPISound got audioSampleRate: %u\n", audioSampleRate);
}

void PPISound::write(bool value) {

	emuTimeType timeSpan = emuTime - lastUpdateEmuTime;
	unsigned int samples = (unsigned int)(((float)timeSpan * (float)audioSampleRate) / (float)3579545);
	updateBuffer(samples);	
	
	portIsHigh = value;
}

void PPISound::sync(unsigned int) {

/* TODO:
	Als de output NUL is en er ook geen verandering in het geluid is geweest, doe dan niets!
*/
	emuTimeType timeSpan = emuTime - lastUpdateEmuTime;
	unsigned int samples = (unsigned int)(((float)timeSpan * (float)audioSampleRate) / (float)3579545);
	updateBuffer(samples);
}


void PPISound::updateBuffer(unsigned int samples) {

	const float gain = 0.997;

	AudioMixer::Lock();
	for (unsigned int i=0;i<samples;i++) {
	
//        signed int value;
		filterOut = (filterOut * gain) - filterIn;

		if (portIsHigh)	{
			filterIn = 16384;
		} else {
			filterIn = 0;
		}
		        
		filterOut += filterIn;
		
		if (filterOut > 32767) filterOut = 32767;
		else if (filterOut < -32768) filterOut = -32768;
		
		if (audioBufferIndex == (2*audioBufferSize)) audioBufferIndex = 0;
        audioBuffer[audioBufferIndex++] = (signed int)filterOut;

	}
	lastUpdateEmuTime += (unsigned int)((float)samples * (float)3579545 / (float)audioSampleRate); // nauwkeuriger
	AudioMixer::Unlock();
}
