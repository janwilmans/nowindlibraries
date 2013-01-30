
// move to compiler.h?
#ifndef _MSC_VER
	#include <getopt.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <list>

#include "Debug.h"

#include "Emulator.h"
#include "EmulatorTester.h"
#include "DiskInterface.h"
#include "Config.h"
#include "Media.h"

#include "SDL.h"

#include "devices/DebugDevice.h"

using namespace std;

/*
#ifdef WIN32
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#endif
*/

/* 
    ==MacOS==
    It is vital to use SDLmain on MacOS because it initializes the Foundation and AppKit frameworks
    also, the -ffunction-sections compiler flag should not be used on MacOs,
    because it triggers a bug or unsupported feature of darwin's gcc 3.3 
    finally -lobjc -framework Foundation -framework AppKit should be added to the linker options.

   ==WINDOWS==
    At run time, sdl.dll and sdl_image.dll will need to be available. 
    Windows will search directories in the following order for these dll's:
    
    The directory from which the application loaded. 
    The current directory. 
    The Windows system directory. 
    The directories that are listed in the PATH environment variable. 
    
    ==PORTABILITY==
    
    use -funsigned-char/-fsigned-char to test portability (char is signed by default on some system
    and unsigned on others)
    
    for the gcc (and derivitives like darwin gcc for macOS) the following precompiler directived are available:
    __GNUC__ contains the gcc (major) version number (so it will equal 4 for gcc v4.xx)
    __APPLE__ will be 1 on MacOS
    
    #if _MSC_VER >= 1400 // this is Visual C++ 2005 or above
    #elif _MSC_VER >= 1310 // this is Visual C++ .NET 2003
    #elif _MSC_VER > 1300 // this is Visual C++ .NET 2002
    #endif

*/

void exitRoutine();

class TestA {
	int a;
};

typedef unsigned int ourNewCycles_t;

class ClockCycle {
public:
   ourNewCycles_t amount;
   bool b_true;
   ourNewCycles_t getAmount() { return amount; }
   ourNewCycles_t getAmountChecked() { 
       if (b_true) return amount; 
       return amount; 
   }
};

ClockCycle* pClockCycle;
unsigned int g_amount;

#define TIMES 100000000

int main2(int argc, char *argv[])
{
    pClockCycle = new ClockCycle();
    unsigned int& rClockCycle = pClockCycle->amount;

    {
        int s = SDL_GetTicks();
        for (unsigned int t=0;t<TIMES;t++)
        {
            g_amount += 9;
        }
        int e = SDL_GetTicks();
        DBERR("g_amount takes: %d ms\n", e-s);
    }
    
    {
        int s = SDL_GetTicks();
        for (unsigned int t=0;t<TIMES;t++)
        {
            pClockCycle->amount += 9;
        }
        int e = SDL_GetTicks();
        DBERR("pClockCycle->amount takes: %d ms\n", e-s);
    }

    {
        int s = SDL_GetTicks();
        for (unsigned int t=0;t<TIMES;t++)
        {
            rClockCycle += 9;
        }
        int e = SDL_GetTicks();
        DBERR("rClockCycle takes: %d ms\n", e-s);
    }

    {
        int s = SDL_GetTicks();
        ourNewCycles_t t1 = pClockCycle->getAmount();
        for (unsigned int t=0;t<TIMES;t++)
        {
            t1 += 9;
        }
        int e = SDL_GetTicks();
        DBERR("pClockCycle->getAmount(): %d ms\n", e-s);
    }

    {
        int s = SDL_GetTicks();
        for (unsigned int t=0;t<TIMES;t++)
        {
            ourNewCycles_t t1 = pClockCycle->getAmount();
        }
        int e = SDL_GetTicks();
        DBERR("pClockCycle->getAmount(): %d ms\n", e-s);
    }

    {
        int s = SDL_GetTicks();
        for (unsigned int t=0;t<TIMES;t++)
        {
            ourNewCycles_t t1 = pClockCycle->getAmountChecked();
        }
        int e = SDL_GetTicks();
        DBERR("pClockCycle->getAmountChecked(): %d ms\n", e-s);
    }


	return 0;
}



int main(int argc, char *argv[])
{ 
    /* first of all, set the path were logfiles are written */
    string programPath(argv[0]);
    
    // gdb does something weird on win32, it starts the program as: D:\project\nowind\emuv1/nowindcons.exe 
    // dunno how to fix this good, so a quick fix for now:
    int last = programPath.find_last_of("\\");
    int last2 = programPath.find_last_of("/");
    if (last2 > last) last = last2;

    if (last > 1) {
        string workDir = programPath.substr(0,last+1);
        Debug::Instance()->setPath(workDir);
    } else {
        Debug::Instance()->setPath("");
    }          

    // call Debug::Instance()->setPath BEFORE Debug::Instance()->initialize();
    // and call Debug::Instance()->initialize() BEFORE the first use of DBERR !!!
    Debug::Instance()->initialize();

#ifdef WIN32
    _putenv("SDL_VIDEODRIVER=directx");
#endif    

	list<string> media;
#ifndef _MSC_VER
    DBERR("Parse commandline parameters...\n");

    /* parse the commandline parameters */
    char *cvalue = NULL;
    int index, c;
    
     
    /* by default getopt will permute the contents of argv while scanning it so 
     * that eventually all the non-options are at the end. This allows options to 
     * be given in any order, even with programs that were not written to expect this.
     */
     
    /* option string: characters in the represent valid options, a trailing : 
     * means the option also takes na argument so:
     * "a" allows -a  
     * "a:" allows -a <somearg> 
     * "a::" means -a [<somearg>] (so the argument is optional)
     */
     
    /* in getopt.h some of the argv globals are explained (ie optind) */
    bool hasErrors = false;
    opterr = 1; 
    while (1)
    {
        int option_index = 0;
        static struct option long_options[] =
        {
          {"add", 1, 0, 0},
          {"append", 0, 0, 0},
          {"delete", 1, 0, 0},
          {"verbose", 0, 0, 0},
          {"create", 1, 0, 'c'},
          {"file", 1, 0, 0},
          {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "dwi:", long_options, &option_index);

        // no more parameters? stop...
        if (c == -1) break;

        switch (c)
        {
        case 0: // it was a long-option
          DBERR("option %s", long_options[option_index].name);
          if (optarg)
             DBERR(" with arg %s", optarg);
          DBERR("\n");
          break;
        
        // handle other options (short with and without arguments)
          
        case 'd':   putenv("SDL_VIDEODRIVER=dummy"); break;
        case 'w':   putenv("SDL_VIDEODRIVER=windib"); break;
        case 'i':
          media.push_back(string(optarg));
          cvalue = optarg;
          // note: all arguments are strings! conversion a later problem :) 
          DBERR("found i value: %s\n", optarg);
          break;

        case '?':
          hasErrors = true;
          break;
          // nothing to do here (getopt gives its own error messages)
        default:
         DBERR ("getopt returned unexpected code 0x%X ??\n", c);
        }
    }
    if (optind < argc) {
      while (optind < argc)
      {
        printf ("unknown option: %s\n", argv[optind]);
        optind++;
      }
    }
    if (hasErrors) return -1;
#endif // _MSC_VER 

//    putenv("SDL_AUDIODRIVER=disk");
    _putenv("SDL_VIDEO_CENTERED=center");

#ifndef CONSOLE_DEBUGGING_ON
	if( SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) <0 ) {
		DBERR("Unable to init SDL: %s\n", SDL_GetError());
		return 1;
	}
	
    SDL_Rect **modes;
    modes=SDL_ListModes(NULL, SDL_HWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF|SDL_SWSURFACE|SDL_NOFRAME|SDL_OPENGL);

    /* Check if there are any modes available */
    if(modes == (SDL_Rect **)0){
        DBERR("No modes available!\n");
        exit(-1);
    }
    

    /* Check if our resolution is restricted */
/*
    if(modes == (SDL_Rect **)-1){
        DBERR("All resolutions available.\n");
    }
    else{
        // Print valid
        DBERR("Available Modes\n");
        for(int i=0;modes[i];++i)
        DBERR("  %d x %d\n", modes[i]->w, modes[i]->h);
    }
*/

#endif

#ifdef CONSOLE_DEBUGGING_ON
	if( SDL_Init(SDL_INIT_TIMER ) <0 ) {
		DBERR("Unable to init SDL: %s\n", SDL_GetError());
		return 1;
	}
#endif


    /* install an exit handler */
	atexit(exitRoutine); //(&Emulator::quit);

    DBERR("Nowind v1.0\n");

	/*
	// CheckSanity();
	TestA * t = new TestA();
	unsigned int * bvp = (unsigned int *) t;
	bvp--;
	unsigned int bv = *bvp;

	unsigned char * evp = (unsigned char *) t;
	evp =  evp + sizeof(t);
	unsigned int * evp2 = (unsigned int *) evp;
	unsigned int ev = *evp2;
	// *bvp = 1;
	// CheckSanity();
	*/

    DBERR("Emulator initializing...\n");
	Emulator::Instance()->initialize();		// this constructs EVERYTHING

    if (media.size() == 0) // no disks or roms specified on the commandline
    {
        // try image.dsk in the emulator-directory
        Media::Instance()->insertMedia(Debug::Instance()->getPath() + "../disks/dos1.dsk",0);
    }
    else
    for(list<string>::iterator i = media.begin(); i!=media.end();i++)
    {
        //todo: fix this media priority crap :( 
        // (any third or above image or disk will just overidde the second)
        // even worse: -i some.rom -i foo.dsk will insert the disk in de second drive!
        // just because some other media came before it on the cmdline 
        static int firstMedia = 0; // static, so initialized just once.
        DBERR("media: %s, insert with priority: %u\n", (*i).c_str(), firstMedia);
        Media::Instance()->insertMedia((*i).c_str(), firstMedia);
        firstMedia = 1;
    }   

    /* execute some tests to see if we're still sane */
	//EmulatorTester::Instance()->start();

//Media::Instance()->insertNowindImage("hdimage.nowind", 4*1024*1024);
    
	Emulator::Instance()->start();

	/* exit program */
	return 0;
}

void exitRoutine() {
#ifndef NO_TRACEALLOCATOR
   DumpReport();
#endif   
}
