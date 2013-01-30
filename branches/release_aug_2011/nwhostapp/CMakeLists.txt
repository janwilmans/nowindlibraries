cmake_minimum_required(VERSION 2.6)

PROJECT(nwhostapp)

INCLUDE_DIRECTORIES(../general)
INCLUDE_DIRECTORIES(../nwhost)
INCLUDE_DIRECTORIES(../ftdx)
INCLUDE_DIRECTORIES(../nowind)

find_library(FTDI ftdi /usr/lib /usr/local/lib /opt/local/lib)
find_library(BOOST_FS3 boost_filesystem /usr/lib /usr/local/lib /opt/local/lib)
find_library(BOOST_SYS boost_system /usr/lib /usr/local/lib /opt/local/lib)

ADD_EXECUTABLE(nwhostapp mainUsbHost.cpp HostApp.cpp getopt.c getopt1.c)
TARGET_LINK_LIBRARIES(nwhostapp 
  ${FTDI} nowind nwhost ftdx nwgeneral
  ${BOOST_FS3} 
  ${BOOST_SYS} 
)
