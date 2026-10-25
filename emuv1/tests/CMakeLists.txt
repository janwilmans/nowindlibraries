PROJECT(emuv1tests)

# the unit tests use googletest, they are skipped when it is not installed
find_package(GTest)

if(NOT GTest_FOUND)
  message(STATUS "emuv1: googletest not found, the emuv1 unit tests will not be built")
  return()
endif()

ADD_EXECUTABLE(emuv1tests RamBlockTest.cpp Z80Test.cpp DisassemblerTest.cpp)
target_link_libraries(emuv1tests emuv1lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(emuv1tests PROPERTIES LABELS unit)

# a directory laid out like the repository, for the machine and the integration tests: emuv1 loads ../roms/*.ROM,
# ../msxsrc/nowind.rom (here the ROM assembled from msxsrc) and ../disks/dos2.dsk relative to its directory (emu/)
set(EMUV1_STAGE ${CMAKE_CURRENT_BINARY_DIR}/stage)
set(STAGE_ROMS MSX2.ROM MSX2EXT.ROM)
list(TRANSFORM STAGE_ROMS PREPEND ${CMAKE_SOURCE_DIR}/roms/ OUTPUT_VARIABLE STAGE_ROM_SOURCES)
add_custom_command(
    OUTPUT ${EMUV1_STAGE}/msxsrc/nowind.rom ${EMUV1_STAGE}/disks/dos2.dsk
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EMUV1_STAGE}/emu ${EMUV1_STAGE}/roms ${EMUV1_STAGE}/msxsrc ${EMUV1_STAGE}/disks
    COMMAND ${CMAKE_COMMAND} -E copy ${STAGE_ROM_SOURCES} ${EMUV1_STAGE}/roms
    COMMAND ${CMAKE_COMMAND} -E copy ${NOWIND_ROM} ${EMUV1_STAGE}/msxsrc/nowind.rom
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/disks/DOS2.dsk ${EMUV1_STAGE}/disks/dos2.dsk
    DEPENDS nowindrom ${NOWIND_ROM} ${STAGE_ROM_SOURCES} ${CMAKE_SOURCE_DIR}/disks/DOS2.dsk
    COMMENT "Staging the ROMs for the emuv1 tests"
    VERBATIM)
add_custom_target(emuv1stage DEPENDS ${EMUV1_STAGE}/msxsrc/nowind.rom)

# the configured MSX2 (singletons), in a process of its own
ADD_EXECUTABLE(emuv1machinetests MachineTest.cpp)
target_link_libraries(emuv1machinetests emuv1lib GTest::gtest_main)
target_compile_definitions(emuv1machinetests PRIVATE EMUV1_STAGE="${EMUV1_STAGE}")
add_dependencies(emuv1machinetests emuv1stage)
gtest_discover_tests(emuv1machinetests PROPERTIES LABELS unit)
