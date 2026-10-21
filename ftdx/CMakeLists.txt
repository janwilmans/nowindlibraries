PROJECT(ftdx)
INCLUDE_DIRECTORIES(../general)

ADD_LIBRARY(ftdx STATIC ConFTD2XX.cpp ftdx.cpp UsbStream.cpp)

if(NOT WIN32)
  find_library(FTDI ftdi NAMES ftdi PATHS /usr/lib /usr/local/lib /opt/local/lib /usr/lib/i386-linux-gnu)
  # the linux kernel (ftdi_sio) and libftdi backends are not available on Windows
  target_sources(ftdx PRIVATE ConFtdiSio.cpp ConLibFtdi.cpp)
endif()
