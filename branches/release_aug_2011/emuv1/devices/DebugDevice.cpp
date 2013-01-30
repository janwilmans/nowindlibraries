// DebugDevice.ccp// http://openmsx.sourceforge.net/manual/user.html#debugdevice

#include "stdio.h"
#include "DebugDevice.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;

DebugDevice::DebugDevice() {

	DBERR("DebugDevice constructor...\n");
	modeRegister = 0;
	DBERR("DebugDevice constructor...finished\n");
}

DebugDevice::~DebugDevice() {
	
	DBERR("DebugDevice destroyed.\n");
}

void DebugDevice::setModeRegister(nw_byte value) {

	if (!(value & 0x40)) DBERR("\n");
	modeRegister = value & 0x3f;
}

void DebugDevice::writeData(nw_byte value) {

	switch (modeRegister >> 4) {
	case 0: 
		// device output disabled
		break;
	case 1:
		// single byte mode
		if (modeRegister & 1) printHex(value);
		if (modeRegister & 2) printBinary(value);
		if (modeRegister & 4) printDecimal(value);
		if (modeRegister & 8) printAscii(value);

		DBERR("emuTime: %u\n", Z80::Instance()->emuTime);
		break;
	case 2:
		// multi byte mode
		switch (modeRegister & 3) {
		case 0: printHex(value); break;
		case 1: printBinary(value); break;
		case 2: printDecimal(value); break;
		case 3: DBERR("%c", value); break;
		default:
			assert(false);
		}
		break;
	default:
		break;
	}
}

void DebugDevice::printHex(nw_byte value) {

	DBERR("%.2xh ", value);
}

void DebugDevice::printBinary(nw_byte value) {

	for (int i=128;i>0;i>>=1) {
		if (value & i) { DBERR("1"); }
		else { DBERR("0"); }
	}
	DBERR("b ");
}

void DebugDevice::printDecimal(nw_byte value) {

	DBERR("%.3i ", value);
}

void DebugDevice::printAscii(nw_byte value) {

	if (value < 32) value = 46;
	DBERR("'%c' ", value);
}
