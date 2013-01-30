//! Keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "msxtypes.h"
#include "SDL.h"


class Keyboard {

private:
		Keyboard();
		nw_byte	keyboardMatrix[16];
		nw_byte	keyboardLine;

		
public:

		~Keyboard();
		static	Keyboard * Instance();

		void	selectKeyboardLine(nw_byte);
		nw_byte readKeyboardLine();
		void 	modifyMatrix(SDL_KeyboardEvent);
		void 	modifyMatrixAscii(char, bool);

};

#endif
