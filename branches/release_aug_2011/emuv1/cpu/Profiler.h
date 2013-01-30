//! Profiler.h
#ifndef PROFILER_H
#define PROFILER_H

#include <msxtypes.h>
#include "cpu/Z80.h"
#include <iostream>

class Disassembler;

class Profiler {

private:
    
    unsigned int opcodeCounter[256];
    unsigned int opcodeCounterCB[256];
    unsigned int opcodeCounterDD[256];
    unsigned int opcodeCounterED[256];
    unsigned int opcodeCounterFD[256];
    unsigned int opcodeCounterDDCB[256];
    unsigned int opcodeCounterFDCB[256];
    unsigned int total;
		
public:

    Profiler(); 
	~Profiler();
	void count(nw_byte, nw_byte);
};

#endif
