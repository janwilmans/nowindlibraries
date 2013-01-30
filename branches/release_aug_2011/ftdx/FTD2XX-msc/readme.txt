ftd2xx.lib might be vs2008 specific, i've not tested it with mingw-gcc
it should be possible to create a .lib for mingw-gcc using:



Q. How can an MSVC program call an MinGW DLL, and vice versa?

A. Assume we have a testdll.h, testdll.c, and testmain.c. In the first case, we will compile testdll.c with MinGW, 
and let the MSVC-compiled testmain call it. You should use

    gcc -shared -o testdll.dll testdll.c \
        -Wl,--output-def,testdll.def,--out-implib,libtestdll.a

to produce the DLL and DEF files. MSVC cannot use the MinGW library, 
but since you have already the DEF file you may easily produce one by the Microsoft LIB tool:

    lib /machine:i386 /def:testdll.def

Once you have testdll.lib, it is trivial to produce the executable with MSVC:

    cl testmain.c testdll.lib

Now for MinGW programs calling an MSVC DLL. We have two methods. One way is to specify the LIB files directly on the command 
line after the main program (in newer MinGW versions; MinGW GCC 2.95.2 is reported not to work). For example, after

    cl /LD testdll.c

use

    gcc -o testmain testmain.c testdll.lib

The other way is to produce the .a files for GCC. For __cdecl functions (in most cases), it is simple: you only need to apply the reimp tool 
(the original site is unavailable now, but you may download here a version enhanced by José Fonseca):

    reimp testdll.lib
    gcc -o testmain testmain.c -L. -ltestdll

However, the above method does not work with __stdcall functions. For MSVC will prefix an underscore to __stdcall functions while MinGW will not. 
The right way is to produce the DEF file using the pexports tool (downloadable here if not found elsewhere) and filter off the first underscore by sed:

    pexports testdll.dll | sed "s/^_//" > testdll.def

Then, when using dlltool to produce the import library, add `-U' to the command line:

    dlltool -U -d testdll.def -l libtestdll.a

And now, you can proceed in the usual way:

    gcc -o testmain testmain.c -L. -ltestdll

Hooray, we got it. And that is the end of this FAQ.

2001-12-21, written by Wu Yongwei
2004-7-29, last revised by Wu Yongwei 

