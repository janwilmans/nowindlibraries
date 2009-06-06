//! ConLibFtdi.h
#ifndef CONLIBFTDI_H
#define CONLIBFTDI_H

#include <string>
#include <fstream>
#include <deque>

#include "ftdi.h"
#include "UsbStream.h"

namespace ftdx {

class ConLibFtdi : public UsbStream {
	
public:
	ConLibFtdi();
	virtual ~ConLibFtdi();
	virtual bool open();
	virtual void close();
	virtual void write(unsigned char * aBuffer, unsigned long aBytesToWrite, unsigned long * aBytesWritten);		// may block until all data is send
	virtual const char * getIdString() { return "LIBFTDI"; }
	virtual void reset();
	virtual void purgeRx();
	virtual void purgeTx();
	virtual int readExact(unsigned char * aBuffer, unsigned long aBytesToRead);
	virtual int readBlocking(unsigned char * aBuffer, unsigned long maxBytesToRead);

private:
	int waitForData();
	struct ftdi_context mFtdiContext;
	unsigned char mInternalBuffer[8192];
	std::deque<unsigned char> mDeQue;
};

} // namespace ftdx

#endif
