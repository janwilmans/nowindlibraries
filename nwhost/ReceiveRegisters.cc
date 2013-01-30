
#include "ReceiveRegisters.hh"
#include "DataBlock.hh"
#include "NowindHostSupport.hh"

#include <cassert>

#define DBERR nwhSupport->debugMessage

namespace nwhost {

ReceiveRegisters::ReceiveRegisters()
{
	processedData = 0;
	transferSize = 12; // fixed size
	header = 0;
	errors = 0;
	clear();
}

void ReceiveRegisters::initialize(NowindHostSupport* aSupport)
{
    nwhSupport = aSupport;
}

ReceiveRegisters::~ReceiveRegisters()
{
}

bool ReceiveRegisters::isDone() const
{
    return done;
}

void ReceiveRegisters::clear()
{
    buffer.clear();
    buffer.resize(transferSize);
}

void ReceiveRegisters::setF(byte data)
{
    if (data & 1)
    {
        DBERR("warning: Carry flag is used for timeouts and will not be send");
    }
    buffer[0] = data & 0xfe;
}

void ReceiveRegisters::setA(byte data)
{
    buffer[1] = data;
}

void ReceiveRegisters::setBC(word data)
{
    buffer[2] = data & 0xff;
    buffer[3] = data >> 8;
}

void ReceiveRegisters::setDE(word data)
{
    buffer[4] = data & 0xff;
    buffer[5] = data >> 8;
}

void ReceiveRegisters::setHL(word data)
{
    buffer[6] = data & 0xff;
    buffer[7] = data >> 8;
}

void ReceiveRegisters::setIX(word data)
{
    buffer[8] = data & 0xff;
    buffer[9] = data >> 8;
}

void ReceiveRegisters::setIY(word data)
{
    buffer[10] = data & 0xff;
    buffer[11] = data >> 8;
}

void ReceiveRegisters::send()
{
    transferSize = buffer.size();  // hardcoded to AF+BC+DE+HL = 8 bytes
    done = false;
   
    bool byteInUse[256];    // 'byte in use' map
    for (int i=0;i<256;i++)
    {
        byteInUse[i] = false;
    }
    for (unsigned int i=0;i<transferSize;i++)
    {
        byte currentByte = buffer[i];
        byteInUse[currentByte] = true;
    }      
    for (int i=0; i<256; i++)
    {
        if (byteInUse[i] == false)
        {
            // found our header (first byte not 'used' by the data)
            header = i;
            break;
        }
    }
    sendData();
}

void ReceiveRegisters::sendData()
{
    nwhSupport->sendHeader();
    nwhSupport->send(0xff);     // dummy so it is never possible to mistake for a new command
    nwhSupport->send(header);   // data header
    for (unsigned int i=0; i<transferSize; i++)
    {
        byte currentByte = buffer[i];
        nwhSupport->send(currentByte);
    }      
    nwhSupport->send(header);   // data tail
}

void ReceiveRegisters::ack(byte tail)
{
    if (header == tail)
    {		
		//DBERR("ReceiveRegisters::ack, tail matched\n");
		done = true;
    }
    else
    {
        static int errors = 0;
        errors++;
        DBERR("ReceiveRegisters::ack failed! (errors: %u, tail: 0x%02x)\n", errors, tail);
        sendData(); // resend
    }
}

} // namespace nwhost
