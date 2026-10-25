# the unit tests use googletest, they are skipped when it is not installed
find_package(GTest)

if(NOT GTest_FOUND)
  message(STATUS "nwhost: googletest not found, the nwhost unit tests will not be built")
  return()
endif()

include(GoogleTest)

# helpers shared with the emuv1 tests (support/*.h)
add_library(nwhosttestsupport INTERFACE)
target_include_directories(nwhosttestsupport INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nwhosttestsupport INTERFACE nwhost GTest::gtest)

add_executable(nwhosttests
    BDOSProxyTest.cpp BlockReadTest.cpp BlockWriteTest.cpp DataBlockTest.cpp DeviceTest.cpp ImageTest.cpp
    NowindHostProtocolTest.cpp NowindUsbApiTest.cpp ResponseTest.cpp SupportTest.cpp)
target_link_libraries(nwhosttests PRIVATE nwhosttestsupport GTest::gtest_main project::warnings_options)
gtest_discover_tests(nwhosttests PROPERTIES LABELS unit)

# checks that the host and the ROM sources (msxsrc) and the ROM assembled from them agree
add_executable(nwhostsynctests ProtocolSyncTest.cpp)
target_link_libraries(nwhostsynctests PRIVATE nwhosttestsupport GTest::gtest_main project::warnings_options)
target_compile_definitions(nwhostsynctests PRIVATE NOWIND_SOURCE_DIR="${CMAKE_SOURCE_DIR}" NOWIND_ROM="${NOWIND_ROM}"
                                                   NOWIND_ROM_SYMBOLS="${NOWIND_ROM_SYMBOLS}")
add_dependencies(nwhostsynctests nowindrom)
gtest_discover_tests(nwhostsynctests PROPERTIES LABELS sync)
