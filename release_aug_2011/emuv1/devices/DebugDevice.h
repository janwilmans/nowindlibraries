//! DebugDevice.h
#ifndef DEBUGDEVICE_H
#define DEBUGDEVICE_H

#include <iostream>
#include "msxtypes.h"

class DebugDevice {
	
private:
	
	nw_byte		modeRegister;
	
	void		printHex(nw_byte);	
	void 		printBinary(nw_byte);
	void		printDecimal(nw_byte);
	void		printAscii(nw_byte);

public:
				DebugDevice();
				~DebugDevice();

	void 		setModeRegister(nw_byte);
	void 		writeData(nw_byte);
};

#endif

