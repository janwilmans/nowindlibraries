//! LogWatcher.h
#ifndef LOGWATCHER_H
#define LOGWATCHER_H

#include "msxtypes.h"
#include "SDL.h"
#include <list>

/*!
 * The LogWatcher class 
 */
class LogWatcher {

private:
		LogWatcher();
		std::list<std::string> textBuffer;
		bool update;
		bool displayTail;

        std::list<std::string> logBuffer;		
		std::list<std::string>::iterator logIterator;
        
public:
		~LogWatcher();
		static LogWatcher * Instance();
		
		std::list<std::string> *getTextBuffer();

		bool    handleKeyEvent(SDL_Event *);
        void    updateTextBuffer();
        void    addLogLine(char *);

};

#endif

