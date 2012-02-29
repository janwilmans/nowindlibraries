#include "stdio.h"
#include "Debug.h"
#include "AudioMixer.h"
#include "Emulator.h"
#include "cpu/Z80.h"
#include "AY38910.h"

using namespace std;

AY38910 * AY38910::Instance() {

		/* implies singleton class */
		static AY38910 deInstantie;
		return &deInstantie;
}
  
AY38910::AY38910() : emuTime(Z80::Instance()->emuTime) {

	DBERR("AY38910 constructor...\n");

    deviceName = "SND_AY38910";

	/* init some values */
	for (int i=0;i<16;i++) reg[i] = 255;
	regSelect = 0;

	precalcVolume();
	
	countA = countB = countC = 1;
	outA = outB = outC = 1;
	volumeA = volumeB = volumeC = 0;
    countNoise = 0;
   	countEnvelope = 0;
   	envelopeState = ENVHOLD;

    periodNoise = 1;
    periodEnvelope = 1;
	periodA = periodB = periodC = 1;
	noiseA = noiseB = noiseC = 1;

    pseudoRnd = 0xaaaaaaaa;		// non-zero value
    bufferOffsetCopy = 0;
    bufferLevel = 1;            // het eerste buffer is leeg geinitializeerd
    
	csvFile = new fstream("audio.csv", ios::out);

	DBERR("AY38910 constructor...finished\n");
}

AY38910::~AY38910() {

	csvFile->close();
	delete csvFile;
	// destructor
	DBERR("AY38910 destroyed.\n");
}

void AY38910::startup() {

    initialize();
    DBERR("AY38910 startup\n");

    audioBuffer = AudioMixer::Instance()->getBuffer(3);
    audioBufferIndex = 0;
	samplesInBuffer = 0;
}

void AY38910::precalcVolume() {

	 double a = 32768 / 3;
	 for (int i=15;i>0;i--) {
	 	 logVolume[i] = (unsigned int)a;
	 	 //DBERR("vol[%u] %u\n", i, logVolume[i]);
	 	 a *= 0.70711;
	 }
	 logVolume[0] = 0;
}

void AY38910::writePort0(nw_byte value) {

#ifdef PSG_COMMANDS_ON
	if (value > 15) {
		DBERR("PSG register %u does not exist!\n", value);
	}
#endif
	regSelect = value & 0x0F;
}


void AY38910::writePort1(nw_byte value) {

#ifdef PSG_COMMANDS_ON
	DBERR("PSG WRITE #%u: %u\n", registerSelect, value);
#endif

    float AYticks = ((float) Z80::Instance()->cpuFrequency) / audioSampleRate;
    
    unsigned int timeSinceLastsync = emuTime - lastSync;
    if (emuTime < lastSync)
    {
        timeSinceLastsync = emuTime + (0xffffffff-lastSync);
    }
    unsigned int samples = unsigned int(timeSinceLastsync / AYticks);
    
    //DBERR("samples: %u\n", samples);
    updateBuffer(samples);	// update buffer until emuTime (now)
	
	reg[regSelect] = value & regMask[regSelect];
    signed int prevPeriod;
	switch (regSelect) {

	case AFINE:
	case ACOARSE:
 	    prevPeriod = periodA;
		periodA = reg[AFINE] + (reg[ACOARSE]<<8);
		periodA *= 16;
		countA = periodA - (prevPeriod - countA);
		break;
		
	case BFINE:
	case BCOARSE:
		prevPeriod = periodB; 
		periodB = reg[BFINE] + (reg[BCOARSE]<<8);
		periodB *= 16;
		countB = periodB - (prevPeriod - countB);
		break;
		
	case CFINE:
	case CCOARSE:
		prevPeriod = periodC;
		periodC = reg[CFINE] + (reg[CCOARSE]<<8);
		periodC *= 16;
		countC = periodC - (prevPeriod - countC);
		break;

	case NOISE:
		value &= 31;
 	    if (value == 0) value++; // tested on real msx
 	    periodNoise = value * 32; // doc says 16x, 32x sounds more real
		break;
		
	case ENABLE:  break;

	case AAMPL: volumeA = (value & USE_ENVELOPE) ? volumeE : logVolume[value & 15]; break;
	case BAMPL: volumeB = (value & USE_ENVELOPE) ? volumeE : logVolume[value & 15]; break;
	case CAMPL: volumeC = (value & USE_ENVELOPE) ? volumeE : logVolume[value & 15]; break;

	case ENVFINE:
	case ENVCOARSE:
 	    // changing the envelope period does not reset envelope volume
		periodEnvelope = reg[ENVFINE] + (reg[ENVCOARSE]<<8);
		periodEnvelope *= 32;
		if (periodEnvelope == 0) periodEnvelope++; // TODO!
		break;
	case ENVSHAPE:
 	    // changing the envelope shape sets the envelope volume to the
 	    // first value of the new shape (either 0 or 15)
 	    envelopeState = value & 15;
		envelope = (value & 0x04) ? 0:15;
		volumeE = logVolume[envelope];
		if (reg[AAMPL] & USE_ENVELOPE) volumeA = volumeE;
		if (reg[BAMPL] & USE_ENVELOPE) volumeB = volumeE;
		if (reg[CAMPL] & USE_ENVELOPE) volumeC = volumeE;
		break;
		
	// hiervoor hoeft het audiobuffer niet geupdate te worden...	
	case PORTA: //if (reg[ENABLE] & 0x40) Console::Instance()->writePSGPortA(value); break;
	case PORTB: //if (reg[ENABLE] & 0x80) Console::Instance()->writePSGPortB(value); break;

	default:
		break;
	}
}

nw_byte AY38910::readPort2() {

	switch (regSelect) {
	case PORTA: reg[PORTA] = 255;//(reg[ENABLE] & 0x40) ? 255 : Console::Instance()->readPSGPortA(); break;
	case PORTB: reg[PORTB] = 255;//(reg[ENABLE] & 0x80) ? 255 : Console::Instance()->readPSGPortB(); break;
	default: break;
	}
	return reg[regSelect];
}

inline Uint32 AY38910::makeWave() {

		Uint32 value;
		unsigned int AYticks = Z80::Instance()->cpuFrequency / audioSampleRate;
		
		countNoise += AYticks;
		if (countNoise > periodNoise) {
   			countNoise %= periodNoise;
			
			// random bit generator (lineair freeback shift register)
			// TODO: check whether ^0x86000001 produces the maximum cycle
			// http://www.maxim-ic.com/appnotes.cfm/appnote_number/1743
	  		pseudoRnd >>= 1;
			if (pseudoRnd & 1) { pseudoRnd ^= 0x86000001; outNoise = 1; }
			else outNoise = 0;
			assert (pseudoRnd != 0);
			
			noiseA = (reg[ENABLE] & noiseEnableA) ? 1:outNoise;
			noiseB = (reg[ENABLE] & noiseEnableB) ? 1:outNoise;
			noiseC = (reg[ENABLE] & noiseEnableC) ? 1:outNoise;
  		}
		
		if (periodA != 0) countA += AYticks;
		if ((periodA == 0) || (reg[ENABLE] & toneEnableA)) {
			outA = 1; 
		} else {
			if (countA > periodA) {
   			     countA %= periodA;
				 outA ^= 1;
			}
		}	

		if (periodB != 0) countB += AYticks;
		if ((periodB == 0) || (reg[ENABLE] & toneEnableB)) {
			outB = 1; // channel disabled
		} else {
			if (countB > periodB) {
   			     countB %= periodB;
				 outB ^= 1;
			}
		}

		if (periodC != 0) countC += AYticks;
		if ((periodC == 0) || (reg[ENABLE] & toneEnableC)) {
			outC = 1; // channel disabled
		} else {
			if (countC > periodC) {
   			     countC %= periodC;
				 outC ^= 1;
			}
		}

		value = ((outA & noiseA) * volumeA) +
			  	((outB & noiseB) * volumeB) +
			  	((outC & noiseC) * volumeC);

		countEnvelope += AYticks;
		if (countEnvelope > periodEnvelope) {
  		    countEnvelope %= periodEnvelope;
  		    
			switch (envelopeState) {
   			case 0x00: case 0x01:
		 	case 0x02: case 0x03:
		 	case 0x09:
		 	    envelope--;
		 	    if (envelope == 0) envelopeState = ENVHOLD;
		 	    break;
   			case 0x04: case 0x05:
		 	case 0x06: case 0x07:
		 	case 0x0f:
	 	 	    envelope++;
	   			if (envelope == 16) {
			   	    envelopeState = ENVHOLD;
			   	    envelope = 0;
			    }
			    break;
		    case 0x08:
 	 		    envelope--;
 	 		    envelope &= 15;
 	 		    break;
		    case 0x0a:
		 	    envelope--;
		 	    if (envelope == 0) envelopeState = 0x0e;
		 	    break;
    		case 0x0b:
			    if (envelope == 0) {
		 	   	    envelopeState = ENVHOLD;
		 	   	    envelope = 15;
   			 	} else {
 	  	   		    envelope--;
		  	   	}
		  	   	break;
	  	   	case 0x0c:
		 	    envelope++;
		 	    envelope &= 15;
		 	    break;
	 	    case 0x0d:
	 	 	    envelope++;
	 	 	    if (envelope == 15) envelopeState = ENVHOLD;
	 	 	    //DBERR("env 0x0d - envelope: " << dec << envelope << endl);
   			 	break;
		 	case 0x0e:
		 	    envelope++;
		 	    if (envelope == 15) envelopeState = 0x0a;
		 	    break;
 			case ENVHOLD:
			    break;
   			}

   			// misschien hier volume checks doen? hoeft minder vaak dan

   			volumeE = logVolume[envelope];
   			if (reg[AAMPL] & USE_ENVELOPE) volumeA = volumeE;
   			if (reg[BAMPL] & USE_ENVELOPE) volumeB = volumeE;
   			if (reg[CAMPL] & USE_ENVELOPE) volumeC = volumeE;
  		}

		return value;
}

// updateBufferSync contains no locks, it should only be called from sync() directly,
// otherwise use updateBuffer()
void AY38910::updateBuffer(unsigned int requiredSamples) {

    unsigned samples = requiredSamples - samplesInBuffer;

 	if (samples > AudioMixer::Instance()->getMaxBufferIndex()) {
		// this happened sometimes, appearently when the emuTime wraps.
		// the emulator either hangs trying to generate that many samples or crashes because memory is overwritten
		DBERR("ERROR updateBuffer(%i samples) requested but audioBuffer size is \n", (int) samples);
		return;
	}

	for (unsigned int i=0;i<samples;i++) { 
		audioBuffer[samplesInBuffer++] = makeWave();
    }
}

// requiredSamples is the amount of samples to complete in the buffer 
void AY38910::sync(unsigned int requiredSamples) 
{ 
	if (samplesInBuffer < requiredSamples)
	{
		//DBERR("syncing for %u/%u samples\n", requiredSamples-samplesInBuffer, requiredSamples);
		updateBuffer(requiredSamples);		// make sure at least this amount of samples is ready
	}
}

void AY38910::audioDone()
{
    samplesInBuffer = 0;
    lastSync = emuTime;
}
