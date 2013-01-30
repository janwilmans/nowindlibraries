//! AudioDevice.cpp 

#include <string>
#include "audio/AudioDevice.h"
#include "audio/AudioMixer.h"
#include "Debug.h"

using namespace std;

AudioDevice::AudioDevice() {
	
}

void AudioDevice::initialize() {

	DBERR(deviceName.c_str());
    DBERR(" initializing...\n");

    audioBufferSize = AudioMixer::Instance()->getBufferSize();
    audioSampleRate = AudioMixer::Instance()->getSampleRate();

    DBERR("init audioBufferSize = %u\n",audioBufferSize);

	maxBufferSize = BUFFER_DEPTH*audioBufferSize;
}

AudioDevice::~AudioDevice() {
    
	DBERR("AudioDevice: %s\n", deviceName.c_str());
    DBERR(" destruction...\n");
}
