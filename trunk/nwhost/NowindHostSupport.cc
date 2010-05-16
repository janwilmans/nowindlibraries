
#include "NowindHostSupport.hh"

namespace nwhost {

NowindHostSupport::NowindHostSupport()
{
}

NowindHostSupport::~NowindHostSupport()
{
}

void NowindHostSupport::debugMessage(const char *, ...) const
{
    // override this method in a subclass to log messages
}


// send:  pc -> msx
void NowindHostSupport::send(byte value)
{
	hostToMsxFifo.push_back(value);
}

void NowindHostSupport::send16(word value)
{
	hostToMsxFifo.push_back(value & 255);
	hostToMsxFifo.push_back(value >> 8);
}

void NowindHostSupport::purge()
{
	hostToMsxFifo.clear();
}

void NowindHostSupport::sendHeader()
{
	send(0xFF); // needed because first read might fail (hardware design choise)!
	send(0xAF);
	send(0x05);
}

byte NowindHostSupport::peek() const
{
	return isDataAvailable() ? hostToMsxFifo.front() : 0xFF;
}

// receive:  msx <- pc
byte NowindHostSupport::read()
{
	if (!isDataAvailable()) {
		return 0xff;
	}
	byte result = hostToMsxFifo.front();
	hostToMsxFifo.pop_front();
	return result;
}

bool NowindHostSupport::isDataAvailable() const
{
	return !hostToMsxFifo.empty();
}


} // namespace nwhost
