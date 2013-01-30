
the usbhost currently works with two libraries: 

- FTDIchip's own ftd2xx library
  This library is only available from the FTDI website in binary form.
  
  libftd2xx0.4.15.tar.gz is the 32-bit x86 compiled version
  libftd2xx0.4.15_x86_64.tar.gz	is the 64-bit x86 compiled version
  PPC D2XX0.1.4.dmg is the PowerPC version (Requires Mac OS X 10.3 (Panther) or later.)
  Universal D2XX0.1.4.dmg is the Intel version (Requires Mac OS X 10.4 (Tiger) or later.)
  
  Since these drivers are in binary form they will only work on the platform 
  they were compiled for. (for reference: the 32-bit version _can_ work on a 64 bit platform but all 
  other dependant libraries, like libSDL and libnowind must also be available on 32-bit compiled form,
  now a 64-bit version of the FTD2xx library is available it's easier to use that).

  for linux extract libftd2xx and see README.dat for installation instructions
  also in the ftd2xx.h in this directory has been extracted from libftd2xx0.4.15.tar.gz and should be overridden
  when compiling on different platforms

- an open source library called libftdi
  This can be compiled natively for any platform, but requires libUSB, so you must use a platform
  supported by libUSB. To compile libftdi, you need libusb-dev (apt-get install libusb-dev)
  extract libftdi-0.14.tar.gz, ./configure, make, make install
  note: you dont need to compile libftdi to have the ftdx library use it, it's sources are part of the ftdx project