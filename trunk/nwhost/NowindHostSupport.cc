
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
	response.send(value);
}

void NowindHostSupport::send16(word value)
{
	response.send16(value);
}

void NowindHostSupport::purge()
{
	response.purge();
}

void NowindHostSupport::sendHeader()
{
	response.sendHeader();
}

byte NowindHostSupport::peek() const
{
	return response.peek();
}

// receive:  msx <- pc
byte NowindHostSupport::read()
{
	return response.read();
}

bool NowindHostSupport::isDataAvailable() const
{
	return response.isDataAvailable();
}


} // namespace nwhost
