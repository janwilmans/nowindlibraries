UPPDIR1 = ../

UPPOUT = _out/
CINC =  -I$(UPPDIR1) -I/usr/include/freetype2 -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/gtk-2.0/include -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/atk-1.0
Macro =  -DflagCONSOLE -DflagGCC -DflagDEBUG -DflagSHARED -DflagDEBUG_FULL -DflagLINUX
CC = c++ -c -D_DEBUG -O0 -ggdb -g2 -I../general -I../nowind -I../ftdx -I../nwhost
CFLAGS = $(CC) -x c
CPPFLAGS = $(CC) -x c++ 
LIBPATH =  -L"/usr/X11R6/lib"
AR = ar -sr
OutDir_nwhostapp = $(UPPOUT)nwhostapp/GCC-Console-Debug-Debug_full-Gcc-Linux-Main-Shared/
Macro_nwhostapp = $(Macro) -DflagMAIN
OutDir_nowind = $(UPPOUT)nowind/GCC-Console-Debug-Debug_full-Gcc-Linux-Shared/
Macro_nowind = $(Macro)
OutDir_general = $(UPPOUT)general/GCC-Console-Debug-Debug_full-Gcc-Linux-Shared/
Macro_general = $(Macro)
OutDir_ftdx = $(UPPOUT)ftdx/GCC-Console-Debug-Debug_full-Gcc-Linux-Shared/
Macro_ftdx = $(Macro)
OutDir_nwhost = $(UPPOUT)nwhost/GCC-Console-Debug-Debug_full-Gcc-Linux-Shared/
Macro_nwhost = $(Macro)

OutDir = $(OutDir_nwhostapp)
OutFile = nwhostapp

.PHONY: all
all: install $(OutFile)

.PHONY: install
install:
	-mkdir -p $(OutDir)
	-mkdir -p $(OutDir_nwhostapp)
	-mkdir -p $(OutDir_nowind)
	-mkdir -p $(OutDir_general)
	-mkdir -p $(OutDir_ftdx)
	-mkdir -p $(OutDir_nwhost)

$(OutFile):  \
	$(OutDir_nwhostapp)HostApp.o \
	$(OutDir_nwhostapp)mainUsbHost.o \
	$(OutDir_nowind)nowind.a \
	$(OutDir_general)general.a \
	$(OutDir_ftdx)ftdx.a \
	$(OutDir_nwhost)nwhost.a
	c++ -o $(OutFile) -ggdb $(LIBPATH) -Wl, $(LINKOPTIONS) \
		$(OutDir_nwhostapp)HostApp.o \
		$(OutDir_nwhostapp)mainUsbHost.o \
			$(OutDir_nowind)nowind.a \
			$(OutDir_general)general.a \
			$(OutDir_ftdx)ftdx.a \
			$(OutDir_nwhost)nwhost.a \
			-lftd2xx \


$(OutDir_nwhostapp)HostApp.o: $(UPPDIR1)nwhostapp/HostApp.cpp \
	$(UPPDIR1)nwhostapp/HostApp.h
	$(CPPFLAGS) $(CINC) $(Macro_nwhostapp)  $(UPPDIR1)nwhostapp/HostApp.cpp -o $(OutDir_nwhostapp)HostApp.o

$(OutDir_nwhostapp)mainUsbHost.o: $(UPPDIR1)nwhostapp/mainUsbHost.cpp \
	$(UPPDIR1)nwhostapp/HostApp.h
	$(CPPFLAGS) $(CINC) $(Macro_nwhostapp)  $(UPPDIR1)nwhostapp/mainUsbHost.cpp -o $(OutDir_nwhostapp)mainUsbHost.o

$(OutDir_nowind)nowind.o: $(UPPDIR1)nowind/nowind.cpp \
	$(UPPDIR1)nowind/nowind.h \
	$(UPPDIR1)nowind/NowindExports.h \
	$(UPPDIR1)nowind/NwhostService.h
	$(CPPFLAGS) $(CINC) $(Macro_nowind)  $(UPPDIR1)nowind/nowind.cpp -o $(OutDir_nowind)nowind.o

$(OutDir_nowind)NwhostService.o: $(UPPDIR1)nowind/NwhostService.cpp
	$(CPPFLAGS) $(CINC) $(Macro_nowind)  $(UPPDIR1)nowind/NwhostService.cpp -o $(OutDir_nowind)NwhostService.o

$(OutDir_nowind)Attribute.o: $(UPPDIR1)nowind/Attribute.cpp
	$(CPPFLAGS) $(CINC) $(Macro_nowind)  $(UPPDIR1)nowind/Attribute.cpp -o $(OutDir_nowind)Attribute.o

$(OutDir_nowind)nowind.a: \
	$(OutDir_nowind)nowind.o \
	$(OutDir_nowind)NwhostService.o \
	$(OutDir_nowind)Attribute.o
	$(AR) $(OutDir_nowind)nowind.a \
		$(OutDir_nowind)nowind.o \
		$(OutDir_nowind)NwhostService.o \
		$(OutDir_nowind)Attribute.o

$(OutDir_general)general.o: $(UPPDIR1)general/general.cpp \
	$(UPPDIR1)general/GeneralExports.h \
	$(UPPDIR1)general/Util.h
	$(CPPFLAGS) $(CINC) $(Macro_general)  $(UPPDIR1)general/general.cpp -o $(OutDir_general)general.o

$(OutDir_general)Util.o: $(UPPDIR1)general/Util.cpp
	$(CPPFLAGS) $(CINC) $(Macro_general)  $(UPPDIR1)general/Util.cpp -o $(OutDir_general)Util.o

$(OutDir_general)general.a: \
	$(OutDir_general)general.o \
	$(OutDir_general)Util.o
	$(AR) $(OutDir_general)general.a \
		$(OutDir_general)general.o \
		$(OutDir_general)Util.o

$(OutDir_ftdx)ConFTD2XX.o: $(UPPDIR1)ftdx/ConFTD2XX.cpp
	$(CPPFLAGS) $(CINC) $(Macro_ftdx)  $(UPPDIR1)ftdx/ConFTD2XX.cpp -o $(OutDir_ftdx)ConFTD2XX.o

$(OutDir_ftdx)ConFtdiSio.o: $(UPPDIR1)ftdx/ConFtdiSio.cpp \
	$(UPPDIR1)ftdx/ConFtdiSio.h \
	$(UPPDIR1)ftdx/ftdx.h \
	$(UPPDIR1)ftdx/FtdxExports.h \
	$(UPPDIR1)ftdx/UsbStream.h
	$(CPPFLAGS) $(CINC) $(Macro_ftdx)  $(UPPDIR1)ftdx/ConFtdiSio.cpp -o $(OutDir_ftdx)ConFtdiSio.o

$(OutDir_ftdx)ConLibFtdi.o: $(UPPDIR1)ftdx/ConLibFtdi.cpp \
	$(UPPDIR1)ftdx/ftdi.h
	$(CPPFLAGS) $(CINC) $(Macro_ftdx)  $(UPPDIR1)ftdx/ConLibFtdi.cpp -o $(OutDir_ftdx)ConLibFtdi.o

$(OutDir_ftdx)ftdi.o: $(UPPDIR1)ftdx/ftdi.c \
	$(UPPDIR1)ftdx/ftdi.h
	$(CFLAGS) $(CINC) $(Macro_ftdx)  $(UPPDIR1)ftdx/ftdi.c -o $(OutDir_ftdx)ftdi.o

$(OutDir_ftdx)ftdx.o: $(UPPDIR1)ftdx/ftdx.cpp \
	$(UPPDIR1)ftdx/ConFTD2XX.h \
	$(UPPDIR1)ftdx/ConFtdiSio.h \
	$(UPPDIR1)ftdx/ConLibFtdi.h \
	$(UPPDIR1)ftdx/ftdi.h \
	$(UPPDIR1)ftdx/ftdx.h \
	$(UPPDIR1)ftdx/FtdxExports.h \
	$(UPPDIR1)ftdx/other/ftd2xx.h \
	$(UPPDIR1)ftdx/other/WinTypes.h \
	$(UPPDIR1)ftdx/UsbStream.h
	$(CPPFLAGS) $(CINC) $(Macro_ftdx)  $(UPPDIR1)ftdx/ftdx.cpp -o $(OutDir_ftdx)ftdx.o

$(OutDir_ftdx)UsbStream.o: $(UPPDIR1)ftdx/UsbStream.cpp
	$(CPPFLAGS) $(CINC) $(Macro_ftdx)  $(UPPDIR1)ftdx/UsbStream.cpp -o $(OutDir_ftdx)UsbStream.o

$(OutDir_ftdx)ftdx.a: \
	$(OutDir_ftdx)ConFTD2XX.o \
	$(OutDir_ftdx)ConFtdiSio.o \
	$(OutDir_ftdx)ConLibFtdi.o \
	$(OutDir_ftdx)ftdi.o \
	$(OutDir_ftdx)ftdx.o \
	$(OutDir_ftdx)UsbStream.o
	$(AR) $(OutDir_ftdx)ftdx.a \
		$(OutDir_ftdx)ConFTD2XX.o \
		$(OutDir_ftdx)ConFtdiSio.o \
		$(OutDir_ftdx)ConLibFtdi.o \
		$(OutDir_ftdx)ftdi.o \
		$(OutDir_ftdx)ftdx.o \
		$(OutDir_ftdx)UsbStream.o

$(OutDir_nwhost)Image.o: $(UPPDIR1)nwhost/Image.cpp \
	$(UPPDIR1)nwhost/DiskHandler.hh \
	$(UPPDIR1)nwhost/Image.h \
	$(UPPDIR1)nwhost/NwhostInternal.h \
	$(UPPDIR1)nwhost/PartitionInfo.h \
	$(UPPDIR1)nwhost/SectorMedium.hh
	$(CPPFLAGS) $(CINC) $(Macro_nwhost)  $(UPPDIR1)nwhost/Image.cpp -o $(OutDir_nwhost)Image.o

$(OutDir_nwhost)ImageHandler.o: $(UPPDIR1)nwhost/ImageHandler.cpp \
	$(UPPDIR1)nwhost/DiskHandler.hh \
	$(UPPDIR1)nwhost/Image.h \
	$(UPPDIR1)nwhost/ImageHandler.h \
	$(UPPDIR1)nwhost/PartitionInfo.h \
	$(UPPDIR1)nwhost/SectorMedium.hh
	$(CPPFLAGS) $(CINC) $(Macro_nwhost)  $(UPPDIR1)nwhost/ImageHandler.cpp -o $(OutDir_nwhost)ImageHandler.o

$(OutDir_nwhost)NowindHostDebug.o: $(UPPDIR1)nwhost/NowindHostDebug.cpp \
	$(UPPDIR1)nwhost/NowindHost.hh \
	$(UPPDIR1)nwhost/NowindHostDebug.h \
	$(UPPDIR1)nwhost/NowindTypes.hh \
	$(UPPDIR1)nwhost/NwhostInternal.h
	$(CPPFLAGS) $(CINC) $(Macro_nwhost)  $(UPPDIR1)nwhost/NowindHostDebug.cpp -o $(OutDir_nwhost)NowindHostDebug.o

$(OutDir_nwhost)nwhost.o: $(UPPDIR1)nwhost/nwhost.cpp \
	$(UPPDIR1)nwhost/NowindTypes.hh \
	$(UPPDIR1)nwhost/nowindusb.h \
	$(UPPDIR1)nwhost/NwhostExports.h
	$(CPPFLAGS) $(CINC) $(Macro_nwhost)  $(UPPDIR1)nwhost/nwhost.cpp -o $(OutDir_nwhost)nwhost.o

$(OutDir_nwhost)nowindusb.o: $(UPPDIR1)nwhost/nowindusb.cpp \
	$(UPPDIR1)nwhost/DiskHandler.hh \
	$(UPPDIR1)nwhost/Image.h \
	$(UPPDIR1)nwhost/ImageHandler.h \
	$(UPPDIR1)nwhost/NowindHost.hh \
	$(UPPDIR1)nwhost/NowindHostDebug.h \
	$(UPPDIR1)nwhost/NowindTypes.hh \
	$(UPPDIR1)nwhost/NwhostInternal.h \
	$(UPPDIR1)nwhost/PartitionInfo.h \
	$(UPPDIR1)nwhost/SectorMedium.hh
	$(CPPFLAGS) $(CINC) $(Macro_nwhost)  $(UPPDIR1)nwhost/nowindusb.cpp -o $(OutDir_nwhost)nowindusb.o

$(OutDir_nwhost)PartitionInfo.o: $(UPPDIR1)nwhost/PartitionInfo.cpp \
	$(UPPDIR1)nwhost/PartitionInfo.h
	$(CPPFLAGS) $(CINC) $(Macro_nwhost)  $(UPPDIR1)nwhost/PartitionInfo.cpp -o $(OutDir_nwhost)PartitionInfo.o

$(OutDir_nwhost)NowindHost.o: $(UPPDIR1)nwhost/NowindHost.cc \
	$(UPPDIR1)nwhost/DiskHandler.hh \
	$(UPPDIR1)nwhost/NowindHost.hh \
	$(UPPDIR1)nwhost/NowindTypes.hh \
	$(UPPDIR1)nwhost/SectorMedium.hh
	$(CPPFLAGS) $(CINC) $(Macro_nwhost)  $(UPPDIR1)nwhost/NowindHost.cc -o $(OutDir_nwhost)NowindHost.o

$(OutDir_nwhost)nwhost.a: \
	$(OutDir_nwhost)Image.o \
	$(OutDir_nwhost)ImageHandler.o \
	$(OutDir_nwhost)NowindHostDebug.o \
	$(OutDir_nwhost)nwhost.o \
	$(OutDir_nwhost)nowindusb.o \
	$(OutDir_nwhost)PartitionInfo.o \
	$(OutDir_nwhost)NowindHost.o
	$(AR) $(OutDir_nwhost)nwhost.a \
		$(OutDir_nwhost)Image.o \
		$(OutDir_nwhost)ImageHandler.o \
		$(OutDir_nwhost)NowindHostDebug.o \
		$(OutDir_nwhost)nwhost.o \
		$(OutDir_nwhost)nowindusb.o \
		$(OutDir_nwhost)PartitionInfo.o \
		$(OutDir_nwhost)NowindHost.o

