cmake_minimum_required(VERSION 2.6)

PROJECT(nwhostapp)

INCLUDE_DIRECTORIES(../general)
INCLUDE_DIRECTORIES(../nwhost)
INCLUDE_DIRECTORIES(../ftdx)
INCLUDE_DIRECTORIES(../nowind)

ADD_EXECUTABLE(nwhostapp mainUsbHost.cpp HostApp.cpp getopt.c getopt1.c)
TARGET_LINK_LIBRARIES(nwhostapp ftdi nowind nwhost ftdx nwgeneral)
