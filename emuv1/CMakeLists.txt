# emuv1 is the SDL 1.2 based MSX emulator with an emulated nowind interface, it needs SDL 1.2 and SDL_ttf 2.0
# (cmake/Dependencies.cmake: downloaded on Windows, the installed packages elsewhere)
if(NOT TARGET SDL::SDL OR NOT TARGET SDL::ttf)
  # cmake/Dependencies.cmake says how to install it
  return()
endif()

# everything but main() goes in a library, so the unit tests can link the emulator too
add_library(emuv1lib STATIC
    Config.cpp Debug.cpp DiskDrive.cpp DiskInterface.cpp Emulator.cpp EmulatorTester.cpp GUI.cpp
    Media.cpp VirtualDrive.cpp
    audio/AY38910.cpp audio/AudioDevice.cpp audio/AudioDummy.cpp audio/AudioMixer.cpp audio/MSXAudio.cpp
    audio/PPISound.cpp audio/SCC.cpp audio/YM2413.cpp
    controls/Joystick.cpp controls/Keyboard.cpp controls/Mouse.cpp
    cpu/Disassembler.cpp cpu/Profiler.cpp cpu/Z80.cpp
    devices/DebugDevice.cpp devices/F4Device.cpp devices/I8255.cpp devices/Kanji.cpp devices/RP5C01.cpp
    devices/SlotSelector.cpp devices/SwitchedPorts.cpp
    memory/EmptyPage.cpp memory/Mapper.cpp memory/MapperFMPAC.cpp memory/MapperKonami4.cpp
    memory/MapperKonami5.cpp memory/MapperMsxDos2.cpp memory/MemoryDevice.cpp memory/NowindInterface.cpp
    memory/RamBlock.cpp memory/RomBlock.cpp memory/WD279X.cpp
    osd/Command.cpp osd/Debugger.cpp osd/LogWatcher.cpp osd/MemoryMonitor.cpp osd/OnScreenDisplay.cpp
    video/V9938.cpp video/V9938commands.cpp video/V9938renderer.cpp video/V9938sprites.cpp)

# the sources include headers relative to this directory (cpu/Z80.h)
target_include_directories(emuv1lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(WIN32)
  target_compile_definitions(emuv1lib PUBLIC _CRT_SECURE_NO_DEPRECATE)
endif()
target_link_libraries(emuv1lib PUBLIC nwhost SDL::SDL SDL::ttf PRIVATE project::warnings_options)

add_executable(emuv1 maincons.cpp)
target_link_libraries(emuv1 PRIVATE emuv1lib project::warnings_options)
nowind_copy_runtime_dlls(emuv1)
if(WIN32)
  # the real main(), instead of SDL's SDLmain.lib (see the file)
  target_sources(emuv1 PRIVATE SdlMainWindows.cpp)
  # msvcrt has no getopt, use the copy bundled with nwhostapp
  target_link_libraries(emuv1 PRIVATE getopt)
endif()

install(TARGETS emuv1 RUNTIME DESTINATION bin COMPONENT Runtime)
if(NOWIND_RUNTIME_DLLS)
  install(FILES ${NOWIND_RUNTIME_DLLS} DESTINATION bin COMPONENT Runtime)
endif()

add_subdirectory(tests)
