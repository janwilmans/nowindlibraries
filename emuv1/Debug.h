//! Debug.h
#ifndef DEBUG_H
#define DEBUG_H

#include "msxtypes.h"

#include <iostream>
#include <fstream>
#include <string>

#include "TraceAllocInclude.h"

/* when ON enables writing of debugging info to stdout/stderr 
(or stdout.txt and stderr.txt) */
#define DEBUGGING_ON

/* This is default on most unix-like system but doesn't work on windows,
   so turn it on only if your environment supports cout / cerr */
#define USE_NORMAL_STDOUTERR_OFF

/* when ON enables logging of all calls and jumps to stderr */
#define STACKTRACK_OFF

/* when ON enables logging of all executed instructions */
#define INSTRUCTIONS_ON

/* when ON enables logging of all IO (OUT) commands executed */
#define OUT_INSTRUCTIONS_OFF

/* when ON enables logging of all writes to PSG registers */
#define PSG_COMMANDS_OFF

/* when ON enables logging of FDC commands */
#define FDCLOG_OFF

/* when ON disables SDL's video and audio subsystems and all calls to them */
#define CONSOLE_DEBUGGING_OFF

/* when ON disables invalid opcode checking, disassembler and some more things for best speed */
#define FULL_SPEED_OFF

#if __GNUC__ == 4
	#define likely(x)       __builtin_expect(!!(x), 1)
	#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
	#define likely(x)       (x)
	#define unlikely(x)     (x)
#endif

/******************************************************************/

#ifndef DEBUGGING_ON
		#define DEBUGERROR(DEMSG) //
		#define DEBUGMESSAGE(DEMSG) //
#endif

#include "assert.h"

// This shouldn't actually be in this header, but in a separate compiler/platform header
// TODO: create such a header
#ifndef DIR_SEPARATOR
#ifdef WIN32
    #define DIR_SEPARATOR "\\"
#else
    #define DIR_SEPARATOR "/"
#endif
#endif

#ifdef DEBUGGING_ON
   	#define DEBUGERROR(DEMSG) std::cerr << DEMSG
	#define DBERL(e) Debug::debug_wrap_sprintf_erl(__FILE__, __LINE__, e)
   	#define DBERR Debug::debug_wrap_sprintf
#endif

#ifdef _MSC_VER
    #define SNPRINTF _snprintf
    #define VSPRINTF vsprintf_s
#else
    #define SNPRINTF snprintf
    #define VSPRINTF vsnprintf
#endif

/*!
 * The Debug class defines DEBUGERROR and DEBUGMESSAGE used through-out this project
 * these can be globally disabled by defining "DEBUGGING_OFF" instead of "DEBUGGING_ON"
 */
class Debug {

private:
						Debug();
        
public:

		static          Debug * Instance();
		void            initialize();

        std::string     getPath();
        void            setPath(std::string);
        std::string     workingDirectory;
		
		void            memDump(const nw_byte *);
	
		void            memDump(nw_byte *cpuPageRead[4], nw_word);
		void            memDump(nw_byte *cpuPageRead[4], nw_word, nw_word);
		void            memFileDump(const nw_byte *mem[8], char *);
		void            fill(unsigned int);
		bool            fillInit;
		unsigned int    fillCount;
		unsigned int    fillLastDiff;
		bool            RUNTIME_INSTRUCTIONS_ON;
    	std::ostream 	*cerrfile;
    	std::ostream 	*coutfile;
    		
        void            debugError(const char*, const char*, int);

		static void debug_wrap_sprintf_erl(const char *file, int line, const char *cFormat, ...);
        static void debug_wrap_sprintf(const char *cFormat, ...);

						~Debug();
};

#endif
