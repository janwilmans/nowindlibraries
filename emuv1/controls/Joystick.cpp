#include "stdio.h"

#include "Joystick.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;

Joystick::Joystick() {

	DBERR("Joystick constructor...\n");

	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	if (SDL_NumJoysticks()==0) {
		DBERR("No joystick connected!\n");
	} else {
		for(int i=0;i<SDL_NumJoysticks();i++) {
			DBERR("Found joystick[%u] - %s\n", i, SDL_JoystickName(i));
		}
		// alleen de eerste vreugdepaal gebruiken
		SDL_Joystick *stick;
		SDL_JoystickEventState(SDL_ENABLE);
		stick = SDL_JoystickOpen(0);
	}
	joystickPort[0] = 0xFF;
	joystickPort[1] = 0xFF;

	PSGPortA = 0xFF;
 	PSGPortB = 0xFF;

	DBERR("Joystick constructor...finished\n");

}

Joystick *Joystick::Instance() {

	// implies singleton class
	static Joystick deInstantie;
	return &deInstantie;
}

Joystick::~Joystick() {
    
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	DBERR("Joystick destroyed.\n");
}

void Joystick::axisMotion(SDL_Event *event) {
	if ((event->jaxis.axis) == 0) {
		joystickPort[0] |= 0x0C;
		if ((event->jaxis.value) > TRESHOLD) joystickPort[0] &= ~RIGHT;
		if ((event->jaxis.value) < -TRESHOLD) joystickPort[0] &= ~LEFT;
	}
	if ((event->jaxis.axis) == 1) {
		joystickPort[0] |= 0x03;
		if ((event->jaxis.value) > TRESHOLD) joystickPort[0] &= ~DOWN;
		if ((event->jaxis.value) < -TRESHOLD) joystickPort[0] &= ~UP;
	}
}

void Joystick::buttonDown(SDL_Event *event) {
	switch (event->jbutton.button) {
	case 0: joystickPort[0] &= ~BUTTONA; break;
	case 1: joystickPort[0] &= ~BUTTONB; break;
	case 2:
	case 3: DBERR("Joystick button 2 and 3 not used!\n"); break;
	default:
		break;
	}
}

void Joystick::buttonUp(SDL_Event *event) {
	switch (event->jbutton.button) {
	case 0: joystickPort[0] |= BUTTONA; break;
	case 1: joystickPort[0] |= BUTTONB; break;
	case 2: break;
	case 3: break;
	default:
		break;
	}
}
