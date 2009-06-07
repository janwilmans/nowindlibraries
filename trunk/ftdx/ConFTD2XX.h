//! ConFTD2XX.h
#ifndef CONFTD2XX_H
#define CONFTD2XX_H

#include <string>
#include <fstream>
#include <deque>

#ifdef WIN32
    #include <windows.h>
    #include "FTD2XX-msc/FTD2XX.H"
#else
	#include "other/ftd2xx.h"
#endif

#include "UsbStream.h"

namespace ftdx {

class ConFTD2XX : public UsbStream {
	
public:
	ConFTD2XX();
	virtual ~ConFTD2XX();
	virtual bool open();
	virtual void close();
	virtual int readExact(unsigned char * buffer, unsigned long bytesToRead);
	virtual int readBlocking(unsigned char * buffer, unsigned long maxBytesToRead);
	virtual void write(unsigned char * buffer, unsigned long bytesToWrite, unsigned long * bytesWritten);		// may block until all data is send
	virtual const char * getIdString() { return "FTD2XX"; }
	virtual void reset();
	virtual void purgeRx();
	virtual void purgeTx();

	int waitForData();
private:
	FT_HANDLE mHandle;
	FT_STATUS mStatus;
	FT_STATUS initFtdi(FT_HANDLE ftHandle);
	unsigned char mInternalBuffer[8192];
	std::deque<unsigned char> mDeQue;
};

} // namespace ftdx
#endif
