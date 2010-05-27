cmake_minimum_required(VERSION 2.6)

PROJECT(nwhostapp)
INCLUDE_DIRECTORIES(../general)
INCLUDE_DIRECTORIES(../nwhost)
INCLUDE_DIRECTORIES(../ftdx)
INCLUDE_DIRECTORIES(../nowind)

SUBDIRS(../general)
SUBDIRS(../nwhost)
SUBDIRS(../ftdx)
SUBDIRS(../nowind)

# LINK_DIRECTORIES(../general)
# LINK_DIRECTORIES(../nwhost)
# LINK_DIRECTORIES(../ftdx)
# LINK_DIRECTORIES(../nowind)

ADD_EXECUTABLE(nwhostapp mainUsbHost.cpp HostApp.cpp getopt.c getopt1.c)

TARGET_LINK_LIBRARIES(nwhostapp general nwhost ftdx nowind)
