/**
 * @file ConFTD2XX.cpp
 *
 * @brief FTD2xx based FTDI communication (binary drivers from www.ftdichip.com) 
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 * this code is portable but requires binary drivers to be installed.
 * they can be downloaded from www.ftdichip.com
 */

#define FTDX_API_EXPORT
#include "ConFTD2XX.h"
#include <libgeneral.h>

using namespace general;
using namespace ftdx;

#ifndef WIN32
	#include <errno.h>  
	#include <pthread.h>
#endif

static const char * ftdiErrorString[] = {
	"FT_OK",
	"FT_INVALID_HANDLE",
	"FT_DEVICE_NOT_FOUND",
	"FT_DEVICE_NOT_OPENED",
	"FT_IO_ERROR",
	"FT_INSUFFICIENT_RESOURCES",
	"FT_INVALID_PARAMETER",
	"FT_INVALID_BAUD_RATE",
	"FT_DEVICE_NOT_OPENED_FOR_ERASE",
	"FT_DEVICE_NOT_OPENED_FOR_WRITE",
	"FT_FAILED_TO_WRITE_DEVICE",
	"FT_EEPROM_READ_FAILED",
	"FT_EEPROM_WRITE_FAILED",
	"FT_EEPROM_ERASE_FAILED",
	"FT_EEPROM_NOT_PRESENT",
	"FT_EEPROM_NOT_PROGRAMMED",
	"FT_INVALID_ARGS",
	"FT_NOT_SUPPORTED",
	"FT_OTHER_ERROR"
	}; 

ConFTD2XX::ConFTD2XX()
{
    FT_STATUS ftStatus; 
    DWORD dwLibraryVer; // Get DLL version 
    ftStatus = FT_GetLibraryVersion(&dwLibraryVer);
    if (ftStatus == FT_OK)
    {
        printf("FTD2xx library version = 0x%x\n", dwLibraryVer);
    }
    else 
    {
        printf("error: ftd2xx driver not found, download at www.ftdichip.com\n");
    }
}

ConFTD2XX::~ConFTD2XX()
{

}

bool ConFTD2XX::open()
{
	mUsbStreamOpen = false;
	//todo: make device selectable ! there could be more FTDI devices!!!
/*
    DWORD numDevs;
    FT_STATUS ftStatus = FT_ListDevices(&numDevs,NULL,FT_LIST_NUMBER_ONLY);
    if (ftStatus == FT_OK) {
        // FT_ListDevices OK, number of devices connected is in numDevs
    }
*/

    /* Just try to connect to the first available device */  
	bool connected = (FT_Open(0, &mHandle) == FT_OK);

	if (connected)
	{
		if (mTimeoutSet) {
			FT_SetTimeouts(mHandle, mRxTimeout, mTxTimeout);
		}
	
		/* set the buffer-flush-timeout */
		FT_STATUS lStatus = FT_SetLatencyTimer(mHandle, 1);		// lowest meaningfull value is 1 (2 is slower)
		if (lStatus != FT_OK) {
			Util::debug("FT_SetLatencyTimer failed!\n");
		}
	    
		//lStatus = FT_SetUSBParameters(mHandle, 4*1024, 64);
		//if (lStatus != FT_OK) {
		//    Util::debug("FT_SetUSBParameters failed!\n");
		//}

		lStatus = initFtdi(mHandle);		// create auto-reset event
		mUsbStreamOpen = true;
	}
    
    return connected;
}

void ConFTD2XX::close()
{
	if (mUsbStreamOpen)
	{
		FT_Close(mHandle);
		mUsbStreamOpen = false;
	}
}

// waits until exactly bytesToRead are available
// but uses an internal buffer when more data then requested is available
// return:
//  0: connection problem
//  otherwise aBytesToRead is returned 
int ConFTD2XX::readExact(unsigned char * aBuffer, unsigned long aBytesToRead)
{
	DWORD lbytesReturned = 0;
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
		FT_STATUS ftStatus = FT_Read(mHandle, mInternalBuffer, aBytesToRead, &lbytesReturned); 

		if (ftStatus != FT_OK) {
			Util::debug("FTD2XX error: %s\n", ftdiErrorString[ftStatus]);
			return 0;
		}

		if (lbytesReturned == 0) {
            Util::sleep(1);
			continue;
		}

		for (unsigned int i=0;i<lbytesReturned;i++)
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
//  0: connection problem 
//  otherwise: the amount of bytes in written in the buffer
int ConFTD2XX::readBlocking(unsigned char * aBuffer, unsigned long aMaxBytesToRead)
{
	DWORD lRxBytes = 0;
	for (;;)
	{
		waitForData();		
		// waitForData result doesn't matter, whether a timeout occurred or there really
		// is data, we need to check if the device is still present.

		FT_STATUS ftStatus = FT_GetQueueStatus(mHandle, &lRxBytes);
		if (ftStatus != FT_OK) return 0;
		if (lRxBytes == 0) continue;

		unsigned int lReadBytes = aMaxBytesToRead;
		if (lRxBytes < aMaxBytesToRead) lReadBytes = lRxBytes;

		DWORD lBytesReturned = 0;
		ftStatus = FT_Read(mHandle, aBuffer, lReadBytes, &lBytesReturned); 
		if (ftStatus != FT_OK) return 0;
		if (lBytesReturned != lReadBytes) return 0;
		break;
	}
	return lRxBytes;
}

void ConFTD2XX::write(unsigned char * aBuffer, unsigned long aBytesToWrite, unsigned long * aBytesWritten)
{
	*aBytesWritten = 0;

	FT_STATUS ftStatus = FT_Write(mHandle, aBuffer, aBytesToWrite, aBytesWritten); 
	if (ftStatus != FT_OK) {  
		Util::debug("FTD2XX error: %s\n", ftdiErrorString[ftStatus]);
	}
	
	if (aBytesToWrite != *aBytesWritten) {  
		Util::debug("FTD2XX [%u of %u bytes written] not all bytes written.\n", *aBytesWritten, aBytesToWrite);
	}
		
    //Util::debug("FTD2XX [%u of %u bytes written]\n", *aBytesWritten, aBytesToWrite);
}

#ifdef WIN32

HANDLE hEvent = 0;

FT_STATUS ConFTD2XX::initFtdi(FT_HANDLE ftHandle)
{
	hEvent = CreateEventA(0, false, false, "");		// create auto-reset event
	DWORD EventMask = FT_EVENT_RXCHAR | FT_EVENT_MODEM_STATUS;    // TODO: do we need FT_EVENT_MODEM_STATUS ???
	return FT_SetEventNotification(ftHandle, EventMask, hEvent);
}

int ConFTD2XX::waitForData()
{
	DWORD lResult = 0;

	// using mRxTimeout is not safe here, we need to check the device is
	// still connected every 100ms
	lResult = WaitForSingleObject(hEvent, 100);		// return every 100ms to check the device is still connected

	// consider timeouts normal when waiting for data
	if (lResult == WAIT_TIMEOUT) return 0;
	if (lResult != 0)
	{
		Util::debug("WaitForSingleObject err: %i\n", lResult);
		return -1;
	}
	return 1;		// return 1 to indicate data has arrived
}

#else

EVENT_HANDLE eh;
FT_STATUS ConFTD2XX::initFtdi(FT_HANDLE ftHandle)
{
/* Todo: create an SDL equivalent for this to be portable: */ 
	pthread_mutex_init(&eh.eMutex, NULL);
	pthread_cond_init(&eh.eCondVar, NULL);
	DWORD EventMask = FT_EVENT_RXCHAR | FT_EVENT_MODEM_STATUS;
	return FT_SetEventNotification(mHandle, EventMask, (PVOID)&eh);
}

int ConFTD2XX::waitForData()
{
	int lRet = 0;
	pthread_mutex_lock(&eh.eMutex);

	// todo: timeout is now 1 second, change it to 100ms, but how :)
	struct timespec ts;
	struct timeval  tp;
	lRet = gettimeofday(&tp, NULL);
	ts.tv_sec  = tp.tv_sec;
	ts.tv_nsec = tp.tv_usec * 1000;
	ts.tv_sec += 1;
	lRet = pthread_cond_timedwait(&eh.eCondVar, &eh.eMutex, &ts);

	pthread_mutex_unlock(&eh.eMutex);
	if (lRet == ETIMEDOUT) return 0;
	if (lRet != 0)
	{
		Util::debug("pthread_cond_timedwait err: %i\n", lRet);
	}
	return 1;
}
#endif

void ConFTD2XX::reset()
{
	FT_ResetDevice(mHandle);
}

void ConFTD2XX::purgeRx()
{
	FT_Purge(mHandle, FT_PURGE_RX);
}

void ConFTD2XX::purgeTx()
{
	FT_Purge(mHandle, FT_PURGE_TX);
}
