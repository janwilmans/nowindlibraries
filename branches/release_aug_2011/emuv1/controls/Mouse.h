//! Mouse.h
#ifndef MOUSE_H
#define MOUSE_H

#include "msxtypes.h"
#include "SDL.h"


class Mouse {

private:
	
		Mouse();
		
		nw_byte relativeMouseX;
		nw_byte relativeMouseY;
		nw_byte x1, y1;
		nw_byte PSGPortA;
		nw_byte PSGPortB;
		int		mouseStatus1;
		int		mouseStatus2;

public:

		~Mouse();
		static	Mouse * Instance();
		
		void	motion(SDL_Event *);
		void	buttonDown(SDL_Event *);
		void	buttonUp(SDL_Event *);
};

#endif
