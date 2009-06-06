/**
 * @file ConFtdiSio.cpp
 *
 * @brief Linux kernel specific FTDI communication 
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 * this code contains linux specific code, but could very user friendly 
 * because the usb serial I/O (FTDISIO) driver is available in lot of distributions by default.
 *
 */

#include "ConFtdiSio.h"
#include <fcntl.h> 	// for open() and O_RDRW

// POSIX portable methods to control tty devices
#include <termios.h>

ConFtdiSio::ConFtdiSio()
{

}

ConFtdiSio::~ConFtdiSio()
{

}

void ConFtdiSio::initialize()
{

}

bool ConFtdiSio::open()
{
   struct termios lTermios;

	mHandle = ::open("/dev/ttyUSB0", O_RDWR);  //O_NONBLOCK

    // don't respond to escape characters and what not
    // system("stty -F /dev/ttyUSB0 raw");
    ::tcgetattr(mHandle, &lTermios);
    ::cfmakeraw(&lTermios);
    ::tcsetattr(mHandle, TCSANOW, &lTermios);

	return (mHandle != -1);
}

void ConFtdiSio::close()
{
	::close(mHandle);
}

int ConFtdiSio::readExact(unsigned char * aBuffer, unsigned long aBytesToRead)
{
	int ret = ::read(mHandle, aBuffer, aBytesToRead);
	if (ret == -1) return 0;
	return ret;
}

int ConFtdiSio::readBlocking(unsigned char * aBuffer, unsigned long aMaxBytesToRead)
{
	int ret = ::read(mHandle, aBuffer, aMaxBytesToRead);
	if (ret == -1) return 0;
	return ret;
}

void ConFtdiSio::write(unsigned char * aBuffer, unsigned long aBtesToWrite, unsigned long *aBytesWritten)
{
	int ret = ::write(mHandle, aBuffer, aBtesToWrite);
	if (ret == -1) *aBytesWritten = 0;
	else *aBytesWritten = ret;
}

void ConFtdiSio::reset()
{
    ::tcflush(mHandle, TCIOFLUSH);
}

void ConFtdiSio::purgeRx()
{
    // TCIFLUSH flushes data received but not read
    ::tcflush(mHandle, TCIFLUSH);
}

void ConFtdiSio::purgeTx()
{
    // TCOFLUSH flushes data written but not transmitted
    ::tcflush(mHandle, TCOFLUSH);
}
