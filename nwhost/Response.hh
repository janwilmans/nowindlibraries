#ifndef RESPONSE_HH
#define RESPONSE_HH

#include "NowindTypes.hh"
#include <deque>

namespace nwhost {

class Response : private std::deque<byte>
{
public:
	Response();
	virtual ~Response();

    void send(byte value);
    void send16(word value);

    void purge();
    void sendHeader();
    
    byte peek() const;
    byte read();
    bool isDataAvailable() const;
};

} // namespace nwhost

#endif // RESPONSE_HH
