PROJECT(nwhost)
INCLUDE_DIRECTORIES(../general)

ADD_LIBRARY(nwhost STATIC BlockRead.cc nwhost.cpp NowindHost.cc DataBlock.cc NowindHostDebug.cpp Device.cc NowindHostSupport.cc  NowindHostSupportDebug.cpp PartitionInfo.cpp Image.cpp ImageHandler.cpp nowindusb.cpp Response.cc BDOSProxy.cc Command.cc ReceiveRegisters.cc)
 
