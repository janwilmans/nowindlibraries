//! F4Device.h
#ifndef F4DEVICE_H
#define F4DEVICE_H

#include "msxtypes.h"

class F4Device {
	
private:
	
	nw_byte portF4;
	
public:
	
	F4Device();
	~F4Device();
	void 	reset();

	void 	writeRegister(nw_byte);
	nw_byte	readRegister();
};

#endif

