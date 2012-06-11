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


