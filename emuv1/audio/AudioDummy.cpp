#include "stdio.h"
#include "Debug.h"
#include "AudioMixer.h"
#include "Emulator.h"
#include "cpu/Z80.h"
#include "AudioDummy.h"

using namespace std;

AudioDummy * AudioDummy::Instance() {

		/* implies singleton class */
		static AudioDummy deInstantie;
		return &deInstantie;
}
  
AudioDummy::AudioDummy() {

	DBERR("AudioDummy constructor...\n");

    deviceName = "SND_DUMMY";
    
    string filename = "amelie.wav.stripped";
    string fullpath = Debug::Instance()->getPath() + filename;
    fs = new fstream(fullpath.c_str(), ios::in | ios::binary);
    if (fs->fail()) {
		DBERR("Could not open raw audio data: %s\n", filename.c_str());
    }    

	DBERR("AudioDummy constructor...finished\n");
}

AudioDummy::~AudioDummy() {

	// destructor
	delete fs;
	DBERR("AY38910 destroyed.\n");
}

void AudioDummy::startup() {

    initialize();
    DBERR("AudioDummy startup\n");

    audioBuffer = AudioMixer::Instance()->getBuffer(1);
    audioBufferIndex = (BUFFER_DEPTH*audioBufferSize)/2;      // beginpunt halverwege buffer
    
    DBERR("AudioDummy got audioBufferSize: %u\n", audioBufferSize);
    DBERR("AudioDummy got audioSampleRate: %u\n", audioSampleRate);
}

void AudioDummy::sync(unsigned int) {

    if (startupFase == 0) startupFase++;      
    createOneBuffer();
}


void AudioDummy::createOneBuffer() {

/*
    startupFase:
     0 = first audio callback did not occur yet 
     1 = first audio callback occurred, initialize buffers (1ms)
     2 = playing...
*/
    if (startupFase == 0) return;
    if (startupFase == 1) startupFase++;

    audioBufferIndex = 0;
	unsigned int value = 0;
	
	char * buf = new char[audioBufferSize];
	memset(buf,0x80,audioBufferSize);
	fs->read(buf,audioBufferSize);

	for (unsigned int i=0;i<audioBufferSize;i++) {    
        value = buf[i] & 0xff;
        audioBuffer[audioBufferIndex++] = value;
	}
	delete buf;
}
