// Debugger.cpp

#include "Debugger.h"
#include "OnScreenDisplay.h"
#include "Command.h"
#include "cpu/Disassembler.h"
#include "cpu/Z80.h"
#include "Debug.h"

using namespace std;

Debugger::Debugger() {

	DBERR("Debugger constructor...\n");
	DBERR("Debugger constructor...finished\n");

	startAddress = 0;
	currentAddress = startAddress+8;

}

Debugger *Debugger::Instance() {

	// implies singleton class
	static Debugger deInstantie;
	return &deInstantie;
}

void Debugger::handleKeyEvent(SDL_Event *event) {

	switch (event->type) {
	case SDL_KEYDOWN:

		switch (event->key.keysym.sym) {
		case SDLK_ESCAPE:
			Command::Instance()->commandMode = COMMANDMODE;    
			break;
	
    	case SDLK_RETURN:
    	case SDLK_KP_ENTER:
			break;
		case SDLK_UP:
            if (startAddress > 0) {
    			if (currentAddress == startAddress) {
    				startAddress--;
    			}
    			currentAddress--;
            }
			break; 
		case SDLK_DOWN:
            if (startAddress < 0xffff) {
    			if (currentAddress == startAddress+15) {
    				startAddress++;
    			}	
    			currentAddress++;
            }
			break;
		case SDLK_BACKSPACE:
			break;
		case SDLK_DELETE:
			 break;

	    case SDLK_HOME: break;
	    case SDLK_END: break;
  		case SDLK_RIGHT: 
            if (startAddress < 0xfff0) {
    			startAddress += 16;
    			currentAddress += 16;
            }
			break;
		case SDLK_LEFT:	
            if (startAddress > 15) {
            	startAddress -= 16;
    			currentAddress -= 16;
            }
			break;
		default:
			break;
		}
		startAddress &=0xffff;
	default:
	    break;
	}
}


void Debugger::updateDisplay(SDL_Surface *screen, list<string> *textBuffer) {

	textBuffer->clear();
	nw_word reg_pc = startAddress;   
	for (int i=0;i<16;i++) {
		nw_word instructionStartAddress = reg_pc;
	    string pcString = wordToHexstring(reg_pc);
	    nw_word ocPart1 = Z80::Instance()->readMem16Public(reg_pc);
	    nw_word ocPart2 = Z80::Instance()->readMem16Public(reg_pc+2); 
	
	    string opcodeString = Disassembler::Instance()->disAsm(&reg_pc,ocPart1,ocPart2,true);
	
	    string rowString = pcString + ": ";
	    while(rowString.length() < 7) rowString = rowString + " ";
	
	    rowString += wordsToHexstring(reg_pc - instructionStartAddress, ocPart1, ocPart2) + " " + opcodeString;

		textBuffer->push_back(rowString);
	}
//	OnScreenDisplay::Instance()->updateDisplay(screen);
    OnScreenDisplay::Instance()->xorBar(screen, currentAddress-startAddress);
}

Debugger::~Debugger() {
	DBERR("Debugger destroyed.\n");
}


std::string Debugger::wordToHexstring(nw_word number) {

    char temp[20];
    if (number < 0x100) {
        sprintf(temp,"%02X",number);
    } else {
        sprintf(temp,"%04X",number);
    }
    return string(temp);
}

std::string Debugger::wordsToHexstring(nw_word count, nw_word ocword1, nw_word ocword2) {

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
