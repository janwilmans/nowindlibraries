// F4Device.ccp

// TODO: uitzoeken hoe dit register precies reageert
// onderscheid maken tussen MSX2+ en Turbo-R
// http://map.tni.nl/resources/msx_io_ports.php#sysflags
// http://www.funet.fi/pub/msx/mirrors/hanso/hwmodsetc/iopoort.txt

#include "stdio.h"
#include "F4Device.h"
#include "Debug.h"

using namespace std;

F4Device::F4Device() {

	DBERR("F4Device constructor...\n");
	portF4 = 0;
	DBERR("F4Device constructor...finished\n");
}

F4Device::~F4Device() {
	
	DBERR("F4Device destroyed.\n");
}

void F4Device::reset() {
	portF4 = 0;
}

void F4Device::writeRegister(nw_byte value) {

	DBERR("WARNING! F4Device not implemented correct!\n");
	// bit 5 can only be written once
	portF4 = (portF4 & 0x20) | (value & 0xa0);
}

nw_byte F4Device::readRegister() {
	
	DBERR("WARNING! F4Device not implemented correct!\n");
	return portF4;
}
