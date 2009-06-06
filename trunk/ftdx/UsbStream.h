/**
 * @file UsbStream.h
 *
 * @brief Contains connection interface methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <fstream>

#include "FtdxExports.h"

class FTDX_API UsbStream {
	
public:
	UsbStream();
	virtual ~UsbStream();

	virtual void initialize() = 0;
	virtual bool open() = 0;
	virtual void close() = 0;
	virtual int readExact(unsigned char * buffer, unsigned long bytesToRead) = 0;
	virtual int readBlocking(unsigned char * buffer, unsigned long maxBytesToRead) = 0;
	virtual void write(unsigned char * buffer, unsigned long bytesToWrite, unsigned long * bytesWritten) = 0;		// may block until all data is send
	virtual const char * getIdString() = 0;
	virtual void setTimeouts(unsigned int rxTimeout, unsigned int txTimeout);
	virtual void reset() = 0;
	virtual void purgeRx() = 0;
	virtual void purgeTx() = 0;

	void openBlocking();

protected:
	unsigned int mRxTimeout;
	unsigned int mTxTimeout;
	bool mTimeoutSet;
	bool mUsbStreamOpen;
};

#endif
