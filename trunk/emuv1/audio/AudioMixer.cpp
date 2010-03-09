// AudioMixer.cpp

#include <string>
#include "AudioMixer.h"
#include "AudioDevice.h"
#include "cpu/Z80.h"

//JAN: WTF ??
//#define _ASSERT_H

#include "Debug.h"

using namespace std;

SDL_mutex * AudioMixer::mixerMutex = SDL_CreateMutex();

AudioMixer * AudioMixer::Instance() {

		/* implies singleton class */
		static AudioMixer deInstantie;
		return &deInstantie;
}
  
AudioMixer::AudioMixer() {

    bufferDepth = BUFFER_DEPTH;
	DBERR("AudioMixer constructor...\n");

	DBERR("AudioMixer constructor...finished\n");
}

void AudioMixer::initialize() {

	SDL_AudioSpec *desired, *obtained;
    desired = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
    obtained = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
    memset(desired, 0, sizeof(SDL_AudioSpec));
    memset(obtained, 0, sizeof(SDL_AudioSpec));

/*

44100 Hz * 16 Bit audio = 82,2 Kbps 
this means 16kb/82200 = 0,2 seconds of audiobuffer (and so 0,2 latency!)

It would be good to have about a 1024 bytes buffer 
L = B / O) 
L = 1024 / 82200 = 0,0125 seconds (100th of a second delay)
-
*/    
    
    desired->freq = 44100;
// requesting a unsigned audio-format will not work on mac
    desired->format = AUDIO_S16;
#ifdef __APPLE__
    desired->format = AUDIO_S16;
#endif
    desired->samples = 1024; //4*2048; // size in samples (1024 works good)
    desired->userdata = NULL;
	desired->channels = 1;
	desired->callback = &AudioMixer::staticAudioCallback;

	DBERR("desired samples: %u\n", desired->samples);

	// open audio device
	if ( SDL_OpenAudio(desired, obtained) < 0 ) {
		fprintf(stderr, "AudioMixer: Unable to open audio: %s\n", SDL_GetError());
		exit(1);
	} else {
		DBERR("AudioMixer: OpenAudio succesfull\n");
	    DBERR("desired samples: %u\n", desired->samples);

		if ((desired->freq != obtained->freq) || 
			(desired->format != obtained->format) ||
			(desired->samples != obtained->samples) ||		
			(desired->channels != obtained->channels)) {
			DBERR("AudioMixer: WARNING, obtained and desired audio parameters are different!\n");
		}
		DBERR("  got sampleRate: %u (requested: %u)\n", obtained->freq, desired->freq);
		DBERR("  format: "); determineAudioFormat(obtained->format);
		DBERR(" ("); determineAudioFormat(desired->format);
		DBERR(")\n");
		DBERR("  got audioBuffer in samples: %u (requested: %u)\n", obtained->samples, desired->samples);
		DBERR("  got audioBuffer in bytes: %u (requested: %u)\n", obtained->size, desired->size);
		DBERR("  got audioBuffer length: %u ms\n", ((1000*obtained->samples)/obtained->freq));
		DBERR("  got channels: %i (requested: %i)\n", (int)obtained->channels, (int)desired->channels);
		DBERR("  silenceValue: %i\n", (int)obtained->silence);
	}

	sampleRate = obtained->freq;
	audioBufferSize = obtained->samples;		// size in samples
	outputAudioBufferSize = obtained->size;		// size in bytes
	sampleSize = outputAudioBufferSize/audioBufferSize;		//size of one sample in bytes

	free(desired);
	free(obtained);

    outputBuffer = new signed int[audioBufferSize*BUFFER_DEPTH];
    outputBufferReadPointer = audioBufferSize*(BUFFER_DEPTH-1);
    outputBufferWritePointer = 0;
    memset(outputBuffer, 0, outputAudioBufferSize);

	channelCount = 0;
    volumeDeler = 1;
    bufferCount = 0;
    deviceCount = 0;
    
    lastFilterIn = 0;
    lastFilterOut = 0;
	
    string filename = "rawaudio.out";
    string fullpath = Debug::Instance()->getPath() + filename;
    rawout = new fstream(fullpath.c_str(), ios::out | ios::binary | ios::trunc);
    if (rawout->fail()) {
		DBERR("Could not open raw audio output file: %s\n", filename.c_str());
    }   	    
}

void AudioMixer::determineAudioFormat(Uint16 format) {
	switch (format) {
	case AUDIO_U8: DBERR("Unsigned 8bit"); break;
	case AUDIO_S8: DBERR("Signed 8bit"); break;
	case AUDIO_U16: DBERR("Unsigned 16bit"); break;
	case AUDIO_S16: DBERR("Signed 16bit"); break;
	default: DBERR("Unknown\n");
	}
}

void AudioMixer::Lock()
{
	SDL_LockMutex(mixerMutex); 
}

void AudioMixer::Unlock()
{
	SDL_UnlockMutex(mixerMutex); 
}

void AudioMixer::AudioInterrupt(emuTimeType scheduledTime)
{
    //DBERR("=AudioInterrupt, outputBufferWritePointer: %u\n", outputBufferWritePointer);

    SDL_LockAudio();
    for (unsigned int d=0;d<deviceCount;d++) 
    {
        devices[d]->sync(audioBufferSize);	
    }

    static unsigned int totalBuffers = 0;
    totalBuffers++;

    unsigned long long totalsamplesInEmutime = totalBuffers;
    totalsamplesInEmutime = totalsamplesInEmutime * audioBufferSize * Z80::Instance()->cpuFrequency;
    unsigned int nextTime = totalsamplesInEmutime / sampleRate;

    //DBERR("totalBuffers: %u, sampleRate %u, audioBufferSize: %u, nextTime: %u\n", totalBuffers, sampleRate, audioBufferSize, nextTime);
    Emulator::Instance()->scheduleAudioInterrupt(true, nextTime);

    // audioBufferSize is in samples (not bytes)
	for (unsigned int i=0;i<audioBufferSize;i++) {

		assert(bufferCount > 0); // is bufferCount == 0 no audio device was registered

		// jaw: simple addition will definately not work for unsigned audio-formats (so right now unsigned is not supported)
		// and even for signed audio, I don't think this is right!

        // only support for max 8 devices for now...
		unsigned int value = buffers[0][i];
		switch (bufferCount) {
		case 8: value += buffers[7][i];
		case 7: value += buffers[6][i];		
		case 6: value += buffers[5][i];	
		case 5: value += buffers[4][i];	
		case 4: value += buffers[3][i];	
		case 3: value += buffers[2][i];				
		case 2: value += buffers[1][i];
		case 1: break;
		default: assert(false);		// this method is fast but does not scale automatically
		}

        outputBuffer[outputBufferWritePointer+i] = value;
        // write value
	}

    for (unsigned int d=0;d<deviceCount;d++) 
    {
        devices[d]->audioDone();
    }

    outputBufferWritePointer += audioBufferSize;
    if (outputBufferWritePointer == (audioBufferSize*BUFFER_DEPTH)) 
    {
        outputBufferWritePointer = 0;
    }

    SDL_UnlockAudio();
}

// - audioCallback mag nooit sync op devices aanroepen!
// - audioCallback mag alleen het al complete buffer copyeren
void AudioMixer::audioCallback(Uint8 *stream, int len) {
		
    assert(len == outputAudioBufferSize);		// this appears to be true, not sure we should rely on it though
	// DBERR("audiocallback for outputBufferReadPointer: %u, outputBufferWritePointer: %u\n", outputBufferReadPointer, outputBufferWritePointer);

    static unsigned int calls = 0;
    calls++;

    static unsigned int previous = 0;
    unsigned int emutime = Z80::Instance()->emuTime;

    // (Buffer Length In EmuTime)
    unsigned int bliet = audioBufferSize * Z80::Instance()->cpuFrequency / sampleRate;

    //DBERR("[%05d] CB emutime: %u, dt: %09u, bliet: %09u\n", calls, emutime, emutime-previous, bliet);
    previous = emutime;

  	for (unsigned int i=0;i<audioBufferSize;i++) {

        Sint16 *str = (Sint16 *) stream;
  		*str = outputBuffer[outputBufferReadPointer+i];
    	stream+=2;
	}

    //DBERR("outputBufferReadPointer: %u\n", outputBufferReadPointer);

    outputBufferReadPointer += audioBufferSize;
    if (outputBufferReadPointer == (audioBufferSize*BUFFER_DEPTH))
    {
        outputBufferReadPointer = 0;
    }
}

unsigned int * AudioMixer::getBuffer(unsigned int channels) {
           	       
        Uint32 * newBuffer = new Uint32[bufferDepth*audioBufferSize];
        for (Uint32 i=0;i<bufferDepth*audioBufferSize;i++) {
            newBuffer[i] = zeroValue*channels;
        }

        buffers[bufferCount] = newBuffer;
        bufferCount++;        
		channelCount += channels;
	    DBERR("AudioMixer: %u channel(s) provided! (total: %u)\n", channels, channelCount);
        return newBuffer;
}

unsigned int AudioMixer::getSampleRate() {
    
        return sampleRate;
}

unsigned int AudioMixer::getBufferSize() {
    
        return audioBufferSize;
}

void AudioMixer::staticAudioCallback(void *, Uint8 *stream, int len) {

    // let op: len is signed !
    
    int s = SDL_GetTicks();
    AudioMixer::Instance()->audioCallback(stream, len);
    int e = SDL_GetTicks();
}

void AudioMixer::volume(unsigned int vol) {

  //  volumeDeler = 1+(vol/4);
}

void AudioMixer::addDevice(AudioDevice *device) {
    
    device->startup();
    devices[deviceCount++] = device;
    DBERR("AudioMixer: AudioDevice added! (devices: %u)\n", deviceCount);
}

AudioMixer::~AudioMixer() {

    for (int i=0;i<bufferCount;i++) {
        delete [] buffers[i];
    }
        
    unsigned int fileSize = rawout->tellp(); // current write-pointer
    rawout->close();
    delete rawout;

    string filename = "rawaudio.out";
    string fullpath = Debug::Instance()->getPath() + filename;
    rawout = new fstream(fullpath.c_str(), ios::in | ios::binary);
    if (rawout->fail()) {
		DBERR("Could not open raw audio input file for conversion: %s\n", filename.c_str());
    }   	

    if (fileSize > 0)
    {    
	    char * samples = new char[fileSize];
	    rawout->read(samples,fileSize);
    	rawout->close();
    
	    filename = "rawaudio.wav";
    	fullpath = Debug::Instance()->getPath() + filename;

	    FILE *aFile = fopen(fullpath.c_str(),"wb");
    	writeWav(aFile, samples, fileSize, 44100);
	    fclose(aFile);
	    delete [] samples;
	} 

	delete rawout;   	
	// destructor
	DBERR("AudioMixer destroyed.\n");
}


/*
   Write an WAVE sound file
   Only do one channel, only support 16 bit.
   Supports any (reasonable) sample frequency
   Little/big endian independent!
*/
void AudioMixer::writeWav(FILE *fptr,char *samples,long nsamples,int nfreq)
{
   int i;
   unsigned long totalsize,bytespersec;
   nsamples = nsamples / 2; //bytesPerSample;

   /* Write the form chunk */
   fprintf(fptr,"RIFF");
   totalsize = 2 * nsamples + 36; //bytesPerSample * nsamples + 36;
   fputc((totalsize & 0x000000ff),fptr);        /* File size */
   fputc((totalsize & 0x0000ff00) >> 8,fptr);
   fputc((totalsize & 0x00ff0000) >> 16,fptr);
   fputc((totalsize & 0xff000000) >> 24,fptr);
   fprintf(fptr,"WAVE");
   fprintf(fptr,"fmt ");                        /* fmt_ chunk */
   fputc(16,fptr);                              /* Chunk size */
   fputc(0,fptr);
   fputc(0,fptr);
   fputc(0,fptr);
   fputc(1,fptr);                               /* Format tag - uncompressed */
   fputc(0,fptr);
   fputc(1,fptr);                               /* Channels */
   fputc(0,fptr);
   fputc((nfreq & 0x000000ff),fptr);            /* Sample frequency (Hz) */
   fputc((nfreq & 0x0000ff00) >> 8,fptr);
   fputc((nfreq & 0x00ff0000) >> 16,fptr);
   fputc((nfreq & 0xff000000) >> 24,fptr);
   bytespersec = 2 * nfreq; //bytesPerSample * nfreq;
   fputc((bytespersec & 0x000000ff),fptr);      /* Average bytes per second */
   fputc((bytespersec & 0x0000ff00) >> 8,fptr);
   fputc((bytespersec & 0x00ff0000) >> 16,fptr);
   fputc((bytespersec & 0xff000000) >> 24,fptr);
   fputc(2,fptr);                               /* Block alignment */
   fputc(0,fptr);
   //fputc(bytesPerSample*8,fptr);                /* Bits per sample */
   fputc(2*8,fptr);                /* Bits per sample */
   fputc(0,fptr);
   fprintf(fptr,"data");
   //totalsize = bytesPerSample * nsamples;
   totalsize = 2 * nsamples;
   fputc((totalsize & 0x000000ff),fptr);        /* Data size */
   fputc((totalsize & 0x0000ff00) >> 8,fptr);
   fputc((totalsize & 0x00ff0000) >> 16,fptr);
   fputc((totalsize & 0xff000000) >> 24,fptr);

   //if (bytesPerSample == 2) {
   if (2 == 2) {
       unsigned int j = 0;
       /* Write the data */
       for (i=0;i<nsamples;i++) {
          Uint8 byte1 = samples[j++];
          Uint8 byte2 = samples[j++];
          fputc((byte1 & 0x00ff),fptr);
          fputc((byte2 & 0x00ff),fptr);
       }
    } else {
          /* Write the data */
          for (i=0;i<nsamples;i++) {
             Uint8 byte1 = samples[i];
             fputc((byte1 & 0x00ff),fptr);
          }
    }
}
