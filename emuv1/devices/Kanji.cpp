// Kanji.ccp

#include "stdio.h"
#include "Kanji.h"
#include "Media.h"
#include "Debug.h"

using namespace std;

/* TODO:
	JIS2 is nu net zo geimplementeerd als JIS1
	Is dit in werkelijkheid ook zo? En waarmee kun je JIS2 testen???
*/

Kanji::Kanji() {

	DBERR("Kanji constructor...\n");
	kanjiRom[0] = NULL;
	kanjiRom[1] = NULL;
    loadKanjiRom();
	reset(); // TODO: uiteindelijk niet hier!!!!!!
	DBERR("Kanji constructor...finished\n");
}

Kanji::~Kanji() {
	
	if (kanjiRom[0] != NULL) delete [] kanjiRom[0];
	DBERR("Kanji destroyed.\n");
}

void Kanji::reset() {
    
    DBERR("RESET Kanji\n");
	kanjiAddress[0] = 0;
	kanjiAddress[1] = 0;
 	offset[0] = 0;
  	offset[1] = 0;
}    

void Kanji::loadKanjiRom() {

    fstream * ifs = Media::openfile("../roms/KANJI.ROM", ios::in | ios::binary);
	if (!ifs->fail()) {
   		ifs->seekg (0, ios::end);
   		long size = ifs->tellg();
    	DBERR("Found KANJI.ROM (%ukB)\n", size / 1024);
   		
   	    if ((size == 131072) || (size == 262144)) {
	        DBERR("   JIS1 enabled!\n");
	        char *romBuffer = new char[size];
	        ifs->seekg (0, ios::beg);
         	ifs->read(romBuffer, size);
            kanjiRom[0] = romBuffer;
            if (size == 262144) {
                DBERR("   JIS2 enabled!\n");
                kanjiRom[1] = romBuffer + 131072;
            }                 	    	
        } else {
            DBERR("   Invalid KANJI.ROM!\n");
        }    
        ifs->close();
    }        
    delete ifs;
}    

void Kanji::writeAddressLow(nw_byte value, int jis) {
	
	value &= 0x3f;
	kanjiAddress[jis] = (kanjiAddress[jis] & 0x1f800) | (value << 5);
	offset[jis] = 0;
}    

void Kanji::writeAddressHigh(nw_byte value, int jis) {
	
	value &= 0x3f;
	kanjiAddress[jis] = (kanjiAddress[jis] & 0x007e0) | (value << 11);
	offset[jis] = 0;
}    

nw_byte Kanji::readKanjiRom(int jis) {
	
	if (jis==1) {
     	DBERR("JIS2 read -> address: %u\n", kanjiAddress[jis] + offset[jis]);
    }   	
	nw_byte value = 255;
	if (kanjiRom[jis] != NULL) {
		offset[jis] &= 31;
		value = (nw_byte)kanjiRom[jis][kanjiAddress[jis] + offset[jis]++];
	}
 	return value;	
}
