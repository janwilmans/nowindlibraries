// V9938.ccp

#include "stdio.h"

#include "V9938.h"
#include "Debug.h"
#include "V9938sprites.h"
#include "V9938commands.h"
#include "V9938renderer.h"
#include "GUI.h"

using namespace std;

#define CURRENTSCANLINE ((unsigned int)((emuTime - frameStartTime) / SCANLINETIME))

V9938::V9938() : emuTime(Z80::Instance()->emuTime) {

	DBERR("V9938 constructor...\n");

	sprites = new V9938sprites(this);
	commands = new V9938commands(this);
	renderer = V9938renderer::Instance(); // new V9938renderer(this);
	renderer->setVdp(this);	
	gui = GUI::Instance();
	gui->setVdp(this);
	
	dumpScreen = &V9938renderer::dumpGraphic4; // TODO: default??!!!
    
	DBERR("V9938 constructor...finished\n");
}

int V9938::execInterrupt() {
     
    DBERR("execInterrupt\n");
    return 123;
}

V9938 * V9938::Instance() {

		/* implies singleton class */
		static V9938 deInstantie;
		return &deInstantie;
}

V9938::~V9938() {

	DBERR("V9938 destructor\n");

	delete sprites;
	delete(commands);
	
    // The renderer and Gui are singletons, they'll die by themselves
	DBERR("V9938 destroyed.\n");
}

void V9938::reset() {

	DBERR("RESET V9938\n");
   	port1DataLatched = false;
	port2DataLatched = false;
	shiftedAddressLines = false;

 	for (int i=0;i<16;i++) statusReg[i] = 0xff;
	statusReg[1] = 0; 		// 4 voor V9958
	statusReg[2] = 128;     // CE = ready, TR = 1 (ready)
	for (int i=0;i<64;i++) vdpReg[i] = 0;
	
	writeRegister(0,0); // chipstms9918.pdf (2.1.7)
	writeRegister(1,0);
		
	vramAccessCounter = 0;

 	nextDisplayChunk = 0;
 	screenOn = true;				// indicated that changes to mode-registers should trigger dumpScreens
 	
  	SE_counter = 0;
  	SE_line[0] = 0;
  	SE_hAdjust[0] = 7;
  	SE_borderColor[0] = 0;
  	SE_resolution256[0] = false; 
 	
 	// TODO: controleren of de default wel 50 Hz, 192 lines is bij een reset?
 	writeRegister(9,2);

	nextNormalInterrupt = emuTime;
	nextLineInterrupt = emuTime;
	frameStartTime = emuTime;
	
	calculateVDPtiming();
	  	
	sprites->reset();
	commands->reset();
	renderer->reset();
	gui->reset();
}    

unsigned int V9938::incVramAccessCounter() {

    unsigned int address;
	address = (vdpReg[14] << 14) | vramAccessCounter++;
    vramAccessCounter &= 0x3fff;
	
	// increment vdp14 only in MSX2 screens
    if ((vramAccessCounter == 0) && ((vdpReg[0] & 0x0c) != 0)) {
 	    vdpReg[14] = (vdpReg[14] + 1) & 7;
 	}
	if (shiftedAddressLines) address = (address >> 1) | ((address & 1)<<16);
    return address;
}


nw_byte V9938::readVram() {
    
    // port #0 (0x98)
	port1DataLatched = false;
	nw_byte lastRead = vramLatch;
    vramLatch = videoRam[incVramAccessCounter()];
	return lastRead;
}

void V9938::writeVram(nw_byte value) {
	//DBERR("vramWrite: " << hex << vramAccessCounter << " " << (int)value << endl);
    // port #0 (0x98)
    port1DataLatched = false;    
    videoRam[incVramAccessCounter()] = value;
    vramLatch = value;
}

nw_byte V9938::readStatusRegister() {

    // port #1 (0x99)
	nw_byte value;
	
	port1DataLatched = false;
	switch (vdpReg[15]) {
	case 0:
// 		DBERR("read S#0\n");
		value = statusReg[0];
				
        if (nextNormalInterrupt <= emuTime) {
		    value |= 128;
    		calculateNextNormalInterrupt();
      		scheduleNextVDPInterrupt();
        }
   		statusReg[0] &= 0x1F; 	// reset F, 5S and C
		break;

	case 1:
// 		DBERR("read S#1\n");
  		value = statusReg[1];
        if (nextLineInterrupt <= emuTime) {
 		    value |= 1;
            statusReg[1] &= 0xfe;  			// reset line interrupt flag
            calculateNextLineInterrupt();
            scheduleNextVDPInterrupt();
        }
		break;

	case 2: {
 	    value = statusReg[2] | 0x0c;
 	    value &= 0x9F;	// reset HR and VR
        /* Despite of its name, the Vertical Retrace Flag is set for the entire time while 
           lower and upper screen borders are drawn and during actual vertical retrace. 
           The Horizontal Retrace Flag becomes set at the end of each scanline - including 
           hidden/dummy scanlines during vertical retrace.
        */

		// HR (bit 5)
        unsigned int positionInCurrentLine = (emuTime - frameStartTime) % SCANLINETIME;
        if (positionInCurrentLine >= horizontalDisplayTime) value |= 0x20;
/*        
		if ((positionInCurrentLine > 190) || (positionInCurrentLine < 20)) {
  		   value |= 0x20;
        }
*/
		//DBERR("JANSNAPTNIETWATIKWIL: " << dec << (value & 0x20) << endl);
		
		// VR (bit 6), 
		unsigned int i = CURRENTSCANLINE;
		if ((i < firstDisplayLine) || (i > (firstDisplayLine+screenHeight))) value |= 0x40;
		}
		break;
    
	case 3: value = sprites->getCollisionCoordinate(3); break;
	case 4: value = sprites->getCollisionCoordinate(4) | 0xfe; break;
	case 5: value = sprites->getCollisionCoordinate(5); break;
	case 6: value = (sprites->getCollisionCoordinate(6) & 1) | (statusReg[2] & 2) | 0xfc; break;

	case 7: value = commands->transferLmcm(); break;
	case 8: value = commands->getBorderCoordinate() & 255; break;
	case 9: value = (commands->getBorderCoordinate() >> 8) | 0xfe; break;
	
 	default: value = 0xFF;
	}
	return value;
}

void V9938::writeControlRegister(nw_byte value) {

    // port #1 (0x99)
	if (port1DataLatched) {

		port1DataLatched = false;		
		if (value & 0x80) {
			// assert ((value & 0x40) == 0); // space manbow/undeadline
			// when bit 6 is set, no write to register of vram access counter occurs
			if ((value & 0x40) == 0) writeRegister(value & 0x3f, dataLatch);
		} else {
 		    vramAccessCounter = ((value & 0x3f) << 8) | dataLatch;
			if ((value & 0x40) == 0) readVram(); // vram read ahead
		}
	} else {
		port1DataLatched = true;
		dataLatch = value;
	}
}

void V9938::writePaletteRegister(nw_byte value) {

    // port #2 (0x9a)
	if (port2DataLatched) {
 		renderer->writePalette(vdpReg[16]++, dataLatch, value);
     	vdpReg[16] &= 15;
	} else {
 	    dataLatch = value;
	}
	port2DataLatched =!port2DataLatched;
}

void V9938::writeRegisterIndirect(nw_byte value) {

    // port #3 (0x9b)
	dataLatch = value;	
    unsigned int index = vdpReg[17] & 0x3f;
    if (index != 17) writeRegister(index, value);
	if ((vdpReg[17] & 0x80) == 0) {
 	    vdpReg[17] = (++index) & 0x3f;
	}
}

void V9938::writeRegister(nw_byte r, nw_byte value) {

	nw_byte diff = vdpReg[r] ^ value;

	switch (r) {
	case 0: 
        if (diff & 0x0E) {   // screen change 
	        emuTimeType screenChangeTime = frameStartTime + (CURRENTSCANLINE*SCANLINETIME) + horizontalDisplayTime;
   	        Emulator::Instance()->scheduleScreenChangeInterrupt(true, screenChangeTime);
        } 

	    vdpReg[0] = value & registerMask[0];
	    
        // check if a line-interrupt should be scheduled
	    if (diff & 16) {
      		if (vdpReg[0] & 16) {	 // line interrupt is enabled
      		   if (nextLineInterrupt <= emuTime) {		// check if the line interrupt was already pending
			   	   nextLineInterrupt = emuTime;
			   } else {
  		 	  	   calculateNextLineInterrupt();
			   }
            }	
        }
	    scheduleNextVDPInterrupt();
		break;
	case 1: 
		// TODO: nu worden er ook screen ON/OFF changes gescheduled in the retrace ! (buiten het beeld) 
		
	    renderer->setModeRegister1(value);
	    if (diff & 0x58) {
			//todo: screenChangeTime kan nooit frameStartTime bevatten, dit is fout
	        unsigned int screenChangeTime = frameStartTime + (CURRENTSCANLINE*SCANLINETIME) + horizontalDisplayTime;
   	        Emulator::Instance()->scheduleScreenChangeInterrupt(true, screenChangeTime);
        }    
	    
	    vdpReg[1] = value & registerMask[1];

        // check if a normal-interrupt should be scheduled
	    if (diff & 32) {
      		if (vdpReg[1] & 32) {	 // normal interrupt is enabled
          		if (nextNormalInterrupt <= emuTime) {		// check if the normal interrupt was already pending
       			    nextNormalInterrupt = emuTime;
                } else {
	  	   		    // hier de nextNormalInterrupt berekenen
					calculateNextNormalInterrupt();
                }
            }	
        }
	    scheduleNextVDPInterrupt();	
		break;
    case 2: 
  	    if (screenOn) dumpScreenPart();
		vdpReg[2] = value & registerMask[2];      // registermask is al 0x7F 
        renderer->setNameTable(value & 0x7F);   // 0x7F = %01111111   (jan: waarom nog een keer?)
        break;        
	case 3: 
	    vdpReg[3] = value & registerMask[3];
	    renderer->setColorTableLow(value); 
	    break;
	case 4: 
	    vdpReg[4] = value & 0x3F;
		renderer->setPatternGeneratorTable(vdpReg[4]); 
		break;
    case 5: 
        vdpReg[5] = value & registerMask[5];
    	sprites->setAttributeTableLow(value); 
    	break;
    case 6: 
    	vdpReg[6] = value & registerMask[6];
    	sprites->setPatternGeneratorTable(value); 
    	break;
	case 7:	
 		dumpScreenPart();	// dump, even is the screen is off!
	    vdpReg[7] = value;
		renderer->setBorderColor(value); 
		break;
	case 8: 
	    vdpReg[8] = value;
     	if (diff & 0x20) renderer->setModeRegister2(value); 
     	break;
	case 9: 
	    vdpReg[9] = value & registerMask[9];
     	gui->setScreenHeight(value & 0x80 ? 212:192);
//     	if (value & 0x08) assert(false); // interlace
//     	if (value & 0x04) assert(false); // EO
     	break;
	case 10: 
     	vdpReg[10] = value & registerMask[10];
 		renderer->setColorTableHigh(value & 7); 
   		break;
    case 11: 
     	vdpReg[11] = value & registerMask[11];
     	sprites->setAttributeTableHigh(value); 
     	break;
     	
	case 12: renderer->setBlinkColor(value); break;
    case 13: renderer->setBlinkPeriod(value); break;
        
    case 14: vdpReg[14] = value & 7; break; 	// vram access base register high
    case 15: vdpReg[15] = value & 15; break; 	// status register pointer
    case 16:
 	    vdpReg[16] = value & 15;				// pallete register pointer
 	    port2DataLatched = false;
 		break;
    case 17: vdpReg[17] = value & 0xbf; break;	// control register pointer

    case 18:    	
     	vdpReg[18] = value & 255;
     	if (diff & 0xf0) gui->setVerticalAdjust((value >> 4) ^ 7);
     	if (diff & 0x0f) {

            unsigned int lastLine = SE_line[SE_counter];
            unsigned int currentScanLine = CURRENTSCANLINE;
            unsigned int displayLine = 0;
            
            if (currentScanLine >= firstDisplayLine) {
                displayLine = currentScanLine - firstDisplayLine;
            }    
            
            if (displayLine > lastLine) { 
                SE_counter++;
                SE_line[SE_counter] = displayLine;
                SE_resolution256[SE_counter] = SE_resolution256[SE_counter-1];
                SE_borderColor[SE_counter] = SE_borderColor[SE_counter-1];
            }    
            SE_hAdjust[SE_counter] = (value&15) ^ 7;
        }    
        break;
        
	case 19: 
     	vdpReg[19] = value & registerMask[19];
        calculateNextLineInterrupt();
        scheduleNextVDPInterrupt();
	break;
		
	case 20: vdpReg[20] = value; break; // color burst registers
 	case 21: vdpReg[21] = value; break;
 	case 22: vdpReg[22] = value; break;
 	
	case 23: 
        //DBERR("r23 change \n");
   	    if (screenOn) dumpScreenPart();
	    vdpReg[r] = value & registerMask[r];
        renderer->setDisplayOffset(value);
        calculateNextLineInterrupt();
        scheduleNextVDPInterrupt();
    break;
	       
	// MSX 2+
  	case 25: 
  	    if (screenOn) dumpScreenPart();			// actually should check for changes to bit 0 (MSK)

		vdpReg[25] = value & registerMask[25];
		setScreenMode();
		DBERR("YAE/YJK status: %u\n", vdpReg[25]&0x18);
	// break?
    case 26:
    case 27:
    	vdpReg[r] = value & registerMask[r];
        DBERR("MSX 2+ vdpregisters geschreven!\n");	
	break;
		
	// command registers
    case 32:    
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46: commands->writeCommandRegister(r, value); break;
	default:
 		vdpReg[r] = value & registerMask[r];
	}
}

void V9938::setScreenMode() {
    
    //DBERR(" $$$ V9938::setScreenMode: ");

    bool r256 = true;
    int spriteMode = 0;
    
    dumpPos(emuTime);
    
    if (vdpReg[1] & 0x40) {

		shiftedAddressLines = false;
   	    screenOn = true;
        switch (vdpReg[0] & 0x0E) {
        case 0x00:
            DBERR("vdpReg[1] & 0x18: 0x%04X\n", vdpReg[1] & 0x18);
            switch (vdpReg[1] & 0x18) {
            case 0x00: dumpScreen = &V9938renderer::dumpGraphic1; spriteMode = 1; break;
            case 0x08: dumpScreen = &V9938renderer::dumpMultiColor; spriteMode = 1; break;
            case 0x10: dumpScreen = &V9938renderer::dumpText1; break;
            default: DBERR("Unknown screen mode!\n");
            }
            break;
            
        case 0x02: dumpScreen = &V9938renderer::dumpGraphic2; spriteMode = 2; break; // graphic3!
        case 0x06: dumpScreen = &V9938renderer::dumpGraphic4; spriteMode = 2; break;
        case 0x08: dumpScreen = &V9938renderer::dumpGraphic5; spriteMode = 3; r256 = false; break;
        case 0x0A:
		    dumpScreen = &V9938renderer::dumpGraphic6; spriteMode = 4;
			r256 = false;
			shiftedAddressLines = true;
			break;
        case 0x04:
            if (vdpReg[1] & 0x10) {
                dumpScreen = &V9938renderer::dumpText2; r256 = false; 
            } else {
                dumpScreen = &V9938renderer::dumpGraphic2; spriteMode = 1;
            }
            break;

        case 0x0E:
			 shiftedAddressLines = true;
             if (vdpReg[25] & 8) {
                 dumpScreen = &V9938renderer::dumpGraphic7yjk; // TODO: spriteMode = ???
             } else {
                 dumpScreen = &V9938renderer::dumpGraphic7; spriteMode = 5;            
        	 }   	
             break;
             
        default: DBERR("Unknown screen mode!\n");
        }
        
    } else {
	  	// TODO: shiftedAddressLines hier ook zetten!!!!
        screenOn = false;
        r256 = lastResolution256;
        dumpScreen = r256 ? (&V9938renderer::dumpDisabledScreen256) : (&V9938renderer::dumpDisabledScreen512);
    }

    if (vdpReg[8] & 2) spriteMode = 0;	// disable sprites
    
    if (r256 != lastResolution256) {
        
        unsigned int lastLine = SE_line[SE_counter];
        unsigned int currentScanLine = CURRENTSCANLINE;
        unsigned int displayLine = 0;
            
        if (currentScanLine >= firstDisplayLine) {
            displayLine = currentScanLine - firstDisplayLine;
        }    
        if (displayLine > lastLine) {
            SE_counter++;
            SE_line[SE_counter] = displayLine;
            SE_hAdjust[SE_counter] = SE_hAdjust[SE_counter-1];
            SE_borderColor[SE_counter] = SE_borderColor[SE_counter-1];
        }
        SE_resolution256[SE_counter] = r256;
                
        lastResolution256 = r256;
    }    
}    

void V9938::calculateVDPtiming() {

    // VDP Timing constants
    static const unsigned int vdp_vsync = 3;
//    static const unsigned int vdp_bottomerase = 3;
    
    // todo: create a interlace-flag of odd and even frames
    static const unsigned int vdp_top_erase = 13;
    
    unsigned int vdp_top_border = 16;

    if (vdpReg[9] & 0x02) {     // PAL
        scanLines = 313;    
        vdp_top_border += 27;       
    } else {                    // NTSC
        scanLines = 262;
    } 
    
   	if (vdpReg[9] & 0x80) {
        screenHeight = 212;
    } else {
        screenHeight = 192;
        vdp_top_border += 10;       
    }

   	firstDisplayLine = vdp_vsync + vdp_top_erase + vdp_top_border;
	frameTime = (scanLines * SCANLINETIME);
   	
   	if ((vdpReg[1] & 0x10) == 0x10) {
	   beginLineDisplayTime = 0;
	   horizontalDisplayTime = 160;
    } else {
	   beginLineDisplayTime = 0;
	   horizontalDisplayTime = 171;
    }
}

/*! \brief processes the normal VDP interrupt if it is enabled generates a Z80 interrupt
 * 	
 * 	VDPreg[1] bit 5 is "1" if the normal interrupt is enabled
 */
void V9938::renderScreen(emuTimeType screenTime) {

    unsigned int lastChunk = screenHeight * DISPLAYCHUNKS;
	unsigned int chunksToDump = lastChunk - nextDisplayChunk;

//	DBERR("CURRENTSCANLINE: " << CURRENTSCANLINE << endl);
//	DBERR("nextDisplayChunk: " << nextDisplayChunk << endl);
//	DBERR("lastChunk: " << lastChunk << endl);
	assert (nextDisplayChunk <= lastChunk);
    if (nextDisplayChunk < lastChunk) {
    		(*renderer.*dumpScreen)(nextDisplayChunk, chunksToDump);
	}
 	nextDisplayChunk = 0;
/* 	
    SE_line[SE_counter + 1] = screenHeight;
    for (unsigned int i=0;i<SE_counter+1;i++) {
        if (SE_resolution256[i]) {
            gui->simple2x(SE_line[i + 1] - SE_line[i], SE_hAdjust[i]);
        } else {
            gui->simple512(SE_line[i + 1] - SE_line[i], SE_hAdjust[i]);
        }    
    }
*/
        if (SE_resolution256[SE_counter]) {
            gui->simple2x(screenHeight, SE_hAdjust[SE_counter]);
        } else {
            gui->simple512(screenHeight, SE_hAdjust[SE_counter]);
        }    



    SE_resolution256[0] = SE_resolution256[SE_counter];
    SE_hAdjust[0] = SE_hAdjust[SE_counter];
    SE_borderColor[0] = SE_borderColor[SE_counter];
    SE_line[0] = 0;
    SE_counter = 0; 	

 	gui->afterScaler();
	renderer->afterCallScaler();
	
	calculateVDPtiming();
  	frameStartTime = screenTime;
   	Emulator::Instance()->scheduleRenderScreenInterrupt(screenTime + frameTime);
   	statusReg[2] ^= 2;	// display field flag (EO)
}

void V9938::calculateNextNormalInterrupt() {

	 nextNormalInterrupt = frameStartTime + ((firstDisplayLine + screenHeight) * SCANLINETIME);
	 if (nextNormalInterrupt < emuTime) nextNormalInterrupt += frameTime;
}

void V9938::calculateNextLineInterrupt() {

    unsigned int lineNumber = ((256 + vdpReg[19] - vdpReg[23]) % 256) + firstDisplayLine;
    nextLineInterrupt = frameStartTime + (lineNumber * SCANLINETIME) + horizontalDisplayTime + 32;
    if (nextLineInterrupt < emuTime) nextLineInterrupt += frameTime;
}

void V9938::scheduleNextVDPInterrupt() {

    bool IE0 = (vdpReg[1] & 32) == 32;
    bool IE1 = (vdpReg[0] & 16) == 16;
   
    if (IE0 || IE1) {
        
        if (IE0 && IE1) {
            if (nextNormalInterrupt < nextLineInterrupt) {
                Emulator::Instance()->scheduleVDPInterrupt(true, nextNormalInterrupt);
            } else {
                Emulator::Instance()->scheduleVDPInterrupt(true, nextLineInterrupt);
            }
        } else {
            if (IE0) {
                Emulator::Instance()->scheduleVDPInterrupt(true, nextNormalInterrupt);
            } else {
                Emulator::Instance()->scheduleVDPInterrupt(true, nextLineInterrupt);
            }
        }
    } else {
        Emulator::Instance()->scheduleVDPInterrupt(false, 3);
    }
}

void V9938::doScreenChange(emuTimeType scheduledChangeTime) {
    
    // scheduledChangeTime is de tijd waarop deze screenChange had moeten gebeuren,
    // het kan zijn dat er nu al emuTime verstreken is doordat niet zo exact gescheduled
    // kan worden.

    dumpScreenPart();
    
    Emulator::Instance()->scheduleScreenChangeInterrupt(false, 0);   
    setScreenMode();    
}

void V9938::dumpScreenPart() {

//    DBERR(" $$$ V9938::dumpScreenPart: ");
    dumpPos();
    unsigned int currentChunk, chunksToDump;
	unsigned int i = CURRENTSCANLINE;
//	DBERR("CURRENTSCANLINE " << i << endl);
//	DBERR("scanLines " << scanLines << endl);
    assert(i <= scanLines); // gedurende teveel tijd geen dumpScreenPart aangeroepen

    if ((i >= firstDisplayLine) && (nextDisplayChunk < (screenHeight * DISPLAYCHUNKS))) {
	    
		if (i >= (firstDisplayLine + screenHeight)) {
  		    chunksToDump = (screenHeight * DISPLAYCHUNKS) - nextDisplayChunk;
		} else {
    	
			unsigned int timeInCurrentLine = (emuTime - frameStartTime) % SCANLINETIME;
			currentChunk = (i - firstDisplayLine) * DISPLAYCHUNKS;
			
			if (timeInCurrentLine < horizontalDisplayTime) {
				// within display time of current line
				currentChunk += ((timeInCurrentLine - beginLineDisplayTime) * DISPLAYCHUNKS) / 171;
		 	} else {
	   		    currentChunk += DISPLAYCHUNKS;
			}
			chunksToDump = currentChunk - nextDisplayChunk;
		
		}		
		assert((nextDisplayChunk + chunksToDump) <= (screenHeight * DISPLAYCHUNKS));
		assert(chunksToDump <= (screenHeight * DISPLAYCHUNKS));

	    if (chunksToDump) {
    		(*renderer.*dumpScreen)(nextDisplayChunk, chunksToDump);
    		nextDisplayChunk += chunksToDump;
   		}  	
    }  		         	
}

void V9938::dumpPos() {
	dumpPos(emuTime);
}

void V9938::dumpPos(emuTimeType t) {

    int fase = frameStartTime % frameTime;
    emuTimeType localTime = t - fase;

    int frameNr = localTime / frameTime;    
    int lineNr = (localTime % frameTime) / SCANLINETIME;    
   
//    lineNr -= firstDisplayLine;    
    int pos = (localTime % frameTime) % SCANLINETIME;

    char temp[250];
    sprintf(temp,"[%i,%i_%i]",frameNr, lineNr, pos);
//    DEBUGERROR(temp);
}

bool V9938::switchFrequency() {
    vdpReg[9] ^= 2;
    return (vdpReg[9] & 2) != 0;
}
