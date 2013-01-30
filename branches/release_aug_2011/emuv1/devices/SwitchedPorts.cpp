// SwitchedPorts.ccp

#include "stdio.h"
#include <iostream>
#include <string>

#include "Debug.h"

using namespace std;

/* Switched I/O ports

According to the MSX2 Hardware Specification, ports #40-#4F are forming the so-called 
'switched I/O ports'. With these the limitation of a maximum number of 256 I/O ports 
can be overcome, although it does require a little additional logic on the hardware's 
behalf. The port numbers #41-#4F are the actual switched ports, and the device 'connected'
(e.g. listening to) to those ports is determined by the device ID written to port #40. 
This device ID can be a number from 1-254, and when port #40 is read it returns the 
complement of the current device ID.

Port range 	Description
#40 	Device ID register
#41-#4F 	Switched I/O ports

ID numbers between 1 and 127 are manufacturer ID numbers, and ID numbers 128 to 254 
are device ID's. As a basic rule, those devices which are designed specifically for 
one machine should contain the manufacturer's company ID, while periperhal devices 
which can be used on all MSX computers should have a device ID number. Also, the Z80 
actually has a 16-bit I/O addressing space so it is recommended to access in 16-bit 
by decoding the upper 8 bits for those ID's which might be expanded in the future 
(although I have personally only seen this done in the ADVRAM hardware). Especially 
for devices which are connected with their make ID it can be expected that they 
might eventually need more addressing space, and with this it will be future-proofed.
List of BIOS extension device ID's:
    
1	ASCII/Microsoft
2	Canon
3	Casio
4	Fujitsu
5	General
6	Hitachi
7	Kyocera
8	Matsushita (Panasonic)
9	Mitsubishi
10	NEC
11	Nippon Gakki
12	JVC
13	Philips
14	Pioneer
15	Sanyo
16	Sharp
17	SONY
18	Spectravideo
19	Toshiba
20	Mitsumi
128	Image Scanner (Matsushita)
247 Kanji 12x12 dots National (FS-4600 and Panasonic FS-A1FM)
254	MPS2 (ASCII)
*/

#include "SwitchedPorts.h"
#include "stdio.h"

SwitchedPorts::SwitchedPorts() {

	DBERR("SwitchedPorts constructor...\n");
	
 	reset(); // TODO: uiteindelijk niet hier!!!!!!
	DBERR("SwitchedPorts constructor...finished\n");
}

SwitchedPorts * SwitchedPorts::Instance() {

	/* implies singleton class */
	static SwitchedPorts deInstantie;
	return &deInstantie;
}

SwitchedPorts::~SwitchedPorts() {
	
	DBERR("SwitchedPorts destroyed.\n");
}

void SwitchedPorts::reset() {
    
    DBERR("RESET SwitchedPorts\n");

 	for (int i=0;i<256;i++) deviceEnabled[i] = false;
// 	deviceEnabled[0xf7] = true; 
    deviceID = 255;
}    

void SwitchedPorts::writeDeviceID(nw_byte value) {

	deviceID = value & 255;
	
}

nw_byte SwitchedPorts::readDeviceID() {
    
    if (deviceEnabled[deviceID]) return (~deviceID) & 255;
    return 255;
}

nw_byte SwitchedPorts::readPort(nw_word port) {
    
    if (deviceEnabled[deviceID]) {
        switch (deviceID) {
        case 0xf7: return 255; //Kanji::Instance()->readPort(port);
        }    
    }    
    return 255;
}

void SwitchedPorts::writePort(nw_word port, nw_byte value) {
    
    if (deviceEnabled[deviceID]) {
        switch (deviceID) {
        case 0xf7: break; //Kanji::Instance()->writePort(port, value);
        }    
    }    
}
