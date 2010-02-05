//! Debug.cpp

// no trace allocator for debug-messages (would create a mess in the logs)
#ifdef NO_TRACEALLOCATOR
	//undefine it first, if it was defined to prevent re-definition
	#undef NO_TRACEALLOCATOR
#endif
#define NO_TRACEALLOCATOR

#include "Debug.h"
#include <stdio.h>
#include <stdarg.h>
#include "cpu/Z80.h"

//logs to a file even if stderr is redirected
// #define LOG_TO_ERRFILE

#include "osd/LogWatcher.h"

#ifdef WIN32
	#include <windows.h>
#endif

using namespace std;

Debug::Debug() {

		// DBERR :: Don't use DBERR here, not in the constructor or initialize()!
		fillCount = 0;
		fillInit = false;
		fillLastDiff = 0;
}

Debug * Debug::Instance() {

		/* implies singleton class */
		static Debug deInstantie;
		return &deInstantie;
}

Debug::~Debug() {
		// destructor
}

void Debug::initialize() {

		// DBERR :: Don't use DBERR here, not in the constructor or initialize()!
#ifdef LOG_TO_ERRFILE
    	string cerrfilename = getPath() + "nowind_stderr.txt";
    	cerrfile = new fstream(cerrfilename.c_str(),ios::binary | ios::out | ios::trunc);
    	if (cerrfile->fail()) {
    		cerr << "Error opening file " << cerrfilename << " for read/write !\n";
    		exit(1);
    	}   
#endif // LOG_TO_ERRFILE
}

string Debug::getPath() {
    return workingDirectory;
}

void Debug::setPath(string path) {
    workingDirectory = path;
}

void Debug::fill(unsigned int amount) {

		if (!fillInit) {
				fillInit = true;
				fillCount = amount;
				DBERR("fillInit at: %x\n", fillCount);
		}

		if (abs((int)(fillCount-amount)) > 256) {
				DBERR("stack RESYNC!\n");
				fillCount = amount+fillLastDiff;
		}

		unsigned int diff = fillCount - amount;
		DBERR(".");
		char level[250];
		sprintf(level,"%5i %X ",diff, amount);
		DBERR(level);
		DBERR("..");

		diff = diff & 63;

		for (unsigned int j=0;j<diff;j++) {
				for (unsigned int k=0;k<1;k++) {
						DBERR(".");
				}
		}
		fillLastDiff = diff;
}

void Debug::memDump(const nw_byte *mem) {

/*
=32031== Use of uninitialised value of size 4
==32031==    at 0x40302F06: _IO_vfprintf (in /lib/libc.so.6)
==32031==    by 0x4031AA85: _IO_vsprintf (in /lib/libc.so.6)
==32031==    by 0x4030B06D: sprintf (in /lib/libc.so.6)
==32031==    by 0x8059513: Debug::memDump(unsigned char *) (in /home/jan/cvs/nowind/main)

00  00  00  00  00  00  00  00  - ==32031==
==32031== Use of uninitialised value of size 4
==32031==    at 0x40302F06: _IO_vfprintf (in /lib/libc.so.6)
==32031==    by 0x4031AA85: _IO_vsprintf (in /lib/libc.so.6)
==32031==    by 0x4030B06D: sprintf (in /lib/libc.so.6)
==32031==    by 0x80595B3: Debug::memDump(unsigned char *) (in /home/jan/cvs/nowind/main)

*/


		DBERR("(address: 0x%08X\n", mem);
		char temp[250];
		//memset(&temp[0],0,250);

		int index = 0;
		for (int i = 0;i<4;i++) {
				for (int k = 0;k<8;k++) {
						nw_word j = (nw_word) (mem[index] & 255);
						sprintf(temp,"%02x ",j);
						index++;
						DEBUGERROR(temp << " ");
				}
				DBERR("- ");
				for (int k = 0;k<8;k++) {
						nw_word j = (nw_word) (mem[index] & 255);
						sprintf(temp,"%02x ",j);
						index++;
						DEBUGERROR(temp << " ");
				}
				index -=16;
				for (int k = 0;k<8;k++) {
						nw_word j = (nw_word) (mem[index] & 255);
						if (j>31 && j <140) {
								DEBUGERROR(mem[index]);
						} else {
								DBERR(".");
						}
						index++;
				}
				DBERR(" - ");
				for (int k = 0;k<8;k++) {
						nw_word j = (nw_word) (mem[index] & 255);
						if (j>31 && j <140) {
								DEBUGERROR(mem[index]);
						} else {
								DBERR(".");
						}
						index++;
				}
				DEBUGERROR(endl);
		}
}


void Debug::memDump(nw_byte *cpuPageRead[4], nw_word address) {

		memDump(cpuPageRead,address,address);
}

void Debug::memDump(nw_byte *mem[4], nw_word address, nw_word marker) {

		nw_word realAddress = address & 0xFFF0;
		nw_word page = address >> 14;
		nw_word pageOffset = address & 0x3FF0;

		DBERR("(pysical start of page at address: $%04X)\n", (unsigned long) mem[page]);
		DBERR("Dump of address: $%04X, address %#04X is marked.\n", realAddress, marker);

		char temp[250];
		for (int i = 0;i<18;i++) {

				sprintf(temp,"%04x: ",realAddress);
				DBERR(temp);

				DBERR("page: %u   ", page);

				for (int k = 0;k<8;k++) {
						nw_word j = (nw_word) (mem[page][pageOffset] & 255);
						if (realAddress == marker) {
								sprintf(temp,"[%02x]",j);
						} else {
								sprintf(temp," %02x ",j);
						}
						DBERR(temp);
						pageOffset++;
						realAddress++;
						page = realAddress >> 14;
				}
				DBERR("- ");
				for (int k = 0;k<8;k++) {
						nw_word j = (nw_word) (mem[page][pageOffset] & 255);
						if (realAddress == marker) {
								sprintf(temp,"[%02x]",j);
						} else {
								sprintf(temp," %02x ",j);
						}
						DBERR(temp);
						pageOffset++;
						realAddress++;
						page = realAddress >> 14;
				}
				pageOffset -= 16;
				realAddress -= 16;
				for (int k = 0;k<8;k++) {
						nw_word j = (nw_word) (mem[page][pageOffset] & 255);
						if (j>31 && j <140) {
								DBERR("%c", j);
						} else {
								DBERR(".");
						}
						pageOffset++;
						realAddress++;
						page = realAddress >> 14;
				}
				DBERR(" - ");
				for (int k = 0;k<8;k++) {
						nw_word j = (nw_word) (mem[page][pageOffset] & 255);
						if (j>31 && j <140) {
								DBERR("%c", j);
						} else {
								DBERR(".");
						}
						pageOffset++;
						realAddress++;
						page = realAddress >> 14;
				}
				DBERR("\n");

				if (realAddress == (realAddress & 0xFF00)) { DBERR("\n"); }

		}
}

// WARNING: HARDCODED limit of 2000 bytes !
#define MAXMSG 2000	

void Debug::debug_wrap_sprintf_erl(const char *file, int line, const char *cFormat, ...)
{
	va_list arg_list;
	char scratch[MAXMSG];
	char scratch_with_sourceline[MAXMSG+260];
	va_start (arg_list, cFormat);  
	VSPRINTF(scratch, MAXMSG, cFormat, arg_list);
	va_end (arg_list);

	SNPRINTF(scratch_with_sourceline, MAXMSG+260, "%s (%s:%u)\n", scratch, file, line);

	// we need a compatible MSC alternative?
	//	SNPRINTF(scratch_with_sourceline, MAXMSG+260, MAXMSG+260, "%s (%s:%u)\n", scratch, file, line);


#ifdef LOG_TO_ERRFILE
	*Debug::Instance()->cerrfile << scratch_with_sourceline;
	Debug::Instance()->cerrfile->flush();
	cerr << scratch_with_sourceline;
#else 
	cerr << scratch_with_sourceline;
#endif

#ifdef WIN32
	OutputDebugStringA(scratch_with_sourceline);
#endif

}

void Debug::debug_wrap_sprintf(const char *cFormat, ...)
{
	va_list arg_list;
	char scratch[MAXMSG];
	
	va_start (arg_list, cFormat);  
	VSPRINTF(scratch, MAXMSG, cFormat, arg_list);
	va_end (arg_list);

	//LogWatcher::Instance()->addLogLine(scratch);

#ifdef LOG_TO_ERRFILE
	*Debug::Instance()->cerrfile << scratch;
	Debug::Instance()->cerrfile->flush();
	cerr << scratch;
#else 
	cerr << scratch;
#endif

#ifdef WIN32
	OutputDebugStringA(scratch);
#endif
}

void Debug::memFileDump(const nw_byte *mem[8], char *file) {

		string filename(file);

		// delete the existing? file 
		ofstream ofs_delete(filename.c_str(),ios::trunc);
		ofs_delete.close();

		ofstream ofs(filename.c_str(),ios::binary);
		if (ofs.fail()) {
				DBERR("Error opening file %s!\n", filename.c_str());
		}
		for (unsigned int b=0;b<8;b++) { 
			ofs.write((char *)mem[b],8*1024);
		}
		ofs.close();
}

void Debug::debugError(const char *err, const char *file, int line) {

	DBERR("%s (%s:%u)\n", err, file, line);
}
