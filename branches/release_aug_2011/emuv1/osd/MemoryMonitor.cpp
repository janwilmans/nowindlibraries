// MemoryMonitor.cpp

#include "osd/MemoryMonitor.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;

MemoryMonitor::MemoryMonitor() {

	DBERR("MemoryMonitor constructor...\n");
    textBuffer = new list<string>;
    address = 0;
    DBERR("MemoryMonitor constructor...finished\n");
}

MemoryMonitor *MemoryMonitor::Instance() {

	// implies singleton class
	static MemoryMonitor deInstantie;
	return &deInstantie;
}

MemoryMonitor::~MemoryMonitor() {
    delete(textBuffer);
    DBERR("MemoryMonitor destroyed.\n");
}

list<string> *MemoryMonitor::getTextBuffer() {
    return textBuffer;
}

bool MemoryMonitor::handleKeyEvent(SDL_Event *event) {

    if (event->type != SDL_KEYDOWN) return false;
    SDL_keysym keysym = event->key.keysym;
    
    switch (keysym.sym) {
    case SDLK_ESCAPE:
        return true;
    case SDLK_UP:
        address -= 16;
        break;
    case SDLK_DOWN:
        address += 16;
        break;
    case SDLK_PAGEUP:
        address -= 16 * 24;
        break;
    case SDLK_PAGEDOWN:
        address += 16 * 24;
        break;
    default:
        // no need to update
        return false;
    }
    
    textBuffer->clear();
    address &= 0xffff;
    nw_word currentAddress = address;
    
    char tmp[64];

    for (int j=0;j<24;j++) {
        sprintf(tmp, "%04x  ", address);
        for (int i=0;i<16;i++) {

            nw_byte peek = Z80::Instance()->readMemPublic(address++);
            address &= 0xffff;
            sprintf(tmp+6+(i*2)+(i/2), "%02x ", peek);
            
            if ((peek < 32) || (peek > 127)) peek = 46;
            tmp[i+47] = peek;
        }
        tmp[46] = 32;
        tmp[63] = 0;
        textBuffer->push_back(tmp);
    }
    address = currentAddress;
    return false;
}

void MemoryMonitor::setAddress(nw_word addr) {
    
    address = addr;
}
