//! EmulatorTester.h
#ifndef EMULATORTESTER_H
#define EMULATORTESTER_H

#include <list>
#include "msxtypes.h"
#include "cpu/Z80.h"
#include "video/V9938.h"

/*!
 * The Emulator test class contains the main-event-loop and it's setup/initialization
 */
class EmulatorTester {

private:
        Z80 *       cpu;
    	V9938 *		vdp;
        void        z80Tests();
        void        vdpTests();
        void		initTest();
public:
        void        start();
        static      EmulatorTester* Instance();
};
#endif
