PROJECT(nwhosttests)

# the unit tests use googletest, they are skipped when it is not installed
find_package(GTest)

if(NOT GTest_FOUND)
  message(STATUS "nwhost: googletest not found, the nwhost unit tests will not be built")
  return()
endif()

include(GoogleTest)

# helpers shared with the emuv1 tests (support/*.h)
add_library(nwhosttestsupport INTERFACE)
target_include_directories(nwhosttestsupport INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..
                                                       ${CMAKE_CURRENT_SOURCE_DIR}/../../general)
target_link_libraries(nwhosttestsupport INTERFACE nwhost nwgeneral GTest::gtest)

ADD_EXECUTABLE(nwhosttests SupportTest.cpp ResponseTest.cpp DataBlockTest.cpp BlockReadTest.cpp BlockWriteTest.cpp NowindHostProtocolTest.cpp
    DeviceTest.cpp ImageTest.cpp NowindUsbApiTest.cpp BDOSProxyTest.cpp)
target_link_libraries(nwhosttests nwhosttestsupport GTest::gtest_main)
gtest_discover_tests(nwhosttests PROPERTIES LABELS unit)

# checks that the host and the ROM sources (msxsrc) and the ROM assembled from them agree
ADD_EXECUTABLE(nwhostsynctests ProtocolSyncTest.cpp)
target_link_libraries(nwhostsynctests nwhosttestsupport GTest::gtest_main)
target_compile_definitions(nwhostsynctests PRIVATE NOWIND_SOURCE_DIR="${CMAKE_SOURCE_DIR}" NOWIND_ROM="${NOWIND_ROM}"
                                                   NOWIND_ROM_SYMBOLS="${NOWIND_ROM_SYMBOLS}")
add_dependencies(nwhostsynctests nowindrom)
gtest_discover_tests(nwhostsynctests PROPERTIES LABELS sync)
