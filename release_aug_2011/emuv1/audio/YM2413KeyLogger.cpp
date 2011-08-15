// YM2413.cc

#include "stdio.h"
#include "cpu/Z80.h"
#include "Debug.h"
#include "YM2413.h"
#include "Emulator.h"

using namespace std;

YM2413 * YM2413::Instance() {

		/* implies singleton class */
		static YM2413 deInstantie;
		return &deInstantie;
}

YM2413::YM2413() : emuTime(Z80::Instance()->emuTime) {

	DBERR("YM2413 constructor...\n");
	/* init some values */


	for (int c=0;c<0x40;c++) {
		reg[c] = 0;
	}
	for (int c=1;c<10;c++) {
		lastChan[c].freq = 0;
		lastChan[c].oct = 0;
		lastChan[c].sustain = 0;
		lastChan[c].volume = 0;
		lastChan[c].stroke = 0;
		chan[c].freq = 0;
		chan[c].oct = 0;
		chan[c].sustain = 0;
		chan[c].key = 0;
		chan[c].volume = 0;
		chan[c].stroke = 0;
	}
	lastLine = 0;

	DBERR("YM2413 constructor...finished\n");
}

YM2413::~YM2413() {

    	/*
    		mbmStream->seekp(dest);
    		mbmStream->write((char *)buffer,count);
    	*/
    	DBERR("YM2413 destructor\n");
/*
    	return;

		int positions = noteLines.size();
		int songSize = positions/16;

		if (songSize ==0) {
			songSize = 1;
  		}

    	string filename = "output.mbm"; 
    	mbmStream = new fstream(filename.c_str(),ios::binary | ios::out);
    	if (mbmStream->fail()) {
    		DBERR("failed to open MBM log file...\n");
    	}
    
    	mbmHeaderStruct mbmHeader;
    	memset(&mbmHeader,0x00,sizeof(mbmHeaderStruct));
    
    	mbmHeader.length = songSize-1;
    	mbmHeader.id = 0x00;

		for (int i=0;i<16;i++) {
			mbmHeader.instrumentList[i] = 1;
		}

		for (int i=1;i<32;i=i+2) {
			mbmHeader.instrumentVolList[i] = 0;		// volumes op 15 (0=15, 1=14...)
		}

		for (int i=0;i<32;i=i+2) {
			mbmHeader.instrumentVolList[i] = 1;		// alle instrumenten op 1
		}

		mbmHeader.startInstrumentsMsxAudio[0] = 1;
	
    	mbmHeader.channelChipSettings[0] = 1;		// msx-music ?
    	mbmHeader.startTempo = 5;  // 3=20 ?

    	sprintf(mbmHeader.trackName,"Captured by NOWIND msx-music logger     ");
    	sprintf(mbmHeader.sampleKitName,"NONE    ");
    
    	mbmHeader.loopPosition = 1;
    
    	mbmStream->write((char *)&mbmHeader,sizeof(mbmHeaderStruct));

		unsigned char fullLineEnd = 0xF9;
//		unsigned char fullLineEndWithTempo = 0xF8;
//		unsigned char lineEnd = 0xFE;
		unsigned char emptyPos = 0xFF;
		unsigned char pos = 1;
//		unsigned char tempo = 1;
	
		unsigned int baseTempo = 20;

		// amount of vdp-ints pause after a note in this tempo 
		unsigned int baseInts = 25-baseTempo;	

    	for (int j=0;j<songSize;j++) {
			mbmStream->write((char *)&pos,1);
			pos++;
		}
		
		unsigned char pos1 = 0x81;
		unsigned char pos2 = 1;
    	for (int j=0;j<songSize;j++) {
			mbmStream->write((char *)&pos1,1);
			mbmStream->write((char *)&pos2,1);
		}
	
    	for (list<char *>::iterator i = noteLines.begin(); i != noteLines.end(); i++) {
    		char* line = *i;
			mbmStream->write((char *)line,6);			// detect 6 or 9 channels
			mbmStream->write((char *)&fullLineEnd,1);

         	int len = line[10] / baseInts;

//            int rest = line[10] % baseInts;
//			if (rest == 1) {
//				mbmStream->write((char *)&fullLineEndWithTempo,1);		// tempo
//				tempo = 23;
//				mbmStream->write((char *)&tempo,1);			// tempo
//			}
//			mbmStream->write((char *)&fullLineEnd,1);

			for (int l=0;l<len;l++) {
				mbmStream->write((char *)&emptyPos,1);
			}

    		delete line;
    	}

		pos = 0;
		mbmStream->write((char *)&pos,1);
    	
		mbmStream->close();
*/
		// destructor
		DBERR("YM2413 destroyed.\n");
}

void YM2413::writeAddress(nw_byte value) {

	currentRegister = (unsigned int) value;
}

void YM2413::writeData(nw_byte value) {

//    DEBUGMESSAGE("FMPAC writeData, [" << hex << (unsigned int) currentRegister << "]= " << dec << (unsigned int) value << endl);

	reg[currentRegister] = value;
	unsigned int regNr = 0;

	if (currentRegister >= 0x10 && currentRegister <= 0x18) {
		regNr = currentRegister - 0x10;
	}
	if (currentRegister >= 0x20 && currentRegister <= 0x28) {
		regNr = currentRegister - 0x20;
	}
	if (currentRegister >= 0x30 && currentRegister <= 0x38) {
		regNr = currentRegister - 0x30;
	}

	if ((currentRegister >= 0x10 && currentRegister <= 0x18)) {
		chan[regNr].freq = reg[0x10+regNr] + ((reg[0x20+regNr] & 0x01) << 8);
	}

	if ((currentRegister >= 0x20 && currentRegister <= 0x28)) {

		unsigned int lastKey = chan[regNr].key;

		chan[regNr].freq = reg[0x10+regNr] + ((reg[0x20+regNr] & 0x01) << 8);
		chan[regNr].oct = (reg[0x20+regNr] >> 1) & 7;
		chan[regNr].key = (reg[0x20+regNr] >> 4) & 1;
		chan[regNr].sustain = (reg[0x20+regNr] >> 5) & 1;

    	if ((lastKey == 0) && (chan[regNr].key == 1)) {
    			chan[regNr].stroke = 1;
    	}

	}
	if (currentRegister >= 0x30 && currentRegister <= 0x38) {
		chan[regNr].volume = reg[0x30+regNr] & 15; 
	}

	if (currentRegister == 0x0E) {

    	if ((drumStroke == 0) && ((reg[0x0E] >> 5) & 1)) {
    		drumStroke = 1;
    	} else {
			drumStroke = (reg[0x0E] >> 5) & 1;
		}

    	unsigned int drums = reg[0x0E];
    	drum[0] = drums & 1;
        drums = drums >> 1;
    	drum[1] = drums & 1;
        drums = drums >> 1;
    	drum[2] = drums & 1;
        drums = drums >> 1;
    	drum[3] = drums & 1;
        drums = drums >> 1;
    	drum[4] = drums & 1;
	}
}

void YM2413::dumpChannelsBeat() {

	AYbeat++;
	dumpChannels();
}

void YM2413::dumpChannels() {

	if ((reg[0x0e] >> 5) & 1) {
		// rhythm mode (6 ch + 5 drum)
		dump6ChannelsAndRhythm();
	
	} else {
		// melody mode (9 ch)
		dump9Channels();
	}
}

void YM2413::dump6ChannelsAndRhythm() {

	bool changed = false;

	char note[250];
	for (int c=0;c<6;c++) {
		sprintf(note,"%s%u",getNote(chan[c].freq).c_str(),chan[c].oct+1);
		chanNote[c] = string(note);
		if (chan[c].stroke) { changed = true; }
	}

	if (!changed) return;

	char lin[16];

	/* log line with mbm-data */
	nw_byte *line = new nw_byte[16];
	for (int c=0;c<6;c++) {

		if (chan[c].stroke) {
    		line[c] = (12*chan[c].oct)+getNoteNr(chan[c].freq);
			sprintf(lin,"%2X ",line[c]);
   			DEBUGMESSAGE(lin);
		} else {
			line[c] = 0xF3;
   			DEBUGMESSAGE("   ");
		}
	}
/*
	if (lastLine != 0) {
  		lastLine[10] = AYbeat;
	}
	lastLine = line;
*/
	line[10] = AYbeat;
	noteLines.push_back((char *)line);

	DEBUGMESSAGE(endl);

	char timeStr[250];
	sprintf(timeStr,"%02u",AYbeat);
	DEBUGMESSAGE(dec << string(timeStr) << ";");

	for (int c=0;c<6;c++) {
		if (chan[c].stroke) { 
			DEBUGMESSAGE("[" << chanNote[c] << "];(" );
		} else {
			DEBUGMESSAGE("[   ];(" );
		}
		if (((reg[0x20+c] >> 5) & 1)) { DEBUGMESSAGE("S"); } else { DEBUGMESSAGE(" "); }
		if (((reg[0x20+c] >> 4) & 1)) { DEBUGMESSAGE("K"); } else { DEBUGMESSAGE(" "); }
		unsigned int instr = ((reg[0x30+c] >> 4) & 15);
		unsigned int vol = (reg[0x30+c] & 15);
	
		char info[250];
		sprintf(info,"%02u-%02u",instr,vol);
		DEBUGMESSAGE("," << string(info) << ");");
	}

	DEBUGMESSAGE(" + ;");

	DEBUGMESSAGE("{");

	if (drumStroke) {
		if (drum[4] && ((reg[0x36] & 15) > 0 )) {
			DEBUGMESSAGE("B");
		} else { DEBUGMESSAGE(" "); }

    	if (drum[3] && ((reg[0x37] & 15) > 0 )) {
    		DEBUGMESSAGE("S");
       	} else { DEBUGMESSAGE(" "); }

    	if (drum[2] && (((reg[0x38] >> 4) & 15) > 0 )) {
    		DEBUGMESSAGE("T");
       	} else { DEBUGMESSAGE(" "); }

    	if (drum[1] && ((reg[0x38] & 15) > 0 )) {
    		DEBUGMESSAGE("C");
       	} else { DEBUGMESSAGE(" "); }

    	if (drum[0] && (((reg[0x37] >> 4) & 15) > 0 )) {
    		DEBUGMESSAGE("H");
       	} else { DEBUGMESSAGE(" "); }
	}
	DEBUGMESSAGE("}");

   	DEBUGMESSAGE(endl);
	
   	for (int c=0;c<6;c++) {
   		lastChan[c].freq = chan[c].freq; 
		lastChan[c].oct = chan[c].oct;
		lastChan[c].key = chan[c].key;
		lastChan[c].volume = chan[c].volume;
		lastChan[c].sustain = chan[c].sustain;
		chan[c].stroke = 0;
		lastNote[c] = chanNote[c];
   	}
		
	drumStroke = 0;
	AYbeat = 0;
}

void YM2413::dump9Channels() {

	bool changed = false;

	char note[250];
	for (int c=0;c<9;c++) {
		sprintf(note,"%s%u",getNote(chan[c].freq).c_str(),chan[c].oct+1);
		chanNote[c] = string(note);
		if (chan[c].stroke) { changed = true; }
	}

/*  why NO comipler error, while 'time' is not defined ?!!!
	char timeStrTest[250];
	sprintf(timeStrTest,"%03u",time);
*/

	if (!changed) return;

	/* log line with mbm-data */
	nw_byte *line = new nw_byte[16];
	for (int c=0;c<9;c++) {
		if (chan[c].stroke) {
			line[c] = (12*chan[c].oct)+getNoteNr(chan[c].freq);
		} else {
			line[c] = 0xF3;
		}
	}
	line[10] = AYbeat;
	noteLines.push_back((char *)line);

	char timeStr[250];
	sprintf(timeStr,"%02u",AYbeat);

	DEBUGMESSAGE(dec << string(timeStr) << ";");

	for (int c=0;c<9;c++) {
		if (chan[c].stroke) { 
			DEBUGMESSAGE("[" << chanNote[c] << "];(" );
		} else {
			DEBUGMESSAGE("[   ];(" );
		}
		if (((reg[0x20+c] >> 5) & 1)) { DEBUGMESSAGE("S"); } else { DEBUGMESSAGE(" "); }
		if (((reg[0x20+c] >> 4) & 1)) { DEBUGMESSAGE("K"); } else { DEBUGMESSAGE(" "); }
		unsigned int instr = ((reg[0x30+c] >> 4) & 15);
		unsigned int vol = (reg[0x30+c] & 15);

		char info[250];
		sprintf(info,"%02u-%02u",instr,vol);
		DEBUGMESSAGE(")" << string(info) << ";");
	}
	
	DEBUGMESSAGE(endl);
	
	for (int c=0;c<9;c++) {
		lastChan[c].freq = chan[c].freq; 
		lastChan[c].oct = chan[c].oct;
		lastChan[c].key = chan[c].key;
		lastChan[c].volume = chan[c].volume;
		lastChan[c].sustain = chan[c].sustain;
		chan[c].stroke = 0;
		lastNote[c] = chanNote[c];
	}
	AYbeat = 0;
}

string YM2413::getNote(unsigned int freq) {

	if (noteMatch(freq,173)) { return string("C "); }

	if (noteMatch(freq,181)) { return string("C#"); }
	if (noteMatch(freq,192)) { return string("D "); }
	if (noteMatch(freq,204)) { return string("D#"); }
	if (noteMatch(freq,216)) { return string("E "); }
	if (noteMatch(freq,229)) { return string("F "); }
	if (noteMatch(freq,242)) { return string("F#"); }
	if (noteMatch(freq,257)) { return string("G "); }
	if (noteMatch(freq,272)) { return string("G#"); }
	if (noteMatch(freq,288)) { return string("A "); }
	if (noteMatch(freq,305)) { return string("A#"); }
	if (noteMatch(freq,232)) { return string("B "); }
	if (noteMatch(freq,343)) { return string("C "); }

	if (noteMatch(freq,257)) { return string("G "); }
	if (noteMatch(freq,272)) { return string("G#"); }
	if (noteMatch(freq,288)) { return string("A "); }
	if (noteMatch(freq,305)) { return string("A#"); }
	if (noteMatch(freq,323)) { return string("B "); }
	if (noteMatch(freq,343)) { return string("C "); }
	if (noteMatch(freq,363)) { return string("C#"); }
	if (noteMatch(freq,385)) { return string("D "); }
	if (noteMatch(freq,408)) { return string("D#"); }
	if (noteMatch(freq,432)) { return string("E "); }
	if (noteMatch(freq,458)) { return string("F "); }
	if (noteMatch(freq,485)) { return string("F#"); }

	return string("??");
}

int YM2413::getNoteNr(unsigned int freq) {

	if (noteMatch(freq,173)) { return 1; }

	if (noteMatch(freq,181)) { return 2; }
	if (noteMatch(freq,192)) { return 3; }
	if (noteMatch(freq,204)) { return 4; }
	if (noteMatch(freq,216)) { return 5; }
	if (noteMatch(freq,229)) { return 6; }
	if (noteMatch(freq,242)) { return 7; }
	if (noteMatch(freq,257)) { return 8; }
	if (noteMatch(freq,272)) { return 9; }
	if (noteMatch(freq,288)) { return 10; }
	if (noteMatch(freq,305)) { return 11; }
	if (noteMatch(freq,232)) { return 12; }
	if (noteMatch(freq,343)) { return 1; }

	if (noteMatch(freq,257)) { return 8; }
	if (noteMatch(freq,272)) { return 9; }
	if (noteMatch(freq,288)) { return 10; }
	if (noteMatch(freq,305)) { return 11; }
	if (noteMatch(freq,323)) { return 12; }
	if (noteMatch(freq,343)) { return 1; }
	if (noteMatch(freq,363)) { return 2; }
	if (noteMatch(freq,385)) { return 3; }
	if (noteMatch(freq,408)) { return 4; }
	if (noteMatch(freq,432)) { return 5; }
	if (noteMatch(freq,458)) { return 6; }
	if (noteMatch(freq,485)) { return 7; }

	return 0;
}

bool YM2413::noteMatch(unsigned int freq, unsigned int fixedFreq) {

	return (freq > fixedFreq-5) && (freq < fixedFreq+5);
}

void YM2413::clockPulse() {

	/* prevent start-up problems
	 * FIX: initialize this better.
	 */

	if (lastClockPulseEmutime < 2) {
		lastClockPulseEmutime = emuTime;
		return;
	}

	// z80 fclock = 3579545
	// tone en noise are calculated by taking fclock / ( 16 * period)

	// calculate elapsed emutime to become independant of how often
	// a clockPulse is generated (there is still a certian minimum ofcource)
	// but 120 hz would do just fine, where is normally a freq. of 223 Khz
	// would be required.

	emutimeElapsed = emuTime - lastClockPulseEmutime;
	if (emutimeElapsed < 4000) DBERR("Warning: soundinterrupt occurring too fast (" << emutimeElapsed << ") , sound might become inaccurate...\n");

	unsigned int value = 0;
	unsigned int redistribute = int(emutimeElapsed/(Z80::Instance()->cpuFrequency/sampleRate));      // rounding error!!!
	unsigned long AYticks = int(emutimeElapsed);

//  DBERR("clockPulse emutimeElapsed: " << emutimeElapsed << " redistribute: " << dec << redistribute << " AYticks: " << AYticks);

	if (redistribute > 0) AYticks = (100*AYticks) / redistribute;

//  DBERR(" per rad: " << AYticks << " tonePeriodA: " << tonePeriodA << endl);

	for (unsigned int i=0;i<redistribute;i++) {            // 5512 = frate/4

		/* set initialial output value to zero for every iteration, very important */
		value = 0;

		// de waarde in AYticks is aantal emutijd per byte
		// redistribute is aantal bytes
		
#ifndef CONSOLE_DEBUGGING_ON
#ifdef FMPAC_SOUND_ON

		// value
		
		if (soundBufferIndex < soundBufferSize) {
			 soundBuffer[soundBufferIndex++] = value;
		} else {
			 /* if the buffer overruns, there is something very wrong..
			  * Just flush the buffer, enjoy 0,25 second of silence and continue...
			  */
			 DBERR("Audio buffer overrun, flushing buffer... (will cause a small silent period)\n");
			 soundBufferIndex = 0;
		}

#endif
#endif

	}
	lastClockPulseEmutime = emuTime;

}

