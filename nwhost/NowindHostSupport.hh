#ifndef NOWNDHOSTSUPPORT_HH
#define NOWNDHOSTSUPPORT_HH

#include "NowindTypes.hh"
#include <deque>

namespace nwhost {

class NowindHostSupport
{
public:
	NowindHostSupport();
	virtual ~NowindHostSupport();
	virtual void debugMessage(const char *cFormat, ...) const;

	// public for usb-host implementation
	bool isDataAvailable() const;

	// read one byte of response-data from the host (msx <- pc)
	byte read();

	// like read(), but without side effects (doesn't consume the data)
	byte peek() const;

	void send(byte value);
	void send16(word value);
	void sendHeader();
	void purge();

private:
	// queue
	std::deque<byte> hostToMsxFifo;

};

} // namespace nwhost

#endif // NOWNDHOSTSUPPORT_HH
