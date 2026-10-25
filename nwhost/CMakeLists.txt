# nwhost: the host side of the nowind protocol
add_library(nwhost STATIC
    BDOSProxy.cc BlockRead.cc BlockWrite.cc Command.cc DataBlock.cc DataBlockWrite.cc Device.cc Image.cpp
    ImageHandler.cpp NowindHost.cc NowindHostDebug.cpp NowindHostSupport.cc NowindHostSupportDebug.cpp
    PartitionInfo.cpp ReceiveRegisters.cc Response.cc nowindusb.cpp nwhost.cpp)
target_include_directories(nwhost PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(nwhost PUBLIC NOWIND_STATIC_LIBS)
# public: the headers include general's (PrintfFormat.h)
target_link_libraries(nwhost PUBLIC nwgeneral PRIVATE project::warnings_options project::warning_suppressions)

add_subdirectory(tests)
