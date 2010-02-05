//! Debugger.h
#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "msxtypes.h"
#include "SDL_events.h"
#include <list>

/*!
 * The Debugger class 
 */
class Debugger {

private:
		Debugger();
		unsigned int 	startAddress;
		unsigned int 	currentAddress;
        static std::string wordToHexstring(nw_word number);
        static std::string wordsToHexstring(nw_word count, nw_word ocword1, nw_word ocword2);
	

public:
		~Debugger();
		static Debugger * Instance();
		void handleKeyEvent(SDL_Event *event);
		void updateDisplay(SDL_Surface *, std::list<std::string> *);
};

#endif

