#include "stdio.h"

#include "Mouse.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;

Mouse::Mouse() {

	DBERR("Mouse constructor...\n");

	mouseStatus1 = 0;
	mouseStatus2 = 0;

	DBERR("Mouse constructor...finished\n");

}

Mouse *Mouse::Instance() {

	// implies singleton class
	static Mouse deInstantie;
	return &deInstantie;
}

Mouse::~Mouse() {
    
	DBERR("Mouse destroyed.\n");
}

void Mouse::motion(SDL_Event *event) {
	signed int coordinate;

	coordinate = (event->motion.xrel) / -1; // TODO: hiervoor straks de scalefactor gebruiken
	if (coordinate > 127) coordinate = 127;
	if (coordinate < -127) coordinate = -127;
	relativeMouseX = coordinate;

	coordinate = (event->motion.yrel) / -1;
	if (coordinate > 127) coordinate = 127;
	if (coordinate < -127) coordinate = -127;
	relativeMouseY = coordinate;
}

void Mouse::buttonDown(SDL_Event *event) {
//	switch (event->button.button) {
//	case SDL_BUTTON_LEFT: MSXJoystickPort[1] &= ~BUTTONA; break;
//	default: MSXJoystickPort[1] &= ~BUTTONB; break;
//	}
}

void Mouse::buttonUp(SDL_Event *event) {
//	switch (event->button.button) {
//	case SDL_BUTTON_LEFT: MSXJoystickPort[1] |= BUTTONA; break;
//	default: MSXJoystickPort[1] |= BUTTONB; break;
//	}
}

/* COPIED FROM CONSOLE.CPP
nw_byte Console::readPSGPortA() {
	nw_byte rval;
	if (PSGPortB & 0x40) {
		// joystick port 2

  //      DBERR("mouseStatus1:"<<mouseStatus1<<endl);
  //      DBERR("mouseStatus2:"<<mouseStatus2<<endl);
  		
		switch (mouseStatus2) {
		case 0: rval = 0xFF; break;
		case 1:
			x1 = relativeMouseX;
			y1 = relativeMouseY;
			relativeMouseX = 0;
			relativeMouseY = 0;
			rval = x1 >> 4;
			break;
		case 2: rval = x1 & 0x0F; break;
		case 3: rval = y1 >> 4; break;
		default:
			rval = y1 & 0x0F;
			mouseStatus2 = 0;
			break;
		}
	} else {
		// joystick port 1
		rval = 255;//joystickPort[0]; // nu even joystick op port1
	}
	return rval;
}

nw_byte Console::readPSGPortB() {
	return PSGPortB;
}

void Console::writePSGPortA(nw_byte value) {
	PSGPortA = value;
}

void Console::writePSGPortB(nw_byte value) {

//    DBERR("PSGPortB:" << hex << (int)(PSGPortB&0x20)<< endl);
	if ((PSGPortB ^ value) & 0x10) mouseStatus1++;
	if ((PSGPortB ^ value) & 0x20) mouseStatus2++;

//  if ((mouseStatus2==0) && (PSGPortB & 0x20)) mouseStatus2++;
	PSGPortB = value;
}
*/
