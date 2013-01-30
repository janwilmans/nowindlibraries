//! MemoryMonitor.h
#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

#include "msxtypes.h"
#include "SDL.h"
#include <list>

/*!
 * The MemoryMonitor class 
 */
class MemoryMonitor {

private:
		MemoryMonitor();
		std::list<std::string> *textBuffer;
		nw_word address;

public:
		~MemoryMonitor();
		static MemoryMonitor * Instance();
		
		std::list<std::string> *getTextBuffer();
		bool  handleKeyEvent(SDL_Event *);
		void  setAddress(nw_word);
};

#endif

