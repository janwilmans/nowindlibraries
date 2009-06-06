/**
 * @file ConLibFtdi.cpp
 *
 * @brief libusb based FTDI communication 
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 * this code contains linux specific code, but could very user friendly 
 * because the usb serial I/O (FTDISIO) driver is available in lot of distributions by default.
 *
 */

#define FTDX_API_EXPORT
#include "ConLibFtdi.h"
#include <libgeneral.h>

// libftdi v0.14 is from http://www.intra2net.com/de/produkte/opensource/ftdi/
// this solution is portable to any platform that supports libusb 
#include "ftdi.h"			// libftdi v0.14 in /libftdi

ConLibFtdi::ConLibFtdi()
{
}

ConLibFtdi::~ConLibFtdi()
{
}

void ConLibFtdi::initialize()
{
}

bool ConLibFtdi::open()
{
    int ret;
	mUsbStreamOpen = false;
    ftdi_init(&mFtdiContext);
	if((ret = ftdi_usb_open(&mFtdiContext, 0x0403, 0x6001)) < 0) {
        //fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(&mFtdiContext));
		return false;
    }

	// set the latency timer as fast as possible (1ms)
    ftdi_set_latency_timer(&mFtdiContext, 1);

	mFtdiContext.usb_read_timeout = mRxTimeout;
	mFtdiContext.usb_write_timeout = mTxTimeout;

	//ftdi_read_data_set_chunksize(&mFtdiContext, 2048);
	//ftdi_write_data_set_chunksize(&mFtdiContext, 2048);
	mUsbStreamOpen = true;
	return true;
}

void ConLibFtdi::close()
{
	if (mUsbStreamOpen)
	{
		ftdi_usb_close(&mFtdiContext);
		ftdi_deinit(&mFtdiContext);
		mUsbStreamOpen = false;
	}
}

// waits until excactly bytesToRead are available
// but uses an internal buffer when more data then requested is available
// return:
//  0: connection problem
//  otherwise aBytesToRead is returned 
int ConLibFtdi::readExact(unsigned char * aBuffer, unsigned long aBytesToRead)
{
	int result = 0;
	for (;mDeQue.size()<aBytesToRead;)
	{
		// result:
		// <0: error or timeout
		//  0: no data available, method blocks, but can still return zero if the usb device does not 
		//     send a NAK but responds with a zero-byte reply when no data is available
		// >0: number of bytes read 
		//
		// the third parameter is the size of the buffer, not the requisted amount of data!
		// it can (and will) return with less data read also!
		//
		result = ftdi_read_data(&mFtdiContext, mInternalBuffer, 4192);
		if (result == 0) {
			Util:sleep(1);
			continue;
		}

		// stricly, a timeout here is not an error, however, the ft245 seems to respond
		// with zero-length reply packets if no data is available after it's internal LatencyTimer times-out.
		// that means a time-out should never occur and we handle it like an error if it does.
		// this is convinient because the result-code for timeout is unknown :)
		if (result < 0)
		{
			Util::debug("err: %i, %s\n", result, mFtdiContext.error_str);
			// linux: -116 during transfer (ESTALE)
			// linux: -4 when msx is turned off 
			// win32: -5 when msx is turned off (EIO) I/O error?			
			return 0;
		}
		for (int i=0;i<result;i++)
		{
			mDeQue.push_back(mInternalBuffer[i]);
		}
	}

	for (unsigned int i=0;i<aBytesToRead;i++)
	{
		aBuffer[i] = mDeQue.front();
		mDeQue.pop_front();
	}
	return aBytesToRead;
}

// waits until any data is available
// it can use libftdi's internal buffers
//  0: connection problem 
//  otherwise: the amount of bytes in written in the buffer
int ConLibFtdi::readBlocking(unsigned char * aBuffer, unsigned long aMaxBytesToRead)
{
	int result = 0;
	for (;;)
	{
		// result:
		// <0: error or timeout
		//  0: no data available, method blocks, but can still return zero if the usb device does not 
		//     send a NAK but responds with a zero-byte reply when no data is available
		// >0: number of bytes read 
		//
		// the third parameter is the size of the buffer, not the requisted amount of data!
		// it can (and will) return with less data read also!
		//
		result = ftdi_read_data(&mFtdiContext, aBuffer, aMaxBytesToRead);
		if (result == 0) {
			Util:sleep(1);
			continue;
		}

		// stricly, a timeout here is not an error, however, the ft245 seems to respond
		// with zero-length reply packets if no data is available after it's internal LatencyTimer times-out.
		// that means a time-out should never occur and we handle it like an error if it does.
		// this is convinient because the result-code for timeout is unknown :)
		if (result < 0)
		{
			Util::debug("err: %i, %s\n", result, mFtdiContext.error_str);
			return 0;
		}
		break;
	}
	return result;
}

void ConLibFtdi::write(unsigned char * buffer, unsigned long bytesToWrite, unsigned long * bytesWritten)
{
	int size = bytesToWrite;
	int result = ftdi_write_data(&mFtdiContext, buffer, size);
    if (result < 0) Util::debug("libftdi reports error: %s\n", ftdi_get_error_string(&mFtdiContext));
	*bytesWritten = result;
}

void ConLibFtdi::reset()
{
	ftdi_usb_reset(&mFtdiContext);
}

void ConLibFtdi::purgeRx()
{
	mDeQue.clear();
	ftdi_usb_purge_rx_buffer(&mFtdiContext);
}

void ConLibFtdi::purgeTx()
{
	ftdi_usb_purge_tx_buffer(&mFtdiContext);
}
