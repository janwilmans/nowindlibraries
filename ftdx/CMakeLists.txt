PROJECT(ftdx)
INCLUDE_DIRECTORIES(../general)

ADD_LIBRARY(ftdx STATIC ConFTD2XX.cpp ftdx.cpp UsbStream.cpp)

if(WIN32)
  # static x64 FTD2XX library, the header is included relative to this directory (FTD2XX-msc/FTD2XX.H)
  set(FTDI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/FTD2XX-msc/FTD2XX.lib)
else()
  find_library(FTDI_LIB NAMES ftdi1 REQUIRED)
  find_path(FTDI_INCLUDE_DIR
      NAMES ftdi.h
      PATH_SUFFIXES libftdi1
      REQUIRED
  )
  target_include_directories(ftdx PRIVATE ${FTDI_INCLUDE_DIR})
  # the linux kernel (ftdi_sio) and libftdi backends are not available on Windows
  target_sources(ftdx PRIVATE ConFtdiSio.cpp ConLibFtdi.cpp)
endif()

MESSAGE ("find_library FTDI: ${FTDI_LIB}")

target_link_libraries(ftdx PUBLIC ${FTDI_LIB})
