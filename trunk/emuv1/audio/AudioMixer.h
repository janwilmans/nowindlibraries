//! AudioMixer.h

#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <iostream>
#include <string>

#include "msxtypes.h"
#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_mutex.h"

#define BUFFER_DEPTH 3

/*!
 * The AudioDevice class mixes all audio-streams of all audio-devices added to it
 */
class AudioDevice;

typedef struct bufferStatus {
    bool newData;   
} bufferStatus;

class AudioMixer {

private:
        unsigned int        callbackCounter;
        Uint32              startTicks;

        unsigned int        sampleRate;

		AudioMixer();
		static void         staticAudioCallback(void *unused, Uint8 *stream, int len);
		unsigned int        soundBufferIndex;
        void                audioCallback(Uint8 *stream, int len);
		void 				determineAudioFormat(Uint16);
        unsigned int		zeroValue;
        unsigned int		silentValue;
        unsigned int		channelCount;
        
        unsigned int        lastTime;
        unsigned int        volumeDeler;
        std::fstream        *rawout;
        
        // DC filter
        float				lastFilterIn;
        float				lastFilterOut;

		unsigned int        audioBufferSize; 
		unsigned int        outputAudioBufferSize;
		unsigned int        sampleSize;
		static SDL_mutex *  mixerMutex;

        signed int *        outputBuffer;
        unsigned int        outputBufferReadPointer;
        unsigned int        outputBufferWritePointer;
        

public:
		static AudioMixer * Instance();
        void                addDevice(AudioDevice *);
        void                initialize();

        unsigned int *      getBuffer(unsigned int);    
        unsigned int        getBufferSize();
        unsigned int        getSampleRate();

		static void Lock();
		static void Unlock();

		~AudioMixer();

        unsigned int      bufferDepth;
        unsigned int *    buffers[20];          /*!< no Uint8, because these buffers contain multiple already added channels */
												/*!< Max. 20 audioDevices, TODO: make this dynamic? */
		int               bufferCount;

        AudioDevice *     devices[20];
		unsigned int      deviceCount;
        
		void writeWav(FILE *fptr,char *samples,long nsamples,int nfreq);
		
		void volume(unsigned int);
        void AudioInterrupt(emuTimeType scheduledTime);
};

#endif

