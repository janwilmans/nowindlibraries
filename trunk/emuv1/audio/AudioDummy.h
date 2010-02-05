#ifndef AUDIODUMMY_H
#define AUDIODUMMY_H

#include <iostream>
#include <fstream>
#include <string>

#include "msxtypes.h"
#include "AudioDevice.h"

class AudioMixer;

/*!
 * The AudioDummy class is a "perfect" sound generator to test the AudioMixer
 */
class AudioDummy : public AudioDevice {

private:

		AudioDummy();
		std::fstream * fs;
		unsigned int startupFase;
		void createOneBuffer();

public:
		static AudioDummy * Instance();

		void    clockPulse();

        // AudioDevice interface		
		void startup();
		void sync(unsigned int);
		~AudioDummy();
			
};

#endif

