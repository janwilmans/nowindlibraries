//! V9938.h
#ifndef V9938_H
#define V9938_H

#include <iostream>
#include <string>
#include "EmulatorTester.h"

#include "msxtypes.h"

class V9938sprites;
class V9938commands;
class V9938renderer;
class GUI;

#define FH 0x01					// S#1

#define GRAPHICMODE4 0x06
#define GRAPHICMODE5 0x08
#define GRAPHICMODE6 0x0A
#define GRAPHICMODE7 0x0E

typedef void (V9938renderer::*FP_void_uint_uint)(unsigned int, unsigned int);

static const nw_byte registerMask[64] = {
    
	0x7E, 0x7B, 0x7F, 0xFF, 0x3F, 0xFF, 0x3F, 0xFF,
	0xFB, 0xBF, 0x07, 0x03, 0xFF, 0xFF, 0x07, 0x0F,
	0x0F, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7F, 0x3F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, // MSX2+
	0xFF, 0x01, 0xFF, 0x03, 0xFF, 0x01, 0xFF, 0x03,
	0xFF, 0x01, 0xFF, 0x03, 0xFF, 0x7F, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned int DISPLAYCHUNKS = 128;

// (59+27+100+102+56+1024)/6;	 SCANLINETIME = 1368 / (21.48 Mhz / 3.57 MHz)
static const unsigned int SCANLINETIME = 228;

class V9938 {

friend class EmulatorTester;
friend class V9938sprites;
friend class V9938commands;
friend class V9938renderer;
friend class Disassembler;

private:
    
		V9938();
  		V9938sprites    *sprites;
		V9938commands   *commands;
		V9938renderer	*renderer;
		GUI             *gui;

		// vdp registers
		nw_byte         statusReg[16];
		nw_byte         vdpReg[64];
		nw_byte         videoRam[128 * 1024];

		unsigned int	incVramAccessCounter();
		unsigned int	vramAccessCounter;
		nw_byte			vramLatch;
		nw_byte         dataLatch;
		bool            port1DataLatched;
		bool            port2DataLatched;
		void            writeRegister(nw_byte, nw_byte);

		// timing
		emuTimeType     &emuTime;        
        emuTimeType     nextNormalInterrupt;
        emuTimeType     nextLineInterrupt;
        emuTimeType		frameStartTime;

		unsigned int	firstDisplayLine;
		unsigned int	beginLineDisplayTime;
		unsigned int	frameTime;
		unsigned int	horizontalDisplayTime;

		// sprite overlay		
		unsigned int    spriteOverlay[256 * 256];
		
		// screen dimension variables		
		unsigned int	nextDisplayChunk;
		unsigned int	screenHeight;
		unsigned int	scanLines;             //!< amount of scanlines, calculated starting from the normal-interrupt 
		bool 			screenOn;

		bool			shiftedAddressLines;		
		
		// scaler variables
		unsigned int    SE_counter;
		unsigned int    SE_line[212];
        unsigned int    SE_hAdjust[212];
        unsigned int    SE_borderColor[212];
        bool            SE_resolution256[212];
        bool            lastResolution256;

		void			setScreenMode();		
		void			dumpScreenPart();
        void            calculateVDPtiming();
        void			calculateNextNormalInterrupt();
		void			calculateNextLineInterrupt();  

public:
		void            scheduleNextVDPInterrupt();
 		unsigned int    MSXscreen[544*214];
 		
		~V9938();
		static          V9938 * Instance();
		void			reset();
		
		void            renderScreen(emuTimeType t);
		void            doScreenChange(emuTimeType t);

		nw_byte         readVram();
		nw_byte         readStatusRegister();
		void            writeVram(nw_byte);
		void            writeControlRegister(nw_byte);
		void            writePaletteRegister(nw_byte);
		void            writeRegisterIndirect(nw_byte);

        bool 			switchFrequency();
		FP_void_uint_uint	dumpScreen;	

  int execInterrupt();		
		
		/* debug functies */
		
        void        		dumpPos(emuTimeType t);
        void        		dumpPos();


};

#endif

