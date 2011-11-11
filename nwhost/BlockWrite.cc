
#include "BlockWrite.hh"
#include "DataBlock.hh"
#include "NowindHostSupport.hh"

#include <cassert>

#define DBERR nwhSupport->debugMessage
static const unsigned BLOCKSIZE = 240;

namespace nwhost {

// the blockWrite command transfers data from the MSX to the host (so it's 'writing' from the msx perspective)
// blocks of <sequencenr> <d0> .. <d239> <sequencenr> (total 242 bytes) are send, the sequence numbers are generated and checked by the host.

BlockWrite::BlockWrite()
{
}

void BlockWrite::initialize(NowindHostSupport* aSupport)
{
    nwhSupport = aSupport;
}

BlockWrite::~BlockWrite()
{
}

bool BlockWrite::isDone() const
{
    return done;
}

void BlockWrite::init(word aStartAddress, word aSize, std::vector<byte>* data, byte aReturnCode)
{
    //DBERR("BlockWrite::init(startAddress: 0x%04x, size: 0x%04x\n", aStartAddress, aSize);
    startAddress = aStartAddress;
    transferSize = aSize;
    bytesLeft = aSize;
    returnCode = aReturnCode;
    blockSequenceNr = 0;

    transferingToPage01 = (startAddress <= TWOBANKLIMIT);
    processedData = 0;
    done = false;
    
    buffer.clear();
    receiveBuffer = data;
    receiveBuffer->clear();
    blockWrite(startAddress, transferSize);
}

void BlockWrite::blockWrite(word startAddress, word size)
{
    //DBERR("BlockWrite::blockWrite, startAddress: 0x%04X, size: 0x%04X\n", startAddress, size);
    
    if (startAddress < TWOBANKLIMIT)
    {
        word endAddress = std::min(TWOBANKLIMIT, startAddress + size);
        word transferSize = endAddress - startAddress;
        blockWriteHelper(startAddress, transferSize);
    }
    else
    {
        blockWriteHelper(startAddress, size);
    }  
    //DBERR("blockWrite, processedData: 0x%04X\n", processedData);
}

void BlockWrite::blockWriteHelper(word startAddress, word size)
{
    //DBERR("BlockWrite::blockWriteHelper, size: 0x%02x, processedData: 0x%02x\n", size, processedData);
    
	
	transferSize = std::min(bytesLeft, BLOCKSIZE);
	unsigned endAddress = startAddress + size;

    DBERR(" address: 0x%04x, transferSize: 0x%04X \n", startAddress, size);
    
    // request the next block of data (consists of 3+1+2+2+1 = 9 bytes)
	nwhSupport->sendHeader();
	nwhSupport->send(0);          // data ahead!
	nwhSupport->send16(startAddress);
	nwhSupport->send16(size);
	nwhSupport->send(0xaa);       // use blockSequenceNr
    
    blockSequenceNr = (blockSequenceNr+1) & 255;

    //todo: write speed can be optimized by requesting multiple blocks at once, 
    // special care should taken to ensure the msx-receive buffer is does not overflow.
    // for example by making sure there are never more then 256/9 = 28 blocks out-standing.

}

void BlockWrite::receiveData(byte data)
{
    switch (receiveIndex)
    {
    case 0:
        sequenceNrHeader = data;
        break;
    case BLOCKSIZE:
        sequenceNrTail = data;
        break;
    default:
        buffer[receiveIndex] = data;
        break;
    }

    receiveIndex++;
    if (receiveIndex = 242) // block done.
    {
        if (sequenceNrHeader == receiveIndex)
        {
            // block ok
            for (int i=0;i < BLOCKSIZE;i++)
            {
                (*receiveBuffer)[i] = buffer[i+1];
            }

        }
        
    }
}

void BlockWrite::cancelWithCode(byte returnCode)
{
    nwhSupport->sendHeader();
    nwhSupport->send(returnCode);
    done = true;
}

} // namespace nwhost
