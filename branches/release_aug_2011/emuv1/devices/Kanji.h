//! Kanji.h
#ifndef KANJI_H
#define KANJI_H

#include <iostream>
#include <string>
#include "msxtypes.h"

class Kanji {
	
private:
    
    void			loadKanjiRom();
    
	char			*kanjiRom[2];
	unsigned int 	kanjiAddress[2];
	unsigned int	offset[2];
	
public:
    			Kanji();	
				~Kanji();

	void		reset();
	void		writeAddressHigh(nw_byte, int);
	void		writeAddressLow(nw_byte, int);
	nw_byte		readKanjiRom(int);
};

#endif

