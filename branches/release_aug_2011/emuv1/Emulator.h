//! Emulator.h
#ifndef EMULATOR_H
#define EMULATOR_H

#include "msxtypes.h"
#include "SDL.h"

#include <string>
#include <list>

class Z80;
class V9938;

/*!
 * The Emulator class contains the main-event-loop and it's setup/initialization
 */
class Emulator {

static const unsigned int INT_VDP = 0;
static const unsigned int INT_RENDERSCREEN = 1;
static const unsigned int INT_MSXAUDIO = 2;	// TODO: wordt niet meer gebruikt!
static const unsigned int INT_SCREENCHANGE = 3;
static const unsigned int INT_DEBUGGER = 4;
static const unsigned int INT_AUDIO = 5;


static unsigned int         interruptType;
static bool                 running;
static bool                 notPaused;
static bool                 commandMode;
static bool                 osdVisible;

private:
							Emulator();
			static void     handle_key_and_sdl_events();
			void            scheduleNextInterrupt();

            emuTimeType		newVDPInterruptTime;
            emuTimeType		newScreenChangeInterruptTime;	
            emuTimeType     newRenderScreenInterruptTime;
            emuTimeType     newDebuggerInterruptTime;
            emuTimeType     newMsxAudioInterruptTime;
            emuTimeType     newAudioInterruptTime;

            bool            VDPInterruptEnable;
            bool            screenChangeInterruptEnable;
            bool            cpuInterrupt;
            bool            debuggerInterruptEnable;
            bool            msxAudioInterruptEnable;

			/* keep this a double for accuracy! */
			double          statesPerMilliSecond;

//	        msTimeType      diffMeting;
                	
	        msTimeType      startTime;
static      msTimeType      startPauseTime;
static      bool            autoPause;
	        
static      bool            mouseDragging;
			std::string		clipboardStream;  
			int             clipboardTimer;
			char 		    clipboardKey;  
      
public:
            Z80 *           cpu;
			V9938 *			vdp;
            void            setCPUInterrupt(bool);

            void    		scheduleVDPInterrupt(bool enable, emuTimeType);
			void			scheduleScreenChangeInterrupt(bool enable, emuTimeType);
			void			scheduleDebuggerInterrupt(bool enable, emuTimeType);
			void            scheduleRenderScreenInterrupt(emuTimeType);
			void            scheduleMsxAudioInterrupt(bool enable, emuTimeType);
			void            scheduleAudioInterrupt(bool enable, emuTimeType);
			
			emuTimeType		getNextCPUInterruptTime();
			
            void     		setPause(bool);
static      Emulator 		* Instance();
			void            initialize();
			void            start();
			void            reset();
static 		void      		quit();
			~Emulator();
};

#endif
