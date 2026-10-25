Building the nowind libraries, nwhostapp and emuv1
=================================================

The build uses CMake (3.23 or newer) and needs a C++23 compiler.
Two build trees are created, build/debug and build/release (RelWithDebInfo);
each is installed into its own install/ subdirectory, nwhostapp ends up in
build/<debug|release>/install/bin (on macOS: build/<debug|release>/install/nwhostapp.app).

The only external dependency is the FTDI USB library:
- Windows: FTDI's FTD2XX driver library, a static x64 copy is included in
  ftdx/FTD2XX-msc, nothing to install.
- Linux and macOS: libftdi1 (the open source libusb based library),
  including its headers.


Windows
-------
Prerequisites:
- Visual Studio 2022 (Community, Professional or Enterprise) with the
  "Desktop development with C++" workload, this includes CMake and Ninja.
- the FTDI D2XX driver (https://ftdichip.com/drivers/d2xx-drivers/), on most
  systems it is installed automatically by Windows Update when the nowind
  interface is plugged in.

Build, from a command prompt in this directory:
  configure-x64-ninja-vs2022-runtime.bat
  build.bat

configure-x64-ninja-vs2022-runtime.bat sets up the x64 compiler environment
(vcvarsall.bat), it expects Visual Studio to be installed on D:\Program Files,
edit the paths in the script if it is installed elsewhere.


Linux
-----
Prerequisites (Debian / Ubuntu):
  sudo apt install build-essential cmake libftdi1-dev ninja-build

Prerequisites (Fedora):
  sudo dnf install gcc-c++ cmake libftdi-devel

Note: the runtime package (libftdi1-2 on Debian) is not enough, the -dev /
-devel package provides libftdi1.so and libftdi1/ftdi.h that are needed to
build.

Optional, for emuv1 (the SDL based MSX emulator, it is skipped when SDL 1.2
or SDL_ttf is not found):
  sudo apt install libsdl1.2-dev libsdl-ttf2.0-dev

Build:
  ./build.sh

To access the nowind interface without root, the user needs permission on the
USB device, for example with a udev rule in /etc/udev/rules.d/99-nowind.rules:
  SUBSYSTEM=="usb", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", MODE="0666"
then run: sudo udevadm control --reload-rules
and re-plug the interface.


macOS
-----
Prerequisites:
- Xcode command line tools:
    xcode-select --install
- Homebrew (https://brew.sh), then:
    brew install cmake libftdi
  optional, for faster builds:
    brew install ninja

Build:
  ./build.sh


Problems
--------
- "Could not find FTDI_LIB using the following names: ftdi1" or a missing
  ftdi.h: the libftdi1 development package is not installed, see above.
- after changing dependencies or CMake files, remove build/ (or at least
  build/debug/CMakeCache.txt and build/release/CMakeCache.txt) and build again.
