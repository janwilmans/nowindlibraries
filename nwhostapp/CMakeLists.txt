cmake_minimum_required(VERSION 2.6)
PROJECT(nwhostapp)

INCLUDE_DIRECTORIES(../general)
INCLUDE_DIRECTORIES(../nwhost)
INCLUDE_DIRECTORIES(../ftdx)
INCLUDE_DIRECTORIES(../nowind)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_library(FTDI ftdi NAMES ftdi PATHS /usr/lib /usr/local/lib /opt/local/lib /usr/lib/i386-linux-gnu)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

MESSAGE ("find_library FTDI: ${FTDI} <end>")
MESSAGE ("using Boost: ${Boost_LIBRARIES} <end>")

# manual override, not needed anymore
#set(CMAKE_EXE_LINKER_FLAGS "-lftdi -lboost_system -lboost_filesystem")

ADD_EXECUTABLE(nwhostapp mainUsbHost.cpp HostApp.cpp getopt.c getopt1.c)

target_link_libraries(nwhostapp nowind nwhost ftdx nwgeneral ${FTDI} ${Boost_LIBRARIES})

