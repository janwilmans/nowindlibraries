// LogWatcher.cpp

#include "osd/LogWatcher.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;

LogWatcher::LogWatcher() {

    // Can't use DBERR here yet...
    logIterator = logBuffer.begin();
    update = true;
    displayTail = true;
}

LogWatcher *LogWatcher::Instance() {

	// implies singleton class
	static LogWatcher deInstantie;
	return &deInstantie;
}

LogWatcher::~LogWatcher() {
    
    //DBERR("LogWatcher destroyed.\n");
}

list<string> *LogWatcher::getTextBuffer() {
        
    if (update) {
        if (displayTail) {
            logIterator = logBuffer.end();
            for (int i=0;i<25;++i) {
                if (logIterator == logBuffer.begin()) break;
                logIterator--;
            }
        }
        updateTextBuffer();
        update = false;
        return &textBuffer;
    }
    return NULL;
}

void LogWatcher::addLogLine(char *log) {

    logBuffer.push_back(log);
    update = true;
}

bool LogWatcher::handleKeyEvent(SDL_Event *event) {
    
    if (event->type != SDL_KEYDOWN) return false;
    displayTail = false;
  
    switch(event->key.keysym.sym) {
    case SDLK_ESCAPE:
        return true;
    case SDLK_UP:
        if (logIterator != logBuffer.begin()) logIterator--;
        break;
    case SDLK_RETURN:
    case SDLK_DOWN:
        {
            list<string>::iterator tmp = logBuffer.end();
            logIterator++;
            for (int i=0;i<25;++i) {
                if (tmp == logIterator) {
                    logIterator--;
                    break;                
                }
                tmp--;
            }
        }
        break;
    case SDLK_END:
        displayTail = true;
        break;
    case SDLK_PAGEUP:
        for (int i=0; i<25; i++, logIterator--) {
            if (logIterator == logBuffer.begin()) break;
        }
        break;
    case SDLK_HOME:
        logIterator = logBuffer.begin();
        break;
    case SDLK_PAGEDOWN:
        for (int i=0; i<25; i++, logIterator++) {
            if (logIterator == --logBuffer.end()) break;
        }
        break;
    default:
        break;
    }
    updateTextBuffer();
    update = true;
    return false;
}

void LogWatcher::updateTextBuffer() {

    list<string>::iterator tmp = logIterator;

    textBuffer.clear();
    for (int i=0;i<25;++i) {
        if (tmp == logBuffer.end()) break;
        textBuffer.push_back((*tmp++));
    }
}
