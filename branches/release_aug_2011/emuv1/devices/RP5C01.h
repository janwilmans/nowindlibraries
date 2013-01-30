//! RP5C01.h
#ifndef RP5C01_H
#define RP5C01_H

#include <string>
#include "msxtypes.h"

#define ALARMBLOCK 0

/*!
 * The RP5C01 class emulates the CLOCKChip
 */
class RP5C01 {

private:

	nw_byte        indexRegister;
	nw_byte        dataRegister[13][3]; // BLOCK 01-03 (BLOCK00 directly derived from localtime)
	nw_byte        modeRegister;
    std::string	   rtcFilename;
    
public:

	RP5C01();
	~RP5C01();

	void        writePortB4(nw_byte);
	void        writePortB5(nw_byte);
	nw_byte		readPortB5();
};

#endif

