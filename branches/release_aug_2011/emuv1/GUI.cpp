#include "Debug.h"
#include "video/V9938.h"
#include "video/V9938renderer.h"
#include "GUI.h"
#include "osd/OnScreenDisplay.h"

#include "SDL_syswm.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;

unsigned int time1a;
unsigned int time1d;
unsigned int time2a;
unsigned int time2d;

unsigned long long time1;
unsigned long long time2;

unsigned long long speedaverage;
unsigned long long speedcounter;
unsigned int *asmp1;
unsigned int *asmp2;
unsigned int *msxscr;
unsigned int asmeee;
unsigned int asmborder;
unsigned int asmhadjust;
unsigned int asmcount;

GUI::GUI() {

		speedaverage = speedcounter = 0;

		DBERR("GUI constructor...\n");

		lastResolution = 0;
		setResolution(2); 
		selecting = false;
       
		/* On hardware that supports double-buffering, this function sets up a flip and returns.
		The hardware will wait for vertical retrace, and then swap video buffers before the next
		video surface blit or lock will return. On hardware that doesn't support double-buffering,
		this is equivalent to calling SDL_UpdateRect(screen, 0, 0, 0, 0) */

		blendmask = 0x010101;
/*
	    string disksImage = Debug::Instance()->getPath() + "disks.bmp";
		SDL_Surface *temp = SDL_LoadBMP(disksImage.c_str());
		if (temp == NULL) {
            DBERR("GUI: LoadBmp: " << disksImage << " not found!\n");
            exit(1);
        }    
  		bmpdisk = SDL_DisplayFormat(temp);
  		SDL_FreeSurface(temp);

  		int colorkey = SDL_MapRGB(screen->format, 255, 255, 255);
  		SDL_SetColorKey(bmpdisk, SDL_SRCCOLORKEY | SDL_RLEACCEL, colorkey);
  		SDL_SetAlpha(bmpdisk, SDL_SRCALPHA | SDL_RLEACCEL, 175);
*/ 
		osd = OnScreenDisplay::Instance();
				
		selectedScaler = SIMPLE;
		bufp1 = screenBase;

//        SDL_Rect Con_rect;
//    	Con_rect.x = Con_rect.y = 0;
//        Con_rect.w = Con_rect.h = 300;
//    	if((Consoles[0] = CON_Init("ConsoleFont.bmp", Screen, 100, Con_rect)) == NULL) assert(false);
        
  		DBERR("GUI constructor...finished\n");
}

GUI::~GUI() {

		DBERR("GUI destructor\n");
		// destructor

		// only free the surface if to was loaded
//		SDL_FreeSurface(fontImg);
		DBERR("GUI destroyed.\n");
}

GUI * GUI::Instance() {

		/* implies singleton class */
		static GUI deInstantie;
		return &deInstantie;
}

void GUI::setVdp(V9938 *theVdp) {

	vdp = theVdp;
	MSXscreen = vdp->MSXscreen;
}
    
void GUI::reset() {

	DBERR("RESET GUI\n");
	screenHeight = 192;
	verticalAdjust = 7;
	borderColor = 0x000000;
	borderChanged = true;
	
	currentMSXscreenPosition = MSXscreen;
	//screenBase = (Uint32 *)screen->pixels;
	//bufp1 = screenBase;
}    

void GUI::fullscreenToggle() {
	
	/* The semantics of SDL_WM_ToggleFullScreen() are that switching between fullscreen and windowed mode is transprent to the application. The display pixels pointer does not change, the display depth does not change, etc. This cannot be guaranteed on Windows. However, there is a simple method you can use to change between fullscreen and windowed mode: 
        flags ^= SDL_FULLSCREEN;
        screen = SDL_SetVideoMode(..., flags); 
    */
	
//	SDL_FillRect(screen,0,0);	
//	screen = SDL_SetVideoMode(resolution*256, resolution*212+(resolution*16), 32, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
//	SDL_FillRect(screen,0,0);	
}

/* SDL standaard method to lock the screen, use before updating a screen */
void GUI::Slock(SDL_Surface *screen)
{
		if (SDL_MUSTLOCK(screen)) {
				if (SDL_LockSurface(screen) < 0) {
						 return;
				}
		}
}

/* SDL standaard method to unlock the screen, use after updating a screen */
void GUI::Sulock(SDL_Surface *screen) {
		if (SDL_MUSTLOCK(screen)) {
				SDL_UnlockSurface(screen);
		}
}

void GUI::setScaler(int scaler) {

	selectedScaler = scaler;
	SDL_FillRect(screen,0,0);
	lastResolution = resolution;
	
	switch (selectedScaler) {
	case SIMPLE:
  		scalerName = "SIMPLE 2X";
		resolution = 2;
		break;
	case SCALE2X:
		scalerName = "SCALE 2X";
		resolution = 2;
		break;
	case SCALE2X_SAI: 
		scalerName = "SCALE 2X SAI";
		resolution = 2;
		break;
	case SCALE2X_S_2XSAI: 
		scalerName = "SCALE SUPER 2X SAI";
		resolution = 2;
		break;
	case SCALE_S_EAGLE: 
		scalerName = "SCALE SUPER EAGLE";
		resolution = 2;
		break;
	case SIMPLE3X:
	    scalerName = "SIMPLE 3X";
	    resolution = 3;
	    break;
	case SCALE3X: 
		scalerName = "SCALE 3X";
		resolution = 3;
		break;
	case SCALE3XSAI: 
		scalerName = "SCALE 3X SAI";
		resolution = 3;
		break;
	default:
		DBERR("unknown scaler mode, using default 2x");
		scalerName = "UNKNOWN";
		resolution = 2;
		break;
	}

}

int GUI::getScaler() {

	return selectedScaler;
}

void GUI::scaler(unsigned int* screenStream) {

	setResolution(resolution);

	/* vies, maar anders wordt de osd niet geupdate */
	setScaler(selectedScaler);

	//drawBorder(screenStream);
		
	switch (selectedScaler) {
 	case SIMPLE3X:
 		simple3x(screenStream);
 		break;
	case SCALE2X: 
		scale2x(screenStream);
		break;
	case SCALE2X_SAI: 
		scale2xSaI(screenStream);
		break;
	case SCALE2X_S_2XSAI: 
		scaleSuper2xSaI(screenStream);
		break;
	case SCALE_S_EAGLE: 
		scaleSuperEagle(screenStream);
		break;
	case SCALE3X: 
  		scale3x(screenStream);
		break;
	case SCALE3XSAI: 
		scaleSuper3xSaI(screenStream);
		break;
	default:
  		simple2x(212,0); // askjdkajdkakldjlkas TODO
		break;
	}

//	if (osdEnabled) OSDstrings();

#ifndef CONSOLE_DEBUGGING_ON
	SDL_Flip(screen);
#endif
}

void GUI::setResolution(unsigned int res) {

	/* 	In deze routine geen V9938 methodes aanroepen ! -> endless loop */

    char driverName[20];    
    const SDL_VideoInfo *info;
    resolution = res;
	if (lastResolution != res) {

#ifndef CONSOLE_DEBUGGING_ON
		DBERR("GUI Hardware surface: %ux%u\n", res*256, res*212+(res*16));
		screen = SDL_SetVideoMode(res*(256+16), res*212+(res*16), 32, SDL_SWSURFACE|SDL_DOUBLEBUF);
        SDL_VideoDriverName(driverName, 20);
        DBERR("SDL_VideoDriveName: %s\n", driverName);
		info = SDL_GetVideoInfo();
        
        char result[30] = "no";
        if (info->hw_available == 1) {
            sprintf(result, "yes (%u kB)", info->video_mem);
        }
        DBERR("Hardware surfaces available: %s\n" , result);
		DBERR("Window manager available: %s\n", info->wm_available == 1 ? "yes":"no" );
		
        DBERR("Hardware blit acceleration\n");
        DBERR(" - hardware to hardware: %s\n", info->blit_hw == 1 ? "yes":"no");
		DBERR(" - software to hardware: %s\n", info->blit_sw == 1 ? "yes":"no");
        DBERR(" - hardware to hardware (colorkey): %s\n", info->blit_hw_CC == 1 ? "yes":"no");
		DBERR(" - software to hardware (colorkey): %s\n", info->blit_sw_CC == 1 ? "yes":"no");
        DBERR(" - hardware to hardware (alpha): %s\n", info->blit_hw_A == 1 ? "yes":"no");
		DBERR(" - software to hardware (alpha): %s\n", info->blit_sw_A == 1 ? "yes":"no");
		DBERR(" - color fills accelerated: %s\n", info->blit_fill == 1 ? "yes":"no");
        		
//
//		dit werkt ook?! proberen op laptop...
//		SDL_SetVideoMode(res*256, res*212+(res*16), 32, SDL_SWSURFACE);
//
#endif

#ifdef CONSOLE_DEBUGGING_ON
		DBERR("GUI Memory-debugging surface: %ux%u\n", res*256, res*212+(res*16));
	   	screen = getFakeSDL_Surface(res*256,res*212+(res*16));
#endif
		if (screen == 0) {
			DBERR("GUI error: unable to set %ux%uz videomode: ", res*256, res*212+(res*16), SDL_GetError());
		}
		
		screenBase = (Uint32 *)screen->pixels;
		bufp1 = screenBase;
		firstLine = screenBase;
	 	lastResolution = resolution;
	}
}

SDL_Surface * GUI::getFakeSDL_Surface(unsigned int width, unsigned int height) {

		SDL_Surface *fake_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,32, 0,0,0,0 );


/*
		SDL_PixelFormat *fake_pixelFormat = new SDL_PixelFormat();

		fake_pixelFormat->palette = new SDL_Palette();
		fake_pixelFormat->palette->ncolors = 0;

		fake_pixelFormat->BitsPerPixel = 32;
		fake_pixelFormat->BytesPerPixel = 4;
		fake_pixelFormat->Rmask = 255;	
		fake_pixelFormat->Gmask = 255;	
		fake_pixelFormat->Bmask = 255;	
		fake_pixelFormat->Amask = 255;	

		fake_pixelFormat->Rshift = 0;	
		fake_pixelFormat->Gshift = 0;	
		fake_pixelFormat->Bshift = 0;	
		fake_pixelFormat->Ashift = 0;	

		fake_pixelFormat->Rloss = 0;	
		fake_pixelFormat->Gloss = 0;	
		fake_pixelFormat->Bloss = 0;	
		fake_pixelFormat->Aloss = 0;	

		fake_pixelFormat->colorkey = 0;	
		fake_pixelFormat->alpha = 0;	

		SDL_Surface *fake_surface = new SDL_Surface();
		fake_surface->flags = 0;
		fake_surface->format = fake_pixelFormat;
		fake_surface->w = width;
		fake_surface->h = height;
		fake_surface->pixels = new unsigned int[width*height];
		fake_surface->refcount = 1;

		fake_surface->pitch = SDL_CalculatePitch(fake_surface);
		fake_surface->offset = 0;
		fake_surface->hwdata = NULL;
		fake_surface->locked = 0;
		fake_surface->map = NULL;
		fake_surface->unused1 = 0;
		SDL_SetClipRect(fake_surface, NULL);
		SDL_FormatChanged(fake_surface);
*/

	   	return fake_surface;
}
void GUI::setBorderColor(unsigned int value) {
    
    borderColor = value;
    borderChanged = true;
}

void GUI::setScreenHeight(unsigned int value) {

    screenHeight = value;
    borderChanged = true;
}

void GUI::setVerticalAdjust(unsigned int value) {

	verticalAdjust = value;
	borderChanged = true;
}

void GUI::updateBorder() {
    
    unsigned int *scr = screenBase;
    unsigned int ln = 212 - screenHeight;
    unsigned int h = ((verticalAdjust * 2) + ln) * 544;
        
    for (unsigned int i=0;i<h;++i) *scr++ = borderColor;
    firstLine = scr;
   
    scr += (screenHeight * 544 * 2);
    h = (((16 - verticalAdjust) * 2) + ln) * 544;
    for (unsigned int i=0;i<h;++i) *scr++ = borderColor;
    borderChanged = false;  

}    

void GUI::simple2x(unsigned int count, unsigned int hAdjust) {

   // DBERR("simple2x --- count:" << dec << count << "hAdjust:" << hAdjust << endl);
 	Uint32 *bufp2;
	unsigned int eee;
	//Slock(screen);
	for (unsigned int y=0;y<count;++y)   {
	    bufp2 = bufp1 + 512 + 32;
		for (unsigned int i=0;i<hAdjust;++i) { 
            *bufp1++ = borderColor; *bufp1++ = borderColor;
            *bufp2++ = borderColor; *bufp2++ = borderColor;
        }
    
 	    for (unsigned int x=0;x<256;++x) {
			eee = *currentMSXscreenPosition++;
    		*bufp1++ = eee; *bufp1++ = eee;
			*bufp2++ = eee; *bufp2++ = eee;
		}
		for (unsigned int i=0;i<(16-hAdjust);++i) {
            *bufp1++ = borderColor; *bufp1++ = borderColor;
            *bufp2++ = borderColor; *bufp2++ = borderColor;
        }    
		bufp1 = bufp2;
	}
	//Sulock(screen);

	if (selecting) {
    	SDL_FillRect(screen, &selectionRectangle, 0x0f0f0f0f);
    }

}

void GUI::simple512asm(unsigned int count, unsigned int hAdjust) {
	
	asmp1 = bufp1;
	asmborder = borderColor;
	asmhadjust = hAdjust;
	msxscr = currentMSXscreenPosition;
	asmcount = count;
/*
__asm ("pushl %eax");
__asm ("pushl %edx");
__asm ("rdtsc");
__asm ("mov %eax, _time1a");
__asm ("mov %edx, _time1d");
__asm ("popl %edx");
__asm ("popl %eax");
	
	// initialize
	__asm ("pushal; 					\
			pushf;						\
			cld;						\
			movl (_msxscr), %esi;		\
			movl (_asmp1), %edi;		\
			movl $2176, %ebp;			\
			movl (_asmborder), %ebx;	\
			movl (_asmcount), %edx;		\
			");
		
	// draw border
	__asm ("movl (_asmhadjust), %ecx;	\
			shll %ecx;					\
 			movl %ebx, %eax;			\
 			drawborder:					\
  			rep stosl;					\
  			");
	
	// draw screenline (move van esi edi en edi+544)
	__asm ("movw $512, %cx;				\
			drawline:					\
   			lodsl;						\
   			movl %eax, (%ebp, %edi);	\
			stosl;						\
			loop drawline;				\
   			");
   			
	// draw border even line and determine last line
    __asm ("movb $32, %cl;				\
    	    movl %ebx, %eax;			\
    	    rep stosl;					\
    		\
 	        movb $32, %cl;				\
    		leal 2048(%edi), %edi;		\
       		decl %edx;					\
    		jnz drawborder;				\
    		");
    		
    // draw last piece of border
    __asm ("movl $16, %ecx;				\
    		subl (_asmhadjust), %ecx;	\
    		shll %ecx;					\
    		rep stosl;					\
    		\
    		popf;						\
    		popal;						\
    		");
   			  
		  currentMSXscreenPosition += 256 * count;
		  bufp1 += 544 * count;

__asm ("pushl %eax");
__asm ("pushl %edx");
__asm ("rdtsc");
__asm ("mov %eax, _time2a");
__asm ("mov %edx, _time2d");
__asm ("popl %edx");
__asm ("popl %eax");

    if (count == 192) {
        time1 = time1d & 0xffffffff;
        time1 = (time1 << 32) + time1a;

        time2 = time2d & 0xffffffff;
        time2 = (time2 << 32) + time2a;

        speedaverage += (time2-time1);
        speedcounter++;
        DBERR("S5: " << dec << (speedaverage/speedcounter) << endl);
    }
*/
}    

void GUI::simple512(unsigned int count, unsigned int hAdjust) {

 	unsigned int eee;
 	Uint32 *bufp2;
/*
__asm ("pushl %eax");
__asm ("pushl %edx");
__asm ("rdtsc");
__asm ("mov %eax, _time1a");
__asm ("mov %edx, _time1d");
__asm ("popl %edx");
__asm ("popl %eax");
*/
//Slock(screen);
	for (unsigned int y=0;y<count;y++)   {
	    bufp2 = bufp1 + 512 + 32;
		for (unsigned int i=0;i<hAdjust;i++) {
            *bufp1++ = borderColor; *bufp1++ = borderColor;
            *bufp2++ = borderColor; *bufp2++ = borderColor;
        }    
		for (unsigned int x=0;x<512;x++) {
		    eee = *currentMSXscreenPosition++;
			*bufp1++ = eee;
			*bufp2++ = eee;
		}			
  	    
		for (unsigned int i=0;i<(16-hAdjust);i++) {
            *bufp1++ = borderColor; *bufp1++ = borderColor;
            *bufp2++ = borderColor; *bufp2++ = borderColor;
        }    
		bufp1 = bufp2;
	}
//	Sulock(screen);
/*
__asm ("pushl %eax");
__asm ("pushl %edx");
__asm ("rdtsc");
__asm ("mov %eax, _time2a");
__asm ("mov %edx, _time2d");
__asm ("popl %edx");
__asm ("popl %eax");

    if (count == 192) {
        time1 = time1d & 0xffffffff;
        time1 = (time1 << 32) + time1a;

        time2 = time2d & 0xffffffff;
        time2 = (time2 << 32) + time2a;

        speedaverage += (time2-time1);
        speedcounter++;
        DBERR("S5: " << dec << (speedaverage/speedcounter) << endl);
    }    
*/

	if (selecting) {
    	SDL_FillRect(screen, &selectionRectangle, SDL_MapRGBA(screen->format,15,15,15,20));
    }
}

void GUI::afterScaler() {

	if (borderChanged) updateBorder();
	bufp1 = firstLine;
    currentMSXscreenPosition = MSXscreen;

#ifndef CONSOLE_DEBUGGING_ON
	osd->displayOSD(screen);
	SDL_Flip(screen);
#endif
}    

void GUI::simple3x(register unsigned int *MSXscreen) {

 	unsigned int *pE;
	unsigned int eee;

	Uint32 *bufp1 = actualScreen;
	Uint32 *bufp2;
	Uint32 *bufp3;

	pE = MSXscreen + (256 + 16);

	for (unsigned int y=0;y<screenHeight;y++)   {
	    bufp2 = bufp1 + (768 + 48);
		bufp3 = bufp2 + (768 + 48);
  	    for (unsigned int x=0;x<(256 + 16);x++) {
			eee = *pE++;
			*bufp1++ = eee; *bufp1++ = eee; *bufp1++ = eee;
			*bufp2++ = eee; *bufp2++ = eee; *bufp2++ = eee;
			//	eee = (eee >> 1) & 0x7F7F7F;
			*bufp3++ = eee; *bufp3++ = eee; *bufp3++ = eee;
   			
		}
		bufp1 = bufp3;
	}
}

void GUI::scale2x(register unsigned int *MSXscreen) {

	unsigned int *pB, *pF, *pH;
	unsigned int B, D, E, F, H;

	Uint32 *bufp1 = actualScreen;
	Uint32 *bufp2;

	pB = MSXscreen;
	D = *(MSXscreen + 255 + 16);
	E = *(MSXscreen + 256 + 16);
	pF = MSXscreen + 257 + 16;
	pH = MSXscreen + 512 + 32;

	for (unsigned int y=0;y<screenHeight;y++) {
	    bufp2 = bufp1 + 512 + 32;
		for (unsigned int x=0;x<256+16;x++) {
			B = *pB++;
			F = *pF++;
			H = *pH++;

			if (B != H && D != F) {
				*bufp1++ = D == B ? D : E;
				*bufp1++ = B == F ? F : E;
				*bufp2++ = D == H ? D : E;
				*bufp2++ = H == F ? F : E;
			} else {
   			    *bufp1++ = E;
          		*bufp1++ = E;
   			    *bufp2++ = E;
          		*bufp2++ = E;
   			}
			D = E;
			E = F;
		}
		bufp1 = bufp2;
	}
}

void GUI::scale3x(unsigned int *MSXscreen) {

	unsigned int B, D, E, F, H;
	unsigned int *pB, *pF, *pH;

	Uint32 *bufp1 = actualScreen;
	Uint32 *bufp2;
	Uint32 *bufp3;

	pB = MSXscreen;
	D = *(MSXscreen + 255 + 16);
	E = *(MSXscreen + 256 + 16);
	pF = MSXscreen + 257 + 16;
	pH = MSXscreen + 512 + 32;

	for (unsigned int y=0;y<screenHeight;y++) {
		bufp2 = bufp1 + (3*(256+16));
		bufp3 = bufp2 + (3*(256+16));

		for (unsigned int x=0;x<256+16;x++) {
			B = *pB++;
			F = *pF++;
			H = *pH++;

			if (B != H && D != F) {
				*bufp1++ = D == B ? D : E;
				*bufp1++ = E;
				*bufp1++ = B == F ? F : E;
				*bufp2++ = E;
				*bufp2++ = E;
				*bufp2++ = E;
				*bufp3++ = D == H ? D : E;
				*bufp3++ = E;
				*bufp3++ = H == F ? F : E;
			} else {
   				*bufp1++ = E; *bufp1++ = E; *bufp1++ = E;
       			*bufp2++ = E; *bufp2++ = E; *bufp2++ = E;
       			*bufp3++ = E; *bufp3++ = E; *bufp3++ = E;
		    }
   			D = E;
   			E = F;
   		}
		bufp1 = bufp3;
    }
}

inline unsigned GUI::INTERPOLATE(unsigned int A, unsigned int B) {
//    if (A !=B)
//    {
       return ((A & ~blendmask) >> 1) + ((B & ~blendmask) >> 1)	+ (A & blendmask);
//    }
//    else return A;
}

void GUI::scale2xSaI(unsigned int *MSXscreen) {
	
	/* Map of the pixels:
 		I|E F|J
 		G|A B|K
 		H|C D|L
 		M|N O|P
	*/

 	unsigned int colorA, colorB, colorC, colorD, colorE, colorF;
	unsigned int colorG, colorH, colorI, colorJ, colorK, colorL; 
	unsigned int colorM, colorN, colorO, colorP;
	unsigned int product, product1, product2;
	unsigned int *jP, *kP, *lP, *pP;
	unsigned int x, y;
	
	unsigned int *bufp1 = actualScreen;
	unsigned int *bufp2;

	colorI = *(MSXscreen + 0 - 1);
    colorE = *(MSXscreen + 0 + 0);
    colorF = *(MSXscreen + 0 + 1);
    colorG = *(MSXscreen + 272 - 1);
    colorA = *(MSXscreen + 272 + 0);
    colorB = *(MSXscreen + 272 + 1);
    colorH = *(MSXscreen + 544 - 1);
    colorC = *(MSXscreen + 544 + 0);
    colorD = *(MSXscreen + 544 + 1);
    colorM = *(MSXscreen + 816 - 1);
    colorN = *(MSXscreen + 816 + 0);
    colorO = *(MSXscreen + 816 + 1);
 	jP = MSXscreen + 0 + 2;
 	kP = MSXscreen + 272 + 2;
 	lP = MSXscreen + 544 + 2;
 	pP = MSXscreen + 816 + 2;
 	
    for (y=0;y<screenHeight;y++) {

		bufp2 = bufp1 + 544;

	    for (x=0;x<272;x++) {
            colorJ = *jP++;
            colorK = *kP++;
            colorL = *lP++;
            colorP = *pP++;
            
            *bufp1++ = colorA;
            
		if (colorA == colorD) {
			if (colorB == colorC) {
   			    if (colorA == colorB) product = product1 = product2 = colorA;
			    else {
				    unsigned int r = 0;
				    if (colorE == colorG) if (colorA == colorE) r--; else if (colorB == colorE) r++;
				    if (colorF == colorK) if (colorA == colorF) r--; else if (colorB == colorF) r++;
				    if (colorH == colorN) if (colorA == colorH) r--; else if (colorB == colorH) r++;
				    if (colorL == colorO) if (colorA == colorL) r--; else if (colorB == colorL) r++;
				    product = product1 = INTERPOLATE(colorA, colorB);
				    product2 = r > 0 ? colorA : (r < 0 ? colorB : product);
			    }			
			} else {
			    product = ( ( (colorA == colorE && colorB == colorL)
				      || (colorA == colorC && colorA == colorF && colorB != colorE && colorB == colorJ)
				      )
				    ? colorA : INTERPOLATE(colorA, colorB));
        		product1 = ( ( (colorA == colorG && colorC == colorO)
				      || (colorA == colorB && colorA == colorH && colorG != colorC && colorC == colorM)
				      )
				    ? colorA : INTERPOLATE(colorA, colorC));
			    product2 = colorA;
   			}
		} else {
  			if (colorB == colorC) {
   			    product = ( ( (colorB == colorF && colorA == colorH)
				      || (colorB == colorE && colorB == colorD && colorA != colorF && colorA == colorI)
				      )
				    ? colorB : INTERPOLATE(colorA, colorB));
			    product1 = ( ( (colorC == colorH && colorA == colorF)
				      || (colorC == colorG && colorC == colorD && colorA != colorH && colorA == colorI)
				      )
				    ? colorC : INTERPOLATE(colorA, colorC));
			    product2 = colorB;
     		} else {
			    product =
				    ( colorA == colorC && colorA == colorF && colorB != colorE && colorB == colorJ
				    ? colorA : ( colorB == colorE && colorB == colorD && colorA != colorF && colorA == colorI
				      ? colorB : INTERPOLATE(colorA, colorB))
      			    );
			    product1 =
				    ( colorA == colorB && colorA == colorH && colorG != colorC && colorC == colorM
				    ? colorA : ( colorC == colorG && colorC == colorD && colorA != colorH && colorA == colorI
				      ? colorC : INTERPOLATE(colorA, colorC))
				    );
			    product2 = INTERPOLATE( INTERPOLATE(colorA, colorB), INTERPOLATE(colorC, colorD)); //QUAD sneller???
   		    }
		}
  	        *bufp1++ = product;
            *bufp2++ = product1;
            *bufp2++ = product2;

            colorI = colorE; colorE = colorF; colorF = colorJ;
            colorG = colorA; colorA = colorB; colorB = colorK;
            colorH = colorC; colorC = colorD; colorD = colorL;
            colorM = colorN; colorN = colorO; colorO = colorP;
        }
        bufp1 = bufp2;
    }            
}

inline int GUI::GetResult(unsigned int A, unsigned int B, unsigned int C, unsigned int D) {
	int x = 0; 
	int y = 0;
	int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r+=1; 
	if (y <= 1) r-=1;
	return r;
}

void GUI::scaleSuper2xSaI(unsigned int *MSXscreen) {

	unsigned int *pB3, *pS2, *pS1, *pA3;
	unsigned int colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
	unsigned int colorA0, colorA3, colorB0, colorB3;
 	unsigned int color1, color2, color3, color4, color5, color6;
	unsigned int product1a, product1b, product2a, product2b;
	
	switch (screen->format->BytesPerPixel) {
	case 1: // Assuming 8-bpp
		DBERR("NOT SUPPORTED\n");
		break;
	case 2: // Probably 15-bpp or 16-bpp
		DBERR("NOT SUPPORTED\n");
		break;
	case 3: // Slow 24-bpp mode, usually not used
		DBERR("NOT SUPPORTED\n");
		break;
	case 4: // Probably 32-bpp
		Uint32 *bufp1 = actualScreen;
		Uint32 *bufp2;

		colorB0 = *(MSXscreen - 1);
		colorB1 = *(MSXscreen);
        colorB2 = *(MSXscreen + 1);
        pB3 = MSXscreen + 2;

        color4 = *(MSXscreen + 272 - 1);
        color5 = *(MSXscreen + 272);
        color6 = *(MSXscreen + 272 + 1);
        pS2 = MSXscreen + 272 + 2;

        color1 = *(MSXscreen + 544 - 1);
        color2 = *(MSXscreen + 544);
        color3 = *(MSXscreen + 544 + 1);
        pS1 = MSXscreen + 544 + 2;

        colorA0 = *(MSXscreen + 816 - 1);
        colorA1 = *(MSXscreen + 816);
        colorA2 = *(MSXscreen + 816 + 1); 
        pA3 = MSXscreen + 816 + 2;

		for (unsigned int y=0;y<screenHeight;y++)   {
			bufp2 = bufp1 + 544;

			for (int x=0;x<272;x++) {
				colorB3 = *pB3++;
				colorS2 = *pS2++;
				colorS1 = *pS1++;
				colorA3 = *pA3++;
				
				if (color2 == color6) {
    				if (color5 == color3) {
                        unsigned int r = 0;
                        r += GetResult (color6, color5, color1, colorA1);
                        r += GetResult (color6, color5, color4, colorB1);
                        r += GetResult (color6, color5, colorA2, colorS1);
                        r += GetResult (color6, color5, colorB2, colorS2);

                       if (r > 0) product2b = product1b = color6;
                       else {
                           if (r < 0) product2b = product1b = color5;
                           else product2b = product1b = INTERPOLATE (color5, color6);
                       }
    				} else {
    				    product2b = product1b = color2;
        			}
       			} else {
       			    if (color5 == color3) {
    				    product2b = product1b = color5;
 			        } else {
                        if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                           product2b = INTERPOLATE (INTERPOLATE(color3, color2), color3);
                        else
                        if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                           product2b = INTERPOLATE (INTERPOLATE(color2, color3), color2);
                        else
                           product2b = INTERPOLATE (color2, color3);

                        if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                           product1b = INTERPOLATE (INTERPOLATE(color6, color5), color6);
                        else
                        if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                           product1b = INTERPOLATE (INTERPOLATE(color6, color5), color5);
                        else
                           product1b = INTERPOLATE (color5, color6);
    				}
   				}
				
                if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                   product2a = INTERPOLATE (color2, color5);
                else
                if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                   product2a = INTERPOLATE(color2, color5);
                else
                   product2a = color2;

                if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                   product1a = INTERPOLATE (color2, color5);
                else
                if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                   product1a = INTERPOLATE(color2, color5);
                else
                   product1a = color5;
				
				*bufp1++ = product1a;
				*bufp1++ = product1b;				
				*bufp2++ = product2a;
				*bufp2++ = product2b;

				colorB0 = colorB1; colorB1 = colorB2; colorB2 = colorB3;
				color4 = color5; color5 = color6; color6 = colorS2;
				color1 = color2; color2 = color3; color3 = colorS1;
				colorA0 = colorA1; colorA1 = colorA2; colorA2 = colorA3;
			}
			bufp1 = bufp2;
		}
	}
}

void GUI::scaleSuperEagle(unsigned int *MSXscreen) {

	unsigned int *pB2, *pS2, *pS1, *pA2;
	unsigned int colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
 	unsigned int color1, color2, color3, color4, color5, color6;
	unsigned int product1a, product1b, product2a, product2b;
	
	switch (screen->format->BytesPerPixel) {
	case 1: // Assuming 8-bpp
		DBERR("NOT SUPPORTED\n");
		break;
	case 2: // Probably 15-bpp or 16-bpp
		DBERR("NOT SUPPORTED\n");
		break;
	case 3: // Slow 24-bpp mode, usually not used
		DBERR("NOT SUPPORTED\n");
		break;
	case 4: // Probably 32-bpp
		Uint32 *bufp1;
		Uint32 *bufp2;


		colorB1 = *(MSXscreen);
        pB2 = MSXscreen + 1;

        color4 = *(MSXscreen + 272 - 1); // TODO: valt nu buiten de array (maar staks met border erbij niet meer)
        color5 = *(MSXscreen + 272);
        color6 = *(MSXscreen + 272 + 1);
        pS2 = MSXscreen + 272 + 2;

        color1 = *(MSXscreen + 544 - 1);
        color2 = *(MSXscreen + 544);
        color3 = *(MSXscreen + 544 + 1);
        pS1 = MSXscreen + 544 + 2;

        colorA1 = *(MSXscreen + 816);
        pA2 = MSXscreen + 816 + 1; 

		for (unsigned int y=0;y<screenHeight;y++)   {
			bufp1 = actualScreen + y*screen->pitch/2;
			bufp2 = bufp1 + screen->pitch/4;

			for (unsigned int x=0;x<272;x++) {
				colorB2 = *pB2++;
				colorS2 = *pS2++;
				colorS1 = *pS1++;
				colorA2 = *pA2++;
				
                if (color2 == color6 && color5 != color3)
                {
                   product1b = product2a = color2;
                   if ((color1 == color2) || (color6 == colorB2)) {
                       product1a = INTERPOLATE (color2, color5);
                       product1a = INTERPOLATE (color2, product1a);
//                       product1a = color2;
                   }
                   else
                   {
                      product1a = INTERPOLATE (color5, color6);
                   }

                   if ((color6 == colorS2) || (color2 == colorA1)) {
                       product2b = INTERPOLATE (color2, color3);
                       product2b = INTERPOLATE (color2, product2b);
//                       product2b = color2;
                   }
                   else
                   {
                      product2b = INTERPOLATE (color2, color3);
                   }
                }
                else
                if (color5 == color3 && color2 != color6)
                {
                   product2b = product1a = color5;

                   if ((colorB1 == color5) ||
                       (color3 == colorS1))
                   {
                       product1b = INTERPOLATE (color5, color6);
                       product1b = INTERPOLATE (color5, product1b);
//                  product1b = color5;
                   }
                   else
                   {
                      product1b = INTERPOLATE (color5, color6);
                   }

		   if ((color3 == colorA2) || (color4 == color5)) {
                       product2a = INTERPOLATE (color5, color2);
                       product2a = INTERPOLATE (color5, product2a);
//                   product2a = color5;
                   }
                   else
                   {
                      product2a = INTERPOLATE (color2, color3);
                   }

                }
                else
                if (color5 == color3 && color2 == color6) {
                   register int r = 0;

                   r += GetResult (color6, color5, color1, colorA1);
                   r += GetResult (color6, color5, color4, colorB1);
                   r += GetResult (color6, color5, colorA2, colorS1);
                   r += GetResult (color6, color5, colorB2, colorS2);

                   if (r > 0) {
                      product1b = product2a = color2;
                      product1a = product2b = INTERPOLATE (color5, color6);
                   }
                   else
                   if (r < 0) {
                      product2b = product1a = color5;
                      product1b = product2a = INTERPOLATE (color5, color6);
                   }
                   else {
                      product2b = product1a = color5;
                      product1b = product2a = color2;
                   }
                }
                else {
                      product2b = product1a = INTERPOLATE (color2, color6);
                      product2b = INTERPOLATE (INTERPOLATE(color3, product2b), color3);
                      product1a = INTERPOLATE (INTERPOLATE(color5, product1a), color5);

                      product2a = product1b = INTERPOLATE (color5, color3);
                      product2a = INTERPOLATE (INTERPOLATE(color2, product2a), color2);
                      product1b = INTERPOLATE (INTERPOLATE(color6, product1b), color6);

//      product1a = color5;
//      product1b = color6;
//      product2a = color2;
//      product2b = color3;
                }
				
				*bufp1++ = product1a;
				*bufp1++ = product1b;				
				*bufp2++ = product2a;
				*bufp2++ = product2b;

				colorB1 = colorB2;
				color4 = color5; color5 = color6; color6 = colorS2;
				color1 = color2; color2 = color3; color3 = colorS1;
				colorA1 = colorA2;
			}
		}
	}
}

void GUI::scaleSuper3xSaI(unsigned int *MSXscreen) {

	unsigned int *pB3, *pS2, *pS1, *pA3;
	unsigned int colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
	unsigned int colorA0, colorA3, colorB0, colorB3;
 	unsigned int color1, color2, color3, color4, color5, color6;
	unsigned int product1a, product1b, product2a, product2b;
	
	switch (screen->format->BytesPerPixel) {
	case 1: // Assuming 8-bpp
		DBERR("NOT SUPPORTED\n");
		break;
	case 2: // Probably 15-bpp or 16-bpp
		DBERR("NOT SUPPORTED\n");
		break;
	case 3: // Slow 24-bpp mode, usually not used
		DBERR("NOT SUPPORTED\n");
		break;
	case 4: // Probably 32-bpp
		Uint32 *bufp1 = actualScreen;
		Uint32 *bufp2;
		Uint32 *bufp3;

		colorB0 = *(MSXscreen - 1);
		colorB1 = *(MSXscreen);
        colorB2 = *(MSXscreen + 1);
        pB3 = MSXscreen + 2;

        color4 = *(MSXscreen + 256 - 1);
        color5 = *(MSXscreen + 256);
        color6 = *(MSXscreen + 256 + 1);
        pS2 = MSXscreen + 256 + 2;

        color1 = *(MSXscreen + 512 - 1);
        color2 = *(MSXscreen + 512);
        color3 = *(MSXscreen + 512 + 1);
        pS1 = MSXscreen + 512 + 2;

        colorA0 = *(MSXscreen + 768 - 1);
        colorA1 = *(MSXscreen + 768);
        colorA2 = *(MSXscreen + 768 + 1); 
        pA3 = MSXscreen + 768512 + 2;

		for (unsigned int y=0;y<screenHeight;y++)   {
			bufp2 = bufp1 + 768;
			bufp3 = bufp2 + 768;

			for (unsigned int x=0;x<256;x++) {
				colorB3 = *pB3++;
				colorS2 = *pS2++;
				colorS1 = *pS1++;
				colorA3 = *pA3++;
				
				if (color2 == color6) {
    				if (color5 == color3) {
                        unsigned int r = 0;
                        r += GetResult (color6, color5, color1, colorA1);
                        r += GetResult (color6, color5, color4, colorB1);
                        r += GetResult (color6, color5, colorA2, colorS1);
                        r += GetResult (color6, color5, colorB2, colorS2);

                       if (r > 0) product2b = product1b = color6;
                       else {
                           if (r < 0) product2b = product1b = color5;
                           else product2b = product1b = INTERPOLATE (color5, color6);
                       }
    				} else {
    				    product2b = product1b = color2;
        			}
       			} else {
       			    if (color5 == color3) {
    				    product2b = product1b = color5;
 			        } else {
                        if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
                           product2b = INTERPOLATE (INTERPOLATE(color3, color2), color3);
                        else
                        if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
                           product2b = INTERPOLATE (INTERPOLATE(color2, color3), color2);
                        else
                           product2b = INTERPOLATE (color2, color3);

                        if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
                           product1b = INTERPOLATE (INTERPOLATE(color6, color5), color6);
                        else
                        if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
                           product1b = INTERPOLATE (INTERPOLATE(color6, color5), color5);
                        else
                           product1b = INTERPOLATE (color5, color6);
    				}
   				}
				
                if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
                   product2a = INTERPOLATE (color2, color5);
                else
                if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
                   product2a = INTERPOLATE(color2, color5);
                else
                   product2a = color2;

                if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
                   product1a = INTERPOLATE (color2, color5);
                else
                if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
                   product1a = INTERPOLATE(color2, color5);
                else
                   product1a = color5;
				
				*bufp1++ = product1a;
            	*bufp1++ = INTERPOLATE(product1a, product1b);				
				*bufp1++ = product1b;
				
				*bufp2++ = INTERPOLATE(product1a, product2a);
				*bufp2++ = INTERPOLATE(INTERPOLATE(product1a, product2a),INTERPOLATE(product1b, product2b));
				*bufp2++ = INTERPOLATE(product1b, product2b);
				
				*bufp3++ = product2a;
				*bufp3++ = INTERPOLATE(product2a, product2b);	
    			*bufp3++ = product2b;

				colorB0 = colorB1; colorB1 = colorB2; colorB2 = colorB3;
				color4 = color5; color5 = color6; color6 = colorS2;
				color1 = color2; color2 = color3; color3 = colorS1;
				colorA0 = colorA1; colorA1 = colorA2; colorA2 = colorA3;
			}
			bufp1 = bufp3;
		}
	}
}

void GUI::updateSelection(unsigned int x, unsigned int y) {
    
    unsigned int selX = selectionStartRectangle.x;
    unsigned int selY = selectionStartRectangle.y;
    
    if (selX < x) { 
        selectionRectangle.x = selX;
        selectionRectangle.w = x - selX ;
    } else {
        selectionRectangle.x = x;
        selectionRectangle.w = selX - x;
    }           

    if (selY < y) { 
        selectionRectangle.y = selY;
        selectionRectangle.h = y - selY;
    } else {
        selectionRectangle.y = y;
        selectionRectangle.h = selY - y;
    }           

}    

void GUI::startSelection(unsigned int x, unsigned int y) {
    
    selectionStartRectangle.x = x;
    selectionStartRectangle.y = y;
    updateSelection(x,y);
    selecting = true;
}    

void GUI::endSelection(unsigned int x, unsigned int y) {

    updateSelection(x,y);
    
    // do actual copy-operation.
	selecting = false;
    string copyText = V9938renderer::Instance()->copyFromText2(selectionRectangle.x, selectionRectangle.y, selectionRectangle.w, selectionRectangle.h);
        
#ifdef WIN32 

	if (!OpenClipboard(NULL)) {
 	   DBERR("Failed to open clipboard!\n");
 	   return;
    }
    EmptyClipboard(); 

    // Allocate a global memory object for the text. 
    int len = copyText.size() + 1;
    
    void * hglbCopy = GlobalAlloc(GMEM_MOVEABLE, len); 
    if (hglbCopy == NULL) 
    { 
        CloseClipboard(); 
    } 

    // Lock the handle and copy the text to the buffer. 

    void * lptstrCopy = GlobalLock(hglbCopy); 
    memcpy(lptstrCopy, copyText.c_str(), len); 
    GlobalUnlock(hglbCopy); 

    // Place the handle on the clipboard. 
     SetClipboardData(CF_TEXT, hglbCopy);  
     CloseClipboard(); 
        
#endif    
}
