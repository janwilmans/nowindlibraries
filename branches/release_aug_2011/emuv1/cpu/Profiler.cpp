// Profiler.ccp

#include "Profiler.h"
#include "Debug.h"
#include "cpu/Disassembler.h"
#include <algorithm>

using namespace std;

Profiler::Profiler() {

    DBERR("Profiler constructor...\n");

	/*
    memset(opcodeCounter, 0, sizeof(opcodeCounter));
    memset(opcodeCounterCB, 0, sizeof(opcodeCounterCB));
    memset(opcodeCounterDD, 0, sizeof(opcodeCounterDD));
    memset(opcodeCounterED, 0, sizeof(opcodeCounterED));
    memset(opcodeCounterFD, 0, sizeof(opcodeCounterFD));
	*/
//    memset(opcodeCounterDDCB, 0, sizeof(opcodeCounterDDCB));
//    memset(opcodeCounterFDCB, 0, sizeof(opcodeCounterFDCB));
    total = 0;
}

Profiler::~Profiler() {
/*
    DBERR("Profiler destructor...\n");
    DBERR("  Instructions executed: %u\n", total);

    for (int i=0;i<256;i++) {
        string mnemonic = string(Disassembler::Instance()->disAsm(0x0000, i, 0));
        float average = (100.0 * float(opcodeCounter[i]))/float(total);
        DBERR("   %2f%%    0x%02x %s\n", average, i, mnemonic.c_str());
    }
    for (int i=0;i<256;i++) {
        string mnemonic = string(Disassembler::Instance()->disAsm(0x0000, 0xcb+(i<<8), 0));
        float average = (100.0 * float(opcodeCounterCB[i]))/float(total);
        DBERR("   %2f%% CB 0x%02x %s\n", average, i, mnemonic.c_str());
    }
    for (int i=0;i<256;i++) {
        string mnemonic = string(Disassembler::Instance()->disAsm(0x0000, 0xdd+(i<<8), 0));
        float average = (100.0 * float(opcodeCounterDD[i]))/float(total);
        DBERR("   %2f%% DD 0x%02x %s\n", average, i, mnemonic.c_str());
    }
    for (int i=0;i<256;i++) {
        string mnemonic = string(Disassembler::Instance()->disAsm(0x0000, 0xed+(i<<8), 0));
        float average = (100.0 * float(opcodeCounterED[i]))/float(total);
        DBERR("   %2f%% ED 0x%02x %s\n", average, i, mnemonic.c_str());
    }
    for (int i=0;i<256;i++) {
        string mnemonic = string(Disassembler::Instance()->disAsm(0x0000, 0xfd+(i<<8), 0));
        float average = (100.0 * float(opcodeCounterFD[i]))/float(total);
        DBERR("   %2f%% FD 0x%02x %s\n", average, i, mnemonic.c_str());
    }
*/
	DBERR("Profiler destroyed.\n");
}

void Profiler::count(nw_byte opcode, nw_byte operand) {

    // TODO: reg_pc meegeven en zelf geheugen lezen voor operands en eventueel extra bytes
    switch (opcode) {
        case 0xcb: opcodeCounterCB[operand]++; break;
        case 0xdd: opcodeCounterDD[operand]++; break;
        case 0xed: opcodeCounterED[operand]++; break;
        case 0xfd: opcodeCounterFD[operand]++; break;
        default: opcodeCounter[opcode]++;
    }
    total++;  
}
