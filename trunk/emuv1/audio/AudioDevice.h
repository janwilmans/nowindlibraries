//! AudioDevice.h
#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include "SDL.h"
#include "SDL_audio.h"
#include <string>

/*!
 * The AudioDevice class is an abstract class that all audio-devices should inherit from 
 */
class AudioDevice {

protected:
		Uint32 *      audioBuffer;
		Uint32        audioBufferIndex;
		Uint32        audioBufferSize;
		Uint32        audioSampleRate;
		Uint32        maxBufferSize;
		
		std::string     deviceName;   /*!< Identifies (names) the device,  for gui and debugging */

public:
		AudioDevice();
		/*! Gets the audioBufferSize and audioSampleRate from the AudioMixer, fills internal var's etc. */
		virtual void initialize();
		/*! Requests buffer from the AudioMixer and clears it */
		virtual void startup() = 0;
		/*! Called by AudioDevice just before the channel buffers are mixed 
		 *
		 * \param offset Position in an audiobuffer to sync to
		 */
		virtual void sync(Uint32 offset) = 0;
        virtual void audioDone() {}
		virtual ~AudioDevice();
};

#endif
