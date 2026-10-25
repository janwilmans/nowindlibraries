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
gtest_discover_tests(emuv1tests PROPERTIES LABELS unit ENVIRONMENT "${NOWIND_TEST_ENVIRONMENT}")

# a directory laid out like the repository, for the machine and the integration tests: emuv1 loads ../roms/*.ROM,
# ../msxsrc/nowind.rom (here the ROM assembled from msxsrc) and ../disks/dos2.dsk relative to its directory (emu/),
# and the on-screen display font from freetype/
set(EMUV1_STAGE ${CMAKE_CURRENT_BINARY_DIR}/stage)
file(GLOB STAGE_ROM_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/roms/*.ROM) # MSX2, MSX2EXT, KANJI, ...
add_custom_command(
    OUTPUT ${EMUV1_STAGE}/msxsrc/nowind.rom ${EMUV1_STAGE}/disks/dos2.dsk
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EMUV1_STAGE}/emu/freetype ${EMUV1_STAGE}/roms ${EMUV1_STAGE}/msxsrc ${EMUV1_STAGE}/disks
    COMMAND ${CMAKE_COMMAND} -E copy ${STAGE_ROM_SOURCES} ${EMUV1_STAGE}/roms
    COMMAND ${CMAKE_COMMAND} -E copy ${NOWIND_ROM} ${EMUV1_STAGE}/msxsrc/nowind.rom
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/disks/DOS2.dsk ${EMUV1_STAGE}/disks/dos2.dsk
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../copy_to_release/freetype/VeraMono.ttf
            ${EMUV1_STAGE}/emu/freetype/VeraMono.ttf
    DEPENDS nowindrom ${NOWIND_ROM} ${STAGE_ROM_SOURCES} ${CMAKE_SOURCE_DIR}/disks/DOS2.dsk
            ${CMAKE_CURRENT_SOURCE_DIR}/../copy_to_release/freetype/VeraMono.ttf
    COMMENT "Staging the ROMs for the emuv1 tests"
    VERBATIM)
add_custom_target(emuv1stage DEPENDS ${EMUV1_STAGE}/msxsrc/nowind.rom)

# the configured MSX2 (singletons), in a process of its own
ADD_EXECUTABLE(emuv1machinetests MachineTest.cpp)
target_link_libraries(emuv1machinetests emuv1lib GTest::gtest_main)
target_compile_definitions(emuv1machinetests PRIVATE EMUV1_STAGE="${EMUV1_STAGE}")
add_dependencies(emuv1machinetests emuv1stage)
gtest_discover_tests(emuv1machinetests PROPERTIES LABELS unit ENVIRONMENT "${NOWIND_TEST_ENVIRONMENT}")

# boots the emulated MSX2 with the nowind ROM against nwhost; each test needs a process of its own (ctest does that)
ADD_EXECUTABLE(emuv1integrationtests integration/BootTest.cpp)
target_link_libraries(emuv1integrationtests emuv1lib nwhosttestsupport GTest::gtest_main)
target_compile_definitions(emuv1integrationtests PRIVATE EMUV1_STAGE="${EMUV1_STAGE}" NOWIND_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
                                                         NOWMAP_COM="${NOWMAP_COM}")
add_dependencies(emuv1integrationtests emuv1stage nowmapcom)
gtest_discover_tests(emuv1integrationtests PROPERTIES LABELS integration TIMEOUT 600 ENVIRONMENT "${NOWIND_TEST_ENVIRONMENT}")
