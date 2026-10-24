this repository contains:

- nowind libraries
- the nwhostapp application
- the NowindInterfaceHostGUI win32 front for the commandline nwhostapp application

external dependencies:
- libusb-dev
- libftdi-devel   (sudo yum install libftdi-devel on Fedora, extract sources on windows)

see build-readme.txt

For MacOSX (Leopard):

- install Xcode
- install Xcode Commandline tools (under Xcode->Downloads)
- install macports (http://www.macports.org)
- sudo port install subversion
- sudo port install cmake
- sudo port install libftdi
- sudo port install boost
- use 'cmake -G 'Unix Makefiles' to generate makefiles.
- run 'make'

For MacOSX (Mountain Lion),

Actually, just like on Leopard, except if you want to create universal binaries to
run on both 32-bit and 64-bit systems, then use: (notice the +universal)

- install Xcode
- install Xcode Commandline tools (under Xcode->Downloads)
- install macports (http://www.macports.org)
- sudo port install subversion
- sudo port install cmake
- sudo port install libftdi +universal
- sudo port install boost +universal
- use cmake -G "Unix Makefiles" -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.5
- run 'make'

problems compiling or using the nowind? submit an issue at https://github.com/janwilmans/nowindlibraries/issues
using cmake is an attempt to make cross-compiling easier, but so far it seems to break on some
new linux distros, so feel free to contact me, mention these things:
- linux distribution
- the output of cmake / gcc
