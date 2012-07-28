PROJECT(ftdx)
INCLUDE_DIRECTORIES(../general)

find_library(FTDI ftdi NAMES ftdi PATHS /usr/lib /usr/local/lib /opt/local/lib /usr/lib/i386-linux-gnu)

ADD_LIBRARY(ftdx STATIC ConFtdiSio.cpp ConFTD2XX.cpp ConLibFtdi.cpp ftdx.cpp UsbStream.cpp)
