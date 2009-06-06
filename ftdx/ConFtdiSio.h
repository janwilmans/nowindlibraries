/**
 * @file ConFtdiSio.h
 *
 * @brief ConFtdiSio header file
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef CONFTDISIO_H
#define CONFTDISIO_H

#include <string>
#include <fstream>

#include "UsbStream.h"

class ConFtdiSio : public UsbStream {
	
public:
	ConFtdiSio();
	virtual ~ConFtdiSio();
	virtual bool open();
	virtual void close();
	virtual int readExact(unsigned char * buffer, unsigned long bytesToRead);
	virtual int readBlocking(unsigned char * buffer, unsigned long maxBytesToRead);
	virtual void write(unsigned char * buffer, unsigned long bytesToWrite, unsigned long * bytesWritten);		// may block until all data is send
	virtual void reset();
        virtual void purgeRx();
        virtual void purgeTx();
	virtual const char * getIdString() { return "FTDI_SIO"; }

	int mHandle;
};

#endif
