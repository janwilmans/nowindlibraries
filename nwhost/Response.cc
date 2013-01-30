
#define NWHOST_API_EXPORT
#include "Response.hh"

namespace nwhost {

Response::Response()
{
}

Response::~Response()
{
}

// send:  pc -> msx
void Response::send(byte value)
{
	push_back(value);
}

void Response::send16(word value)
{
	push_back(value & 255);
	push_back(value >> 8);
}

void Response::purge()
{
	clear();
}

void Response::sendHeader()
{
	send(0xFF); // needed because first read might fail (hardware design choise)!
	send(0xAF);
	send(0x05);
}

byte Response::peek() const
{
	return isDataAvailable() ? front() : 0xFF;
}

// receive:  msx <- pc
byte Response::read()
{
	if (!isDataAvailable()) {
		return 0xff;
	}
	byte result = front();
	pop_front();
	return result;
}

bool Response::isDataAvailable() const
{
	return !empty();
}

} // namespace nwhost
