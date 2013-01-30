// SwitchedPorts.h
#ifndef SWITCHEDPORTS_H
#define SWITCHEDPORTS_H

#include "msxtypes.h"

/*!
 * The SwitchedPorts class emulates the switch-port-extention at port #41-#4F
 */
class SwitchedPorts {
	
private:
					SwitchedPorts();
    bool			deviceEnabled[256];
    unsigned int	deviceID;
 	
public:
	
	static 		SwitchedPorts* Instance();
				~SwitchedPorts();

	void		reset();
	nw_byte		readDeviceID();
 	nw_byte		readPort(nw_word);
 	
	void		writeDeviceID(nw_byte);
	void		writePort(nw_word, nw_byte);
};

#endif

