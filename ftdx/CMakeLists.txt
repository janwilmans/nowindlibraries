PROJECT(ftdx)
INCLUDE_DIRECTORIES(../general)

ADD_LIBRARY(ftdx STATIC ConFtdiSio.cpp ConFTD2XX.cpp ConLibFtdi.cpp ftdx.cpp UsbStream.cpp)
