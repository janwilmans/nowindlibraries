//! Joystick.h
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "msxtypes.h"
#include "SDL.h"

#define UP 0x01
#define DOWN 0x02
#define LEFT 0x04
#define RIGHT 0x08

#define BUTTONA 0x10
#define BUTTONB 0x20

#define TRESHOLD 6400

class Joystick {

private:
		Joystick();

		nw_byte	joystickPort[2];
		nw_byte PSGPortA;
		nw_byte PSGPortB;
		
public:

		~Joystick();
		static Joystick * Instance();

		void 	axisMotion(SDL_Event *);
		void 	buttonDown(SDL_Event *);
		void 	buttonUp(SDL_Event *);

};

#endif
