/* 
 * Emulator.cpp: Emulation main class.
 *
 * Copyright (C) 2004 Jan Wilmans <jw@dds.nl>
 *                    Aaldert Dekker <a.dekker@student.tue.nl>
 */

#include "osd/Command.h"
#include "Media.h"
#include "cpu/Z80.h"
#include "Debug.h"
#include "video/V9938.h"
#include "audio/AudioMixer.h"
#include "audio/AudioDummy.h"
#include "audio/AY38910.h"
#include "audio/PPISound.h"
#include "audio/SCC.h"
#include "audio/YM2413.h"
#include "audio/MSXAudio.h"
#include "osd/OnScreenDisplay.h"
#include "GUI.h"
#include "controls/Keyboard.h"
#include "controls/Joystick.h"
#include "cpu/Disassembler.h"

#include "SDL_syswm.h"
#include "stdio.h"

#ifdef WIN32
	#include "shellapi.h"		// define HDROP and DragQueryFile / DragQueryPoint prototypes
#endif

// when OFF disables sound altogether
#define PSG_SOUND_ON
#define PPI_SOUND_OFF
#define SCC_SOUND_ON
#define MSXMUSIC_SOUND_ON
#define MSXAUDIO_SOUND_OFF

#define ZEXALL_OFF

using namespace std;

#include "Emulator.h"

unsigned int    Emulator::interruptType = 0;
bool            Emulator::notPaused = 0;
bool            Emulator::commandMode = false;
bool            Emulator::osdVisible = false;
bool            Emulator::running = 0;
bool            Emulator::mouseDragging = 0;
msTimeType      Emulator::startPauseTime = 0;
bool            Emulator::autoPause = 0;


Emulator * Emulator::Instance() {

		/* implies singleton class */
		static Emulator deInstantie;
		return &deInstantie;
}

static unsigned long starttime = 0;

Emulator::Emulator() {

	DBERR("Emulator constructor...\n");
	running = true;

	/* here a reference to the emuTime kept in de z80 is initialized */

	newVDPInterruptTime = starttime;                 // first interrupt ever
	newScreenChangeInterruptTime = starttime;
	newRenderScreenInterruptTime = starttime;      // sync this on the bottom of the screen
	newDebuggerInterruptTime = starttime;
	newMsxAudioInterruptTime = starttime;
	newAudioInterruptTime = starttime;
	cpuInterrupt = false;
	
	VDPInterruptEnable = true;
	screenChangeInterruptEnable = false;
	debuggerInterruptEnable = false;
	msxAudioInterruptEnable = false;
	
	clipboardStream = "";
    clipboardTimer = 0;

	DBERR("Emulator constructor...finished\n");
}

void Emulator::quit() {

    notPaused = false;
	running = false;
}


emuTimeType Emulator::getNextCPUInterruptTime() {
		return newVDPInterruptTime;	
}

void Emulator::scheduleVDPInterrupt(bool enable, emuTimeType interruptTime) {
    
        //DBERR("  Emulator::scheduleVDPInterrupt, enable: %i op %u\n", enable, interruptTime);
        //DBERR("  Emulator::cpuInterrupt: %u\n");
        
        VDPInterruptEnable = enable;
    	newVDPInterruptTime = interruptTime;
		scheduleNextInterrupt();
}

void Emulator::scheduleScreenChangeInterrupt(bool enable, emuTimeType interruptTime) {
        screenChangeInterruptEnable = enable;
  		newScreenChangeInterruptTime = interruptTime;
//  		DBERR("scheduleScreenChangeInterrupt: " << newScreenChangeInterruptTime << endl);
		scheduleNextInterrupt();
}

void Emulator::scheduleDebuggerInterrupt(bool enable, emuTimeType interruptTime) {
        debuggerInterruptEnable = enable;
  		newDebuggerInterruptTime = interruptTime;
//  		DBERR("newDebuggerInterruptTime: " << newDebuggerInterruptTime << endl);
		scheduleNextInterrupt();
}

void Emulator::scheduleRenderScreenInterrupt(emuTimeType interruptTime) {
  		newRenderScreenInterruptTime = interruptTime;
//  		DBERR("scheduleRenderScreenInterrupt: " << newRenderScreenInterruptTime << endl);
		scheduleNextInterrupt();
}

void Emulator::scheduleMsxAudioInterrupt(bool enable, emuTimeType interruptTime) {
        msxAudioInterruptEnable = enable;
  		newMsxAudioInterruptTime = interruptTime;
//  		DBERR("scheduleRenderScreenInterrupt: " << newRenderScreenInterruptTime << endl);
		scheduleNextInterrupt();
}

void Emulator::scheduleAudioInterrupt(bool enable, emuTimeType interruptTime) {
  		newAudioInterruptTime = interruptTime;
//  		DBERR("scheduleAudioInterrupt: " << newAudioInterruptTime << endl);
		scheduleNextInterrupt();
}



void Emulator::setCPUInterrupt(bool enable) {
        cpuInterrupt = enable;
        
        if (cpuInterrupt) { // first VDP interrupt can not occur NOW, but after 1 instruction
            if (newVDPInterruptTime <= cpu->emuTime) newVDPInterruptTime = cpu->emuTime + 1;
        }
        scheduleNextInterrupt(); 
      
}

unsigned long long OUR_SDL_GetTicks()
{
    unsigned long long offset = starttime;
    offset *= 1000;
    offset /= Z80::Instance()->cpuFrequency;

    return SDL_GetTicks()+offset;
}

inline void Emulator::scheduleNextInterrupt() {

    // Is is important to use signed types here, and compare offsets, not absolute times.
    // this way when emutime wraps, the comparisons are still valid.

    static unsigned int lastEmutime = 0;
    if (lastEmutime > cpu->emuTime)
    {
        unsigned long lTicks = OUR_SDL_GetTicks() & 0xffffffff;
        DBERR("Emutime WRAPPPED at %i ms (%i min).", lTicks, lTicks / 60000);        
    }
    lastEmutime = cpu->emuTime;

	cpu->nextInterrupt = newRenderScreenInterruptTime;
	interruptType = INT_RENDERSCREEN;
	int offset = newRenderScreenInterruptTime - cpu->emuTime;
	
    if (cpuInterrupt && VDPInterruptEnable) {
	    int newOffset = newVDPInterruptTime - cpu->emuTime;
		if (newOffset < offset) {
			cpu->nextInterrupt = newVDPInterruptTime;
			offset = newOffset;
			//DBERR("VDPInterruptEnabled and scheduled at: " << newVDPInterruptTime << endl);
			interruptType = INT_VDP;
		}
    }

    if (screenChangeInterruptEnable) {
		int newOffset = newScreenChangeInterruptTime - cpu->emuTime;
		if (newOffset < offset) {
			cpu->nextInterrupt = newScreenChangeInterruptTime;
			offset = newOffset;
			interruptType = INT_SCREENCHANGE;
		}
    }

    if (debuggerInterruptEnable) {
		int newOffset = newDebuggerInterruptTime - cpu->emuTime;
		if (newOffset < offset) {
			cpu->nextInterrupt = newDebuggerInterruptTime;
			offset = newOffset;
			interruptType = INT_DEBUGGER;
		}
    }
    
    if (msxAudioInterruptEnable) {
		int newOffset = newMsxAudioInterruptTime - cpu->emuTime;
		if (newOffset < offset) {
			cpu->nextInterrupt = newMsxAudioInterruptTime;
			offset = newOffset;
			//DBERR("MSXAUDIOInterruptEnabled and scheduled at: " << newMsxAudioInterruptTime << endl);
			interruptType = INT_MSXAUDIO;
		}
    }

	int newOffset = newAudioInterruptTime - cpu->emuTime;
	if (newOffset < offset) {
		cpu->nextInterrupt = newAudioInterruptTime;
		offset = newOffset;
		//DBERR("MSXAUDIOInterruptEnabled and scheduled at: " << newMsxAudioInterruptTime << endl);
		interruptType = INT_AUDIO;
	}

}

void Emulator::initialize() {
   
	/* to allow converting unicode codes to key-names */
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
	
#ifdef WIN32
    DBERR("WIN32 Drag&Drop support enabled.\n");
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    SDL_GetWMInfo(&wminfo);
    
    HWND hwnd = wminfo.window;
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    DragAcceptFiles(hwnd, TRUE);
    DBERR("DragAcceptFiles\n");
#endif
#ifndef WIN32
    DBERR("Drag and Drop not implemented for this platform!\n");
#endif    

	notPaused = true;
	commandMode = false;
	osdVisible = false;
    
    cpu = Z80::Instance();
    cpu->emuTime = starttime;

	vdp = V9938::Instance();
    AudioMixer::Instance()->initialize();
    
#ifdef PSG_SOUND_ON	
	AY38910 *psg = AY38910::Instance();
	AudioMixer::Instance()->addDevice(psg);        // TODO: do this in the device constructor?
#endif

#ifdef PPI_SOUND_ON
	PPISound *ppiSound = PPISound::Instance();
	AudioMixer::Instance()->addDevice(ppiSound);
#endif

#ifdef SCC_SOUND_ON	
	SCC *scc = SCC::Instance();
	AudioMixer::Instance()->addDevice(scc);
#endif

#ifdef MSXMUSIC_SOUND_ON	
	YM2413 *msxMusic = YM2413::Instance();
	AudioMixer::Instance()->addDevice(msxMusic);
#endif

#ifdef MSXAUDIO_SOUND_ON	
	MSXAudio *msxAudio = MSXAudio::Instance();
	AudioMixer::Instance()->addDevice(msxAudio);
#endif

    SDL_WM_SetCaption("Nowind MSX",0);  	
    
    Keyboard* keyboard = Keyboard::Instance();
    PPISound* ppisound = PPISound::Instance();
    Disassembler* disasm = Disassembler::Instance();

	cpu->initialize();
    reset();
}

void Emulator::reset() {
	cpu->reset();
    vdp->reset();       // zet de lastNormalInterruptTime op emuTime
}

ofstream ofs("timereport.txt"); 

void reportCyclesInit()
{
}

void reportCycles(emuTimeType emuTime, unsigned long long msRunning)
{
    static int point = 0;
    ofs << emuTime << ";" << msRunning << "\n";
    point++;
}

void Emulator::start() {

    DBERR("entering main loop\n");
//	SDL_WM_SetCaption("Nowind MSX",0);
	
	/* calculate the amount of T-states per millisecond the cpu should run at */
   	statesPerMilliSecond = cpu->cpuFrequency / 1000;	

    startTime = SDL_GetTicks();

	// todo: execute one instruction before we really start, so emuTime will not be zero and
	// we still won't need extra checks to prevent "division by zero"

//Debug::Instance()->INSTRUCTION_TRACING = true;

#ifdef ZEXALL_ON
    cpu->setupBdosEnv("zexall/zexdoc.com");
    while(1)
    {
        cpu->executeInstructions();
    }
#endif

    SDL_PauseAudio(0);	// start audio

	while(running) {
		while(notPaused) {

		scheduleNextInterrupt();
        cpu->executeInstructions();
		/* Interrupt reached, find out which one */

		/* neem de VALUE waarin het type interrupt staat */
		switch (interruptType) {
		case INT_VDP:

            switch (clipboardTimer) {
            case 0:
            {
        		int i = clipboardStream.length();
        		if (i > 0) {
        			clipboardKey = clipboardStream[0];
        			clipboardStream = clipboardStream.substr(1,i-1);
        			Keyboard::Instance()->modifyMatrixAscii(clipboardKey,true);
        			clipboardTimer = 1;
        		} 
        		break;
            }
            case 1:
                Keyboard::Instance()->modifyMatrixAscii(clipboardKey,false);
                clipboardTimer--;
                break;
            default:
                clipboardTimer--;
            }


            // DBERR("execute INT_VDP\n");
            cpu->intCPU(0xff);

			// TODO: misschien ergens anders neerzetten, voor fmpac logging
			// YM2413::Instance()->dumpChannelsBeat();
			break;

		case INT_RENDERSCREEN:
	 	{

#ifndef CONSOLE_DEBUGGING_ON
			/* handle key/sdl events */
			handle_key_and_sdl_events();

            //DBERR("execute INT_RENDERSCREEN\n");
            vdp->renderScreen(cpu->nextInterrupt);
#endif

			/* check if we're not going to fast */
			msTimeType lastestMs = OUR_SDL_GetTicks();
			msTimeType passedTime = (lastestMs-startTime);

            //DBERR("lastestMs: %u\n", lastestMs);
            //DBERR("passedTime: %u\n", passedTime);
	
			#ifndef ZEXALL_ON

			int slicesReleased = 0;
			
			reportCycles(cpu->emuTime, lastestMs);
	
			/* release time-slices until the average is back in range 
			 * on very fast machines this can be 100s of slices
			 */

            // todo: this will not work after emutime wraps! 'passedTime' will not be ok.
            // also, at startup and when the emutime wraps, the emulator will temporarily at full speed?
            // re-think this.
			while(cpu->emuTime > (statesPerMilliSecond * passedTime)) {
				SDL_Delay(1);

                lastestMs = OUR_SDL_GetTicks();
				passedTime = (lastestMs-startTime);
				slicesReleased++;
			}
//			if (slicesReleased > 0) DBERR("[%u]", slicesReleased);
			reportCycles(cpu->emuTime, lastestMs);
			#endif        
            
            if (autoPause) notPaused = false;           
            break;
	    }
		case INT_SCREENCHANGE:
            //DBERR("execute INT_SCREENCHANGE\n");
        	vdp->doScreenChange(cpu->nextInterrupt);   
			break;
		case INT_AUDIO:
            AudioMixer::Instance()->AudioInterrupt(cpu->nextInterrupt);
			break;

		default: 
            assert(false);
		}
	} // end of while(notPaused)

    DBERR("pause pressed!\n");
	while(!notPaused) {
		handle_key_and_sdl_events();
		if (!running) break;
  		SDL_Delay(17);
	}

    DBERR("unpause pressed!\n");

	} // end of while(running)

	/* classes Z80, VDP, AY38910 etc... will die on their own, they're singletons ;) */
	SDL_Quit();
}

void Emulator::handle_key_and_sdl_events() {

	SDL_Event event;
	while (SDL_PollEvent(&event)) {

		/* GLOBAL KEYS / EVENTS */
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {

			case SDLK_F6:
                if (osdVisible) {
                    // F6 = OSD uit, command-mode uit
                    commandMode = false;
                    osdVisible = false;
                } else {
                    // osd is niet aan en F6 wordt ingedrukt
                    osdVisible = true;
                    if ((event.key.keysym.mod & KMOD_SHIFT) == 0) {
                        // shift is niet ingedrukt
                        commandMode = true;
                    }    
                }  		
                OnScreenDisplay::Instance()->toggleOSD(osdVisible);
    			break;
			case SDLK_F7: {
                // PASTE clipboard
#ifdef WIN32
                if (!IsClipboardFormatAvailable(CF_TEXT)) {
  				    DBERR("No data on clipboard!\n");
                    break;
                }
                if (!OpenClipboard(NULL)) {
  				    DBERR("Failed to open clipboard!\n");
                    break;
                }
                    
                HGLOBAL clipboardHandle = GetClipboardData(CF_TEXT); 
                if (clipboardHandle != NULL) 
                { 
                    LPSTR data = (LPSTR) GlobalLock(clipboardHandle); 
                    if (data != NULL) 
                    { 
			   		 	Emulator::Instance()->clipboardStream = string(data);
                        //DBERR("data pasted: %s\n", data);
                        GlobalUnlock(clipboardHandle); 
                    } else {
                        DBERR("failed to obtain clipboard lock, no data pasted.\n");
                    }

                } else {
                    DBERR("GetClipboardData failed, no data pasted.\n");
                }                    
                CloseClipboard();  
#else
                DBERR("Paste from clipboard not supported on this platform!\n");
#endif                              
                }
                break;
                
			case SDLK_F8:
				 notPaused = !notPaused;
				 if (notPaused) {
                    Emulator::Instance()->startTime += (SDL_GetTicks() - startPauseTime);
                    autoPause = false;              // TODO: jan, dit heb ik erbij gezet, anders werkt het unpausen niet meer, maar is dit jouw bedoeling van autopause?
                 } else {
                    Emulator::Instance()->startPauseTime = SDL_GetTicks();
                    autoPause = true;
                }
    			break;
    			
			case SDLK_F9:
    			Emulator::Instance()->cpu->abortEmulator();
                break;
			case SDLK_F10:
				quit();
    			break;
   			case SDLK_F11:
    			Emulator::Instance()->reset();
    			break;
   			case SDLK_F12:
   				if (SDL_WM_GrabInput(SDL_GRAB_QUERY)==SDL_GRAB_ON) {
   					SDL_ShowCursor(SDL_ENABLE);
   					SDL_WM_GrabInput(SDL_GRAB_OFF);
   				} else {
   					SDL_ShowCursor(SDL_DISABLE);
   					SDL_WM_GrabInput(SDL_GRAB_ON);
   				}
   				break;
			default: 
                break;
			}
			break;
   		case SDL_QUIT:
   			quit();
			break;
        case SDL_MOUSEMOTION:
            if (mouseDragging) {
                GUI::Instance()->updateSelection(event.motion.x,event.motion.y);
                //DBERR("SDL_MOUSEMOTION\n");
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            GUI::Instance()->startSelection(event.motion.x,event.motion.y);
            mouseDragging = true;
            //DBERR("SDL_MOUSEBUTTONDOWN\n");
            break;
        case SDL_MOUSEBUTTONUP:
            {
            GUI::Instance()->endSelection(event.motion.x,event.motion.y);
            mouseDragging = false;
            //DBERR("SDL_MOUSEBUTTONUP\n");      
            break;
        }

#ifdef WIN32
   		case SDL_SYSWMEVENT: 
            switch(event.syswm.msg->msg) {
                
                case WM_DROPFILES:
                {
                    char fileName[512];
                    HDROP hDrop = (HDROP) event.syswm.msg->wParam;
                    int cnt = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);
            
                    if (cnt != -1) {
                        POINT pt;
                        DragQueryPoint(hDrop, &pt);
                        // DBERR("dropping on x,y: " << pt.x << ", " << pt.y << endl); 
                        DBERR("DragQueryFile count: %i\n", cnt);
                        for(int i = 0; i < cnt; i++) {
                            DragQueryFileA(hDrop, i, fileName, 512);
                            // DBERR("file "<< (i + 1)<< ": " << fileName<< endl);
                        }
                        DragFinish(hDrop);
                        if (pt.y < 228) {    
                            Media::Instance()->insertMedia(fileName,0);
                        } else {
                            SlotSelector::Instance()->usbInterface->insertDisk(0, fileName);
                            char caption[250];
                    		sprintf(caption, "Nowind MSX - USB - last inserted: %s", fileName);
                            SDL_WM_SetCaption(caption,0);
                            // Media::Instance()->insertMedia(fileName,1);
                        }                        
                    }   
                } 
                break;  

                default:
                      // DBERR("SDL_SYSWMEVENT,event.syswm.msg->msg: " << dec << event.syswm.msg->msg << endl);
                      break;
            }    
	       	break;
#endif
		default:
        	 //DBERR("SDL_EVENT,event.type: " << dec << (int) event.type << endl);
			break;
		}

		if (commandMode) {

    		// Handle keys and events in commandmode
			Command::Instance()->handleKeyEvent(&event);

		} else {

    		// Not in commandmode, so MSX has to handle this key or event
    		switch (event.type) {
      		case SDL_KEYDOWN:
	    	case SDL_KEYUP:
		 	     Keyboard::Instance()->modifyMatrix(event.key);
			     break;
            case SDL_JOYAXISMOTION:
			     Joystick::Instance()->axisMotion(&event);
			     break;
            case SDL_JOYBUTTONDOWN:
			     Joystick::Instance()->buttonDown(&event);
			     break;
	        case SDL_JOYBUTTONUP:
                 Joystick::Instance()->buttonUp(&event);
                 break;
		
            // TODO: gaat dit wel goed samen met het selecteren van text op het scherm???
            case SDL_MOUSEMOTION:
                 // even tijdelijk een break hier, anders komen er veel meldingen van events not handled.
                 break;
/*
			     mouseMotion(event);
			     break;
            case SDL_MOUSEBUTTONDOWN:
			     mouseButtonDown(event);
                 DBERR("--SDL_MOUSEBUTTONDOWN--\n");
			     break;
	       	case SDL_MOUSEBUTTONUP:
                 mouseButtonUp(event);
                 DBERR("--SDL_MOUSEBUTTONUP--\n");
                 break;
*/                                     
			default:
//				 DBERR("Event not handled by MSX!\n");
    			 break;
			}
    	}
    }
}

Emulator::~Emulator() {
	   DBERR("Emulator destroyed.\n");
}
