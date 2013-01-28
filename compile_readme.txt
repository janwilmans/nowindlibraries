this repository contains:

- nowind libraries
- the nwhostapp application
- the NowindInterfaceHostGUI win32 front for the commandline nwhostapp application

external dependencies:
- libusb-dev
- libftdi-devel   (sudo yum install libftdi-devel on Fedora, extract sources on windows)
- boost >1.44

Example of compiling on Fedora 13:
- make sure boost headers are in /usr/include/boost, see BOOST) below
- export BOOST_ROOT=/usr
- sudo yum -y install cmake
- sudo yum -y install libftdi-devel
- sudo yum -y install boost-devel
- sudo yum -y install gcc
- sudo yum -y install gcc-g++
- use 'cmake -G 'Unix Makefiles' to generate makefiles.
- run 'make' 

For Debian 64 or Ubuntu 12:

- sudo apt-get update
- sudo apt-get libftdi-dev
- sudo apt-get cmake
- sudo apt-get subversion
- sudo apt-get gcc
- sudo apt-get g++
- sudo apt-get libboost-filesystem*
- use 'cmake -G 'Unix Makefiles' to generate makefiles.
- run 'make' 

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

Actually, just like on Leopard, except if you want to create 32bit binaries to
run on both 32-bit and 64-bit systems, then use: (notice the +universal)

- install Xcode
- install Xcode Commandline tools (under Xcode->Downloads)
- install macports (http://www.macports.org)
- sudo port install subversion
- sudo port install cmake
- sudo port install libftdi +universal
- sudo port install boost +universal
- use 'cmake -G 'Unix Makefiles' -DCMAKE_OSX_ARCHITECTURES=i386 to generate makefiles.
- run 'make' 

problems compiling or using the nowind? Post on msx.org or mail directly to me (janwilmans@gmail.com)
using cmake is an attempt to make cross-compiling easier, but so far it seems to break on some
new linux distros, so feel free to contact me, mention these things:
- linux distribution
- the output of cmake, and 'make' and 'make -d' (the latter includes debug info)
- check for locate *boost*.a* if no .a files are present, you need to install boost-system, and boost-filesystem not 'just'
  the header-only part of boost. Also check: locate libftdi.a it should also be found
