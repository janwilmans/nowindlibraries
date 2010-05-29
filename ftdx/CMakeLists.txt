PROJECT(ftdx)
INCLUDE_DIRECTORIES(../general)

FIND_PATH(FTDI_INCLUDE_DIR ftdi.h /usr/include/ftdi /usr/local/include/ftdi)
ADD_LIBRARY(ftdx STATIC ConFtdiSio.cpp ConFTD2XX.cpp ConLibFtdi.cpp ftdx.cpp UsbStream.cpp)
