//! PPISound.h
#ifndef PPISOUND_H
#define PPISOUND_H

#include <iostream>

#include "msxtypes.h"
#include "AudioDevice.h"

class AudioMixer;

/*!
 * The PPISound class emulates the 1-bit soundport (mainly used for key click)
 */
class PPISound : public AudioDevice {

private:
		PPISound();
		
		emuTimeType     &emuTime;
		emuTimeType     lastUpdateEmuTime;
		bool			portIsHigh;
		
		// TODO: niet globaal binnen class!
		float filterIn;
		float filterOut;

		
public:
    
		static PPISound * Instance();

		void	updateBuffer(unsigned int);
		
        // AudioDevice interface		
		void	startup();
		void	sync(unsigned int);
		void	write(bool);

		~PPISound();

};

#endif

