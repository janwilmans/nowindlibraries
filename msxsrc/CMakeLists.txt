PROJECT(nowindrom)

# assemble the nowind ROM from these sources, the tests use it to check the host against the ROM.
# nowind.asm includes ..\roms\*.ROM and ..\disks\dos2.dsk, so the sources are staged next to copies of those files
# (sjasm turns the backslashes into slashes, the disk image is DOS2.dsk in the repo, which matters on linux)
set(STAGE ${CMAKE_CURRENT_BINARY_DIR}/stage)
set(NOWIND_ROM ${STAGE}/msxsrc/nowind.rom)
set(NOWIND_ROM_SYMBOLS ${STAGE}/msxsrc/nowind.sym)

file(GLOB ASM_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.asm)
set(ROM_INPUTS ${CMAKE_SOURCE_DIR}/roms/MSXDOS22.ROM ${CMAKE_SOURCE_DIR}/roms/DISK.ROM ${CMAKE_SOURCE_DIR}/disks/DOS2.dsk)

add_custom_command(
    OUTPUT ${NOWIND_ROM} ${NOWIND_ROM_SYMBOLS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${STAGE}/msxsrc ${STAGE}/roms ${STAGE}/disks
    COMMAND ${CMAKE_COMMAND} -E copy ${ASM_SOURCES} ${STAGE}/msxsrc
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/roms/MSXDOS22.ROM ${CMAKE_SOURCE_DIR}/roms/DISK.ROM ${STAGE}/roms
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/disks/DOS2.dsk ${STAGE}/disks/dos2.dsk
    COMMAND $<TARGET_FILE:sjasm> -s nowind.asm nowind.rom
    WORKING_DIRECTORY ${STAGE}/msxsrc
    DEPENDS sjasm ${ASM_SOURCES} ${ROM_INPUTS}
    COMMENT "Assembling the nowind ROM from msxsrc"
    VERBATIM)
add_custom_target(nowindrom ALL DEPENDS ${NOWIND_ROM})

# for the tests
set(NOWIND_ROM ${NOWIND_ROM} PARENT_SCOPE)
set(NOWIND_ROM_SYMBOLS ${NOWIND_ROM_SYMBOLS} PARENT_SCOPE)
