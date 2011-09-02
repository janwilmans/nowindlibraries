// Command.cpp

#include <math.h>

#include "cpu/Z80.h"
#include "cpu/Disassembler.h"

#include "Command.h"
#include "Debugger.h"
#include "MemoryMonitor.h"
#include "LogWatcher.h"
#include "Emulator.h"
#include "GUI.h"
#include "OnScreenDisplay.h"
#include "DiskInterface.h"
#include "video/V9938.h"
#include "Debug.h"
#include "Media.h"

using namespace std;

Command::Command() {

	DBERR("Command constructor...\n");
	cursor = 0;
	historyIterator = history.begin();
	commandMode = COMMANDMODE;
    update = true;
    
    addLine("Nowind OSD");
	addLine("Type 'list' for all available commands.");
    addLine(">");

    command["help"] = COMMAND_HELP;
    command["quit"] = COMMAND_QUIT;
    command["freq"] = COMMAND_FREQ;
    command["list"] = COMMAND_LIST;
    command["log"] = COMMAND_LOG;
    command["reset"] = COMMAND_RESET;
    command["alpha"] = COMMAND_ALPHA;
    command["mon"] = COMMAND_MON;
    command["print"] = COMMAND_PRINT;
    command["?"] = COMMAND_PRINT;
    command["dis"] = COMMAND_DISASM;
    command["debug"] = COMMAND_DEBUG;
    command["disk"] = COMMAND_CHANGE_DISK;
    
	DBERR("Command constructor...finished\n");
}

Command *Command::Instance() {

	// implies singleton class
	static Command deInstantie;
	return &deInstantie;
}

Command::~Command() {
	DBERR("Command destroyed.\n");
}

void Command::handleKeyEvent(SDL_Event *event) {

    bool enableCommandMode = false;
    switch (commandMode) {
	case DEBUGMODE:
		Debugger::Instance()->handleKeyEvent(event);
		break;
	case MONMODE:
		enableCommandMode = MemoryMonitor::Instance()->handleKeyEvent(event);
        break;
	case COMMANDMODE:
		handleKeyEventCM(event);
		break;
	case LOGMODE:
        enableCommandMode = LogWatcher::Instance()->handleKeyEvent(event);
        break;
    default:
		assert(false);
	}
	if (enableCommandMode) commandMode = COMMANDMODE;
//    OnScreenDisplay::Instance()->renderOSD();
}

void Command::handleKeyEventCM(SDL_Event *event) {
	
    if (event->type != SDL_KEYDOWN) return;
    SDL_keysym keysym = event->key.keysym;
    
    switch (keysym.sym) {
    case SDLK_RETURN:
        if (commandLine != "") {
            history.push_back(commandLine);
            historyIterator = history.end();
            interpretCommand();
            commandLine = "";
            cursor = 0;
        }
        addLine(">");
        break;
    case SDLK_ESCAPE:
        commandLine = "";
        break;
    case SDLK_UP:
        if (historyIterator != history.begin()) {
            historyIterator--;
            commandLine = *historyIterator;
            cursor = commandLine.length();
        }
        break;
    case SDLK_DOWN:
        if (historyIterator != --history.end()) {
            historyIterator++;
            commandLine = *historyIterator;
            cursor = commandLine.length();
        }
        break;
    case SDLK_LEFT:
        if (cursor > 0) cursor--;
        break;
    case SDLK_RIGHT:
        if (cursor < commandLine.length()) cursor++;
        break;
    case SDLK_HOME:
        cursor = 0;
        break;
    case SDLK_END:
        cursor = commandLine.length();
        break;    
    case SDLK_BACKSPACE:
        if (cursor > 0) {
            cursor--;
            commandLine = commandLine.substr(0, cursor) + commandLine.substr(cursor + 1);
        }
        break;
    case SDLK_DELETE:
        if (cursor < commandLine.length()) {
            commandLine = commandLine.substr(0, cursor) + commandLine.substr(cursor + 1);
        }
        break;
	default:
        if ((keysym.unicode > 31) && (keysym.unicode < 127)) {
            commandLine = commandLine.substr(0, cursor) + 
                char(event->key.keysym.unicode) + 
                commandLine.substr(cursor);
            cursor++;
        }
        break;
    }
    
    textBuffer.pop_back();
    string tmp = ">" + commandLine;
    addLine(tmp);
}

void Command::addLine(string str) {

    if (textBuffer.size() > 20) textBuffer.pop_front();
    textBuffer.push_back(str);
    update = true;
}

void Command::nextScaler() {
	
	// TODO: kan dit niet beter naar de GUI?? 
	// anders krijg je hier veel verschilende routines
	
	int scaler = GUI::Instance()->getScaler();
	if (scaler == SCALE_LAST) scaler = 0;
		else scaler++;
	GUI::Instance()->setScaler(scaler);
}

void Command::interpretCommand() {

    bool error = false;
    int i;
    string part[10];
    string::size_type location, last=0;
  
    for (i=0;i<10;i++) {
        location = commandLine.find(' ', last);
        if (location != string::npos) {
            part[i] = commandLine.substr(last, location-last);
            last = location + 1;
         } else {
            part[i] = commandLine.substr(last);
            break;
        }
    }
    
    commandMap::iterator it;
    it = command.find(part[0]);
    
    switch (it->second) {
    case COMMAND_HELP:
        addLine("Usage: help <command>");
        break;
    case COMMAND_QUIT:
		Emulator::Instance()->quit();
		break;
	case COMMAND_FREQ:
        if (V9938::Instance()->switchFrequency()) {
            addLine("VDP interrupt frequency set to 50Hz");   
        } else {
            addLine("VDP interrupt frequency set to 60Hz");   
        }
        break;
    case COMMAND_LIST:
        addLine("Available commands:");
        for(it = command.begin();it != command.end(); it++) {
            addLine(it->first);
        }
        break;
    case COMMAND_LOG:
        commandMode = LOGMODE;
        LogWatcher::Instance()->updateTextBuffer();
        break;        
	case COMMAND_RESET:
        Z80::Instance()->hardReset();
        break;
    case COMMAND_ALPHA:
        if (i == 1) OnScreenDisplay::Instance()->setAlphaBlending(string2Word(part[1]) & 255);
        else error = true;
        break;
    case COMMAND_MON:
        if (i == 1) MemoryMonitor::Instance()->setAddress(string2Word(part[1]));
        commandMode = MONMODE;
        break;
    case COMMAND_PRINT: 
        {
            char tmp[50];
            nw_word value = string2Word(part[1]);
            sprintf(tmp, "dec:%i hex:%04x char(&255):%c", value, value, value & 255);
            addLine(tmp);
        }
        break;
    case COMMAND_DISASM:
        {
            // TODO: check addresses > 0xffff !!!!
            char address[50];
            nw_word addr = string2Word(part[1]);
            for (int i=0;i<20;i++) {
                
                nw_word oc_part1 = Z80::Instance()->readMem16Public(addr);
                nw_word oc_part2 = Z80::Instance()->readMem16Public(addr+2);

                string opcode_str = Disassembler::Instance()->disAsm(&addr, oc_part1, oc_part2, true);
                sprintf(address, "0x%04x: ", addr);
                addLine(address + opcode_str);
            }
        }
        break;
/*
    case COMMAND_DEBUG:
        {
    		commandMode = DEBUGMODE;
            OnScreenDisplay::Instance()->setTextBuffer();
        }
        break;
*/
    case COMMAND_CHANGE_DISK:
        {
		    Media::Instance()->insertMedia(part[1], 0);
        }
        break;
    default:
        if (commandLine != "") addLine("Unknown command...");
    }
    
    if (error) addLine("Invalid argument(s)...");
}

nw_word Command::string2Word(string str) {
       
    nw_word word = 0;
    int len = str.length();
    if (len > 0)
    {
        if (str[0] == '$') return hex2Word(str.substr(1));
        if (str[len-1] == 'h') return hex2Word(str.substr(0, len-1));
 
        nw_word power = 1;
        for (int i=0;i<len;i++) {
           
            nw_word value = str[len-i-1] - 48;
            if (value > 9) return 0xffff;
            word += value * power;
            power *= 10;
        }
    }
    return word;    
}

nw_word Command::hex2Word(string hex) {
    
    int len = hex.length();
    nw_word word = 0;
    
    for (int i=0;i<len;i++) {
        
        nw_word value = (hex[len-i-1] | 0x20) - 48;
        if (value > 9) value -= 39;
        if (value > 15) return 0xffff;
        word += value << (i*4);
    }
    return word;
}

std::string Command::wordToHexstring(nw_word number) {

    char temp[20];
    if (number < 0x100) {
        sprintf(temp,"%02X",number);
    } else {
        sprintf(temp,"%04X",number);
    }
    return string(temp);
}

std::string Command::wordsToHexstring(nw_word count, nw_word ocword1, nw_word ocword2) {

    char temp[40];
    switch (count) {
        case 0:
                assert(false);
        case 1:
                sprintf(temp,"%02X       ",ocword1&0xff);
                break;
        case 2:
                sprintf(temp,"%02X%02X     ",ocword1&0xff, ocword1 >> 8);
                break;
        case 3:
                sprintf(temp,"%02X%02X %02X  ",ocword1&0xff, ocword1 >> 8, ocword2&0xff);
                break;
        case 4:
                sprintf(temp,"%02X%02X %02X%02X",ocword1&0xff, ocword1 >> 8, ocword2&0xff, ocword2 >> 8);
                break;
        default:
                break;
    }
    return string(temp);
}

list<string> *Command::getUpdatedTextBuffer() {

    list<string> *buffer;
    	
	switch (commandMode) {
	case COMMANDMODE:
        {
            if (update) {
                buffer = &textBuffer;
                update = false;
            } else {
                buffer = NULL;
            }
        }
        break;
	case DEBUGMODE:
        buffer = NULL; // TODO!
        break;
	case MONMODE:
        buffer = MemoryMonitor::Instance()->getTextBuffer();
        break;
	case LOGMODE:
        buffer = LogWatcher::Instance()->getTextBuffer();
        break;
	default:
		assert(false);
	}
	return buffer;
}
