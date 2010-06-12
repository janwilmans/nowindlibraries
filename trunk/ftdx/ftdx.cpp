/**
 * @file ftdx.cpp
 *
 * @brief Contains static ftdx methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */

#define FTDX_API_EXPORT
#include "ftdx.h"
#include "libgeneral.h"

#ifdef WIN32
    #include "ConFTD2XX.h"
#else
    #include "ConFtdiSio.h"

    // the libftdi works on libusb-win32 aswell, _but_ libusb relies on the
    // windows usbser.dll driver which performs very poorly 
    // and because we do not want to rely on libusb0.dll if we are never going to use it
    // it is excluded from the windows build
    #include "ConLibFtdi.h"
#endif

using namespace general;

void ftdx::initialize()
{
    general::initialize();
}

ftdx::UsbStream* ftdx::newUsbStream()
{
    UsbStream* lUsbStream = 0;
#ifdef WIN32
    // FTD2XX driver works in kernel-space and has much less retries at 7 MHz
    Util::debug("Using FTD2XX\n");
    lUsbStream = new ConFTD2XX();
#else
    // on linux we favor libusb to minimize dependencies on propriatary binaries
    Util::debug("Using LIBUSB + LIBFTDI\n");
    lUsbStream = new ConLibFtdi();
    
    // LIBUSB driver is a user-space driver, maybe slower in some cases?
    // this is strange because this shouldnt have any effect on the polling rate
    // (which is an important factor in retries)
#endif
    return lUsbStream;
}

ftdx::UsbStream* ftdx::newUsbStream(FtdiDriverType aDriverType)
{
	UsbStream* lUsbStream = 0;
	switch (aDriverType)
	{
#ifdef WIN32
	case eDRIVER_FTD2XX:
		lUsbStream = new ConFTD2XX();
		break;
#else
	case eDRIVER_LibUsb:
		lUsbStream = new ConLibFtdi();
		break;
	case eDRIVER_FtdiSio:
		lUsbStream = new ConFtdiSio();
		break;
#endif

    // todo: support for FTDI's FTSER2K.SYS (or any other virtual serial port driver) on windows (COMx:) and linux /dev/TTYSx?

	default:
		Util::debug("unknown driver type %u specified, null returned\n", aDriverType);
		break;	
	}
	return lUsbStream;	
}
