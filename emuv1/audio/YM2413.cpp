//! YM2413.cc

#include <math.h>

#include "stdio.h"
#include "cpu/Z80.h"
#include "AudioMixer.h"
#include "Debug.h"
#include "YM2413.h"
#include "Emulator.h"

#define PI 3.1415926535897932384626433832795

using namespace std;

unsigned int instrumentData[16][8] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },	// user instrument
	{ 0x61, 0x61, 0x1e, 0x17, 0xf0, 0x7f, 0x00, 0x17 },	// violin
	{ 0x13, 0x41, 0x16, 0x0e, 0xfd, 0xf4, 0x23, 0x23 },	// guitar
	{ 0x03, 0x01, 0x9a, 0x04, 0xf3, 0xf3, 0x13, 0xf3 },	// piano
	{ 0x11, 0x61, 0x0e, 0x07, 0xfa, 0x64, 0x70, 0x17 },	// flute
	{ 0x22, 0x21, 0x1e, 0x06, 0xf0, 0x76, 0x00, 0x28 },	// clarinet
	{ 0x21, 0x22, 0x16, 0x05, 0xf0, 0x71, 0x00, 0x18 },	// oboe
	{ 0x21, 0x61, 0x1d, 0x07, 0x82, 0x80, 0x17, 0x17 },	// trumpet
	{ 0x23, 0x21, 0x2d, 0x16, 0x90, 0x90, 0x00, 0x07 },	// organ
	{ 0x21, 0x21, 0x1b, 0x06, 0x64, 0x65, 0x10, 0x17 },	// horn
	{ 0x21, 0x21, 0x0b, 0x1a, 0x85, 0xa0, 0x70, 0x07 },	// synthesizer
	{ 0x23, 0x01, 0x83, 0x10, 0xff, 0xb4, 0x10, 0xf4 },	// harpsichord
	{ 0x97, 0xc1, 0x20, 0x07, 0xff, 0xf4, 0x22, 0x22 },	// vibraphone
	{ 0x61, 0x00, 0x0c, 0x05, 0xc2, 0xf6, 0x40, 0x44 },	// synthesizer bass
	{ 0x01, 0x01, 0x56, 0x03, 0x94, 0xc2, 0x03, 0x12 },	// acoustic bass
	{ 0x21, 0x01, 0x89, 0x03, 0xf1, 0xe4, 0xf0, 0x23 },	// electric guitar
};

double multiplyFactor[16] = {
	.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15
};


YM2413 * YM2413::Instance() {

		/* implies singleton class */
		static YM2413 deInstantie;
		return &deInstantie;
}

YM2413::YM2413() : emuTime(Z80::Instance()->emuTime) {

	DBERR("YM2413 constructor...\n");
	deviceName = "SND_YM2413";
	generateTables();
	reset();
	DBERR("YM2413 constructor...finished\n");
}

YM2413::~YM2413() {

    	DBERR("YM2413 destructor\n");

		DBERR("YM2413 destroyed.\n");
}

void YM2413::reset() {
	// TODO: reset all opll registers to zero
	for (int i=0;i<8;i++) instrumentData[0][i] = 0;
	for (int i=0;i<8;i++) {
		phase[0] = 0;
	}
}

void YM2413::generateTables() {
	
	// modulation index (TODO: check)
	for (int i=0;i<64;i++) {
		double db = 0.75 * i;
		modulationIndex[i] = pow(10, db/20); 
//		DBERR("mIndex: " << modulationIndex[i] << endl);
	}
	
	// key scale
	double atTable[16] = { // attenuation at 3dB and octave 7
		 0.000,  9.000, 12.000, 13.875, 15.000, 16.125, 16.875, 17.625,
		18.000,	18.750, 19.125, 19.500, 19.875, 20.250, 20.625,	21.000
	};
	double rate[4] = { 0.0, 1.5, 3.0, 6.0 };
	
	for (int ksl=0;ksl<4;ksl++) {
//		DEBUGERROR(dec << rate[ksl] << "dB/oct\n");
		for (int octave=0;octave<8;octave++) {
			for(int fNumber=0;fNumber<16;fNumber++) {
				double att = (atTable[fNumber] - (7 - octave) * 3.0) * (rate[ksl] / 3.0);
				if (att < 0) att = 0;
				keyScaleLevel[ksl][octave][fNumber] = att;
//				DEBUGERROR(att << " ");
			}
//		DEBUGERROR(endl);
		}
	}
}

void YM2413::writeAddress(nw_byte value) {
	
	// TODO: kunnen poort 7c en 7d ook uitgelezen worden???
	address = value;
}

void YM2413::writeData(nw_byte value) {

	// update audio stream until current emuTime
	emuTimeType timeSpan = emuTime - lastUpdateEmuTime;
	unsigned int samples = (unsigned int)(((float)timeSpan * (float)audioSampleRate) / (float)3579545);
	updateBuffer(samples);
	
	unsigned int channel = address & 15; // TODO: wat gebeurt er voor channel > 8?
	switch (address) {

	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x05: case 0x06: case 0x07:
		instrumentData[0][address & 15] = value;
		break;
	case 0x0e:
		rhythmMode = (value & 0x20) != 0;
		rhythm = value & 0x1f;
		break;
	case 0x0f: break;	// TEST register (TODO: doet dit nog iets?)
	
	// FNumber (b7-0)
	case 0x10: case 0x11: case 0x12:
	case 0x13: case 0x14: case 0x15:
	case 0x16: case 0x17: case 0x18:
		FNumber[channel] = (FNumber[channel] & 0x100) | value;
		break;
	
	// sustain, key(on/off), block(octave), FNumber(b8)
	case 0x20: case 0x21: case 0x22:
	case 0x23: case 0x24: case 0x25:
	case 0x26: case 0x27: case 0x28:
		FNumber[channel] = (FNumber[channel] & 0xff) | ((value & 1) << 8);
		DBERR("set octaaf op: %u\n", (value>>1)&7);
		octave[channel] = double(1 << ((value >> 1) & 7));
		key[channel] = (value & 0x10) != 0;
		sustain[channel] = (value & 0x20) != 0;
		phase[channel] = 0; // TODO: verkomt noise bij aanslag, maar check! (gebeurt dit alleen bij en key-on?)
		phaseM[channel] = 0;
//		keyScaleNo = value & 15;
		if (value & 0x10) {
			DBERR("keyON channel: %u\n", channel);
		}
		break;

	// instrument(b7-4), volume(b3-b0)
	case 0x30: case 0x31: case 0x32:
	case 0x33: case 0x34: case 0x35:
		volume[channel] = value & 15;
		instrument[channel] = value >> 4;
		break;		
		
	case 0x36:
		volume[6] = value & 15;
		instrument[6] = value >> 4;
		// TODO: volume (base drum) = value & 15;
		break;	
		
	case 0x37:
		volume[7] = value & 15;
		instrument[7] = value >> 4;
		// TODO: volume (snare drum) = value & 15;
		// TODO: volume (high hat) = value >> 4;
		break;
	case 0x38:
		volume[8] = value & 15;
		instrument[8] = value >> 4;
		// TODO: volume (top cymbal) = value & 15;
		// TODO: volume (tom-tom) = value >> 4;
		break;
		
	default:
		DBERR("MSX-MUSIC register 0x%04X not supported!\n", address);
		break;
	}
	updateUserInstrument();
}

void YM2413::updateUserInstrument() {
	
	unsigned int cpuFrequency = Z80::Instance()->cpuFrequency;
//	unsigned int sampleTime = cpuFrequency / audioSampleRate;
	unsigned int fSam = cpuFrequency / 72;
	
	int instr = instrument[0];
	
	// carrier
	double multiC = multiplyFactor[instrumentData[instr][1] & 15];
	double freqC = (FNumber[0] * octave[0] * multiC * fSam) / 262144;
	periodC = (unsigned int)(cpuFrequency / freqC);        /* converting double to unsigned int ?! */
	if (periodC == 0) periodC = 1; // TODO: check!
	
	DBERR("FREQ: %u\n", freqC);
	DBERR("multiC: %u\n", multiC);
	DBERR("FN: %u\n", FNumber[0]);
	DBERR("oct: %u\n", octave[0]);

/*
	// TODO: check of R=0, dan is RATE ook 0
	if (instrumentData[instr][0] & 0x10) {
		keyScaleOffset = keyScaleNo;
	} else {
		keyScaleOffset = keyScaleNo >> 2;
	}
	attackStep
*/	
}



void YM2413::updateBuffer(unsigned int samples) {

	AudioMixer::Lock();
	unsigned int sampleTime = Z80::Instance()->cpuFrequency / audioSampleRate;

	// modulator wave
//	double multiM = multiplyFactor[instrumentData[inst][0] & 15];
//	double freqM = (FNumber[0] * octave[0] * 50000 * multiM) / (double)262144;
//	unsigned int periodM = 3579545 / freqM;

//	if (periodM == 0) periodM = 1;
	
//	double mi = modulationIndex[instrumentData[0][2] & 0x3f];
//	DBERR("db: " << int(instrumentData[0][2] & 0x1f) << endl);
//	DBERR("mi: " << mi << endl);
	
//	DBERR("modulation Index: " << mi << endl);
//	double volume = 16384/2;

	for (unsigned int i=0;i<samples;i++) {
	
        signed int value = 0;
		if (key[0]) {
			//value += sinusTable[(phase[c] * 1024)/period];
			//value += 16000; // maak er unsigned van....TODO: moet anders!

			double ph = (double)phase[0]*2*PI/periodC;
//			double phm = (double)phaseM[0]*2*PI/periodM;
//			value = (signed int)(volume * sin(ph + mi*sin(phm)));
value = (signed int)(16384 * sin(ph)); // grond frequentie

//			value = (signed int)(16000 * sin((double)(phase[c]/period)*2*PI));

			phase[0] += sampleTime;
			phase[0] %= periodC;
//			phaseM[0] += sampleTime;
//			phaseM[0] %= periodM;
		}
		
        assert(audioBufferIndex <= maxBufferSize);
		if (audioBufferIndex == maxBufferSize) audioBufferIndex = 0;
        audioBuffer[audioBufferIndex++] = value;
	}
	lastUpdateEmuTime += (unsigned int)((float)samples * (float)3579545 / (float)audioSampleRate); // nauwkeuriger
	AudioMixer::Unlock();
}

void YM2413::startup() {

    initialize();
    DBERR("YM2413 startup\n");

    audioBuffer = AudioMixer::Instance()->getBuffer(9);
    audioBufferIndex = 2*audioBufferSize;
}

void YM2413::sync(unsigned int) {

	// DBERR("YM2413 CALLBACK!\n");
	emuTimeType timeSpan = emuTime - lastUpdateEmuTime;
	unsigned int samples = (unsigned int)(((float)timeSpan * (float)audioSampleRate) / (float)3579545);
    
	updateBuffer(samples);
}

/*

rate = 1 -> keyScale Rate is sneller
	heeft iig invloed op attack rate (AR)
	dus waarschijnlijk ook op DR


 */
