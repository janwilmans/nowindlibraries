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

Or for Debian 64 or Ubuntu 12:

- sudo apt-get update
- sudo apt-get libftdi-dev
- sudo apt-get cmake
- sudo apt-get subversion
- sudo apt-get gcc
- sudo apt-get g++
- sudo apt-get libboost-filesystem*
- use 'cmake -G 'Unix Makefiles' to generate makefiles.
- run 'make' 


problems compiling ? Post on msx.org or mail directly to me (janwilmans@gmail.com)
using cmake is an attempt to make cross-compiling easier, but so far it seems to break a lot
on new linux distros, so feel free to contact me, mention these thinks:
- linux distribution
- the output of cmake, and 'make' and 'make -d' (the latter includes debug info)
- check for locate *boost*.a* if no .a files are present, you need to install boost-system, and boost-filesystem not 'just'
  the header-only part of boost. Also check: locate libftdi.a it should also be found
