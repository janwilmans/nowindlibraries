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

# keep the committed msxsrc/nowind.rom equal to what the sources assemble to; runs every build, but only writes the
# file when it differs, so git shows it as modified exactly when the asm changed the ROM
add_custom_target(nowindromupdate ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NOWIND_ROM} ${CMAKE_CURRENT_SOURCE_DIR}/nowind.rom
    DEPENDS ${NOWIND_ROM}
    COMMENT "Updating msxsrc/nowind.rom"
    VERBATIM)

# NOWMAP.COM, the MSX-DOS tool that uses the ROM's EXTBIO API (built like make.bat does, from this directory)
set(NOWMAP_COM ${STAGE}/msxsrc/msxdostools/nowmap.com)
# the repository has asmlibs/String, nowmap.asm includes asmlibs/string (the staged copy is lower case)
set(NOWMAP_INPUTS msxdostools/nowmap.asm asmlibs/macros.inc asmlibs/bdos/bdos.inc asmlibs/String/string.asm)
list(TRANSFORM NOWMAP_INPUTS PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/ OUTPUT_VARIABLE NOWMAP_SOURCES)
# sjasm finds the includes relative to the source file only with the native separator in its name (\ on windows)
file(TO_NATIVE_PATH msxdostools/nowmap.asm NOWMAP_ASM)
add_custom_command(
    OUTPUT ${NOWMAP_COM}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${STAGE}/msxsrc/msxdostools ${STAGE}/msxsrc/asmlibs/bdos
            ${STAGE}/msxsrc/asmlibs/string
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/msxdostools/nowmap.asm ${STAGE}/msxsrc/msxdostools
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/asmlibs/macros.inc ${STAGE}/msxsrc/asmlibs
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/asmlibs/bdos/bdos.inc ${STAGE}/msxsrc/asmlibs/bdos
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/asmlibs/String/string.asm ${STAGE}/msxsrc/asmlibs/string
    COMMAND $<TARGET_FILE:sjasm> ${NOWMAP_ASM}
    WORKING_DIRECTORY ${STAGE}/msxsrc
    # after the ROM: that step stages the *.asm files (debug.asm, labels.asm) this includes; copying them here too
    # raced with it in parallel builds
    DEPENDS sjasm ${NOWIND_ROM} ${NOWMAP_SOURCES}
    COMMENT "Assembling NOWMAP.COM from msxsrc/msxdostools"
    VERBATIM)
add_custom_target(nowmapcom ALL DEPENDS ${NOWMAP_COM})
set(NOWMAP_COM ${NOWMAP_COM} PARENT_SCOPE)

# for the tests
set(NOWIND_ROM ${NOWIND_ROM} PARENT_SCOPE)
set(NOWIND_ROM_SYMBOLS ${NOWIND_ROM_SYMBOLS} PARENT_SCOPE)
