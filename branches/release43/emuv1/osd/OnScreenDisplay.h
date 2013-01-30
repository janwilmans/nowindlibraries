//! OnScreenDisplay.h
#ifndef ONSCREENDISPLAY_H
#define ONSCREENDISPLAY_H

#include "msxtypes.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include <list>
#include <string>

typedef struct {
    SDL_Surface *surface;
    int advance;
    int maxY;
} glyphStructure;

/*!
 * The OnScreenDisplay class handles the "overlay" screens to manually control the emulator
 */
class OnScreenDisplay {

private:
    
	    			OnScreenDisplay();
    SDL_Surface     *osdSurface;
	glyphStructure  glyph[128];
   	SDL_Surface		*background;
   	SDL_Surface		*consoleFont;
	bool			osdEnabled;
	std::list<std::string>    *currentTextBuffer;
    	
    void            initOSD_Surface(int, int);
    void			initializeFont();
	void			renderOSD(std::list<std::string> *);
    	
    int				fontHeight;
	int				fontWidth;
	
	int				osdTop;
	int				osdLeft;
	int				osdWidth;
	int				osdHeight;
	int				osdAlpha;
	int				maxLines;
	int				lastLine;
	
  	
public:
	
	static 			OnScreenDisplay* Instance();
					~OnScreenDisplay();

	void			toggleOSD(bool);
	
    void            displayOSD(SDL_Surface *);
    void            setAlphaBlending(int);
	/*! Replaces the last line of the console with a new string 
	 * 
	 *  \param text The text to add 
	 *  \return the amount of characters that was added ?
	 */

	void			xorBar(SDL_Surface *, int);
};

#endif

