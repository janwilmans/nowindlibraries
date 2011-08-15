//! GUI.h
#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <string>
#include "msxtypes.h"
#include <map>

//#include "SDL_console.h"
//#include "SDL.h"          //jan: waarom werkt het compilen zonder deze include?

class OnScreenDisplay;

static const int SIMPLE = 0;
static const int SCALE2X = 1;
static const int SCALE2X_SAI = 2;
static const int SCALE2X_S_2XSAI = 3;
static const int SCALE_S_EAGLE = 4;
static const int SIMPLE3X = 5;
static const int SCALE3X = 6;
static const int SCALE3XSAI = 7;
static const int SCALE_LAST = 7;

/*!
 * The GUI class contains the actual image-displaying methods, scalers and filters.
 */
class GUI {

private:

		SDL_Surface 	*screen;
		SDL_Event event;
		SDL_Surface 	*bmpdisk;
		OnScreenDisplay *osd;
        
//        ConsoleInformation *Console;

		void Slock(SDL_Surface *screen);
		void Sulock(SDL_Surface *screen);

        V9938 			*vdp;
		unsigned int	*MSXscreen;
		unsigned int	*currentMSXscreenPosition;

        Uint32  		*screenBase;
        Uint32			*actualScreen;
       	Uint32 			*bufp1;
       	
		unsigned int 	blendmask;	
		unsigned int 	selectedScaler;
		std::string 	scalerName;
	
		unsigned int	lastResolution;
		unsigned int	resolution;
        unsigned int	verticalAdjust;
        unsigned int    screenHeight;
        unsigned int	borderColor;
        bool			borderChanged;
        unsigned int	*firstLine;
        
		void			initializeFont();
		void 			setResolution(unsigned int);
		void			updateBorder();
		
		unsigned int 	INTERPOLATE(unsigned int, unsigned int);
		int			 	GetResult(unsigned int, unsigned int, unsigned int, unsigned int);

		SDL_Surface *	getFakeSDL_Surface(unsigned int width, unsigned int height);

		GUI();
		
		bool            selecting;
		SDL_Rect        selectionStartRectangle;
		SDL_Rect        selectionRectangle;

public:
		static	GUI * Instance();
		~GUI();
		void 	reset();
		void	setVdp(V9938*);
		
		void	setBorderColor(unsigned int);
		void 	setScaler(int);
		int	 	getScaler();
		void 	scaler(unsigned int*);
		void	afterScaler();
		
		void	setVerticalAdjust(unsigned int);
		void	setScreenHeight(unsigned int);
		
  		void 	simple2x(unsigned int, unsigned int);
  		void	simple512(unsigned int, unsigned int);
  		void	simple512asm(unsigned int, unsigned int);
  		void	simple3x(unsigned int*);
		void    scale3x(unsigned int*);
		void    scale2x(unsigned int*);
  		void	scale2xSaI(unsigned int*);
  		void	scaleSuper2xSaI(unsigned int*);
  		void	scaleSuperEagle(unsigned int*);
  		void	scaleSuper3xSaI(unsigned int*);

		void	fullscreenToggle(); 
		
        void    updateSelection(unsigned int x, unsigned int y);
        void    startSelection(unsigned int x, unsigned int y);
        void    endSelection(unsigned int x, unsigned int y);
};

#endif

