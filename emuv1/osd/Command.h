//! Command.h
#ifndef COMMANDEMU_H
#define COMMANDEMU_H

#include "msxtypes.h"
#include "SDL.h"
#include <list>
#include <string>
#include <map>

enum { DEBUGMODE, MONMODE, COMMANDMODE, LOGMODE };
enum {
    COMMAND_HELP,
    COMMAND_QUIT,
    COMMAND_FREQ,
    COMMAND_LIST,
    COMMAND_LOG,
    COMMAND_RESET,
    COMMAND_ALPHA,
    COMMAND_MON,
    COMMAND_PRINT,
};

typedef std::map<std::string, int> commandMap;
typedef std::list<std::string> stringList;

/*!
 * The Command class processes/interprets the commands given on the commandline interface
 */
class Command {

private:
		Command();

		std::string   commandLine;
		unsigned int  cursor;
        stringList    textBuffer;
        bool          update;
        
        commandMap    command;
        stringList    history;
        stringList::iterator    historyIterator;

		void      handleKeyEventCM(SDL_Event *);
		void      interpretCommand();
		void      addLine(std::string);
        nw_word    string2Word(std::string);
        
public:
        static nw_word hex2Word(std::string);
        static std::string wordToHexstring(nw_word number);
        static std::string wordsToHexstring(nw_word count, nw_word ocword1, nw_word ocword2);

		~Command();
		static Command * Instance();\
		void handleKeyEvent(SDL_Event *);
        		
		void nextScaler();
		std::list<std::string> *getUpdatedTextBuffer();
		int	commandMode;
};

#endif

