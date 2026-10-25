PROJECT(emuv1tests)

# the unit tests use googletest, they are skipped when it is not installed
find_package(GTest)

if(NOT GTest_FOUND)
  message(STATUS "emuv1: googletest not found, the emuv1 unit tests will not be built")
  return()
endif()

ADD_EXECUTABLE(emuv1tests RamBlockTest.cpp)
target_link_libraries(emuv1tests emuv1lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(emuv1tests)
