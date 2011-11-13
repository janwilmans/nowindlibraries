
#include "BlockWrite.hh"
#include "DataBlock.hh"
#include "NowindHostSupport.hh"

#include <cassert>

#define DBERR nwhSupport->debugMessage

// the BLOCKWRITE_SIZE is not hardcoded in ROM, the host requests the exact amount the msx should send.
static const unsigned BLOCKWRITE_SIZE = 240;    

namespace nwhost {

// the blockWrite command transfers data from the MSX to the host (so it's 'writing' from the msx perspective)
// blocks of <sequencenr> <d0> .. <d239> <sequencenr> (total 242 bytes) are send, the sequence numbers are generated and checked by the host.

BlockWrite::BlockWrite()
{
}

void BlockWrite::copyData(std::vector<byte>& destinationData)
{
    for (unsigned int i=0; i<buffer.size();i++)
    {
        destinationData[i] = buffer[i];
    }
    buffer.clear();
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
    defaultReturnCode = aReturnCode;
    blockSequenceNr = 0;
    receiveIndex = 0;

    SequenceNrQueue.clear();
    buffer.clear();
    buffer.resize(aSize);
    transferingToPage01 = (startAddress <= TWOBANKLIMIT);
    processedData = 0;
    done = false;
    
    receiveBuffer = data;
    receiveBuffer->clear();
    continueWithNextBlock();
}

// todo:
// - add all blocks to receive from msx to a queue
// - send several 9-byte requests for data 
// - receive blocks of 240+2 bytes, check the markers, request again

void BlockWrite::continueWithNextBlock()
{
    if (bytesLeft == 0)
    {
        //DBERR("BlockWrite::continueWithNextBlock, DONE!\n");

        nwhSupport->sendHeader();
        nwhSupport->send(defaultReturnCode);
        done = true;
        return;
    }

    //DBERR("BlockWrite::continueWithNextBlock, startAddress: 0x%04X, size: 0x%04X\n", startAddress, bytesLeft);

    unsigned int startAddr = startAddress + processedData;

    // make sure the transfer does not cross the TWOBANKLIMIT
    if (startAddr < TWOBANKLIMIT)
    {
        word endAddress = std::min(TWOBANKLIMIT, startAddr + bytesLeft);
        word transferSize = endAddress - startAddr;
        requestBlock(startAddr, transferSize);
    }
    else
    {
        requestBlock(startAddr, bytesLeft);
    }  
}

void BlockWrite::requestBlock(word startAddress, word size)
{
	currentBlockSize = std::min(size, BLOCKWRITE_SIZE);
	unsigned int endAddress = startAddress + size;

    //DBERR("BlockWrite::requestBlock, address: 0x%04x, transferSize: 0x%04X \n", startAddress, currentBlockSize);
    
    // request the next block of data (consists of 3+1+2+2+1 = 9 bytes)
	nwhSupport->sendHeader();
	nwhSupport->send(0);                // transfer not done
	nwhSupport->send16(startAddress);
	nwhSupport->send16(currentBlockSize);
	nwhSupport->send(blockSequenceNr);
    
    //DBERR("BlockWrite: SequenceNrQueue: %u\n", blockSequenceNr);
    SequenceNrQueue.push_back(blockSequenceNr);
    blockSequenceNr = (blockSequenceNr+1) & 255;

    //todo: write speed can be optimized by requesting multiple blocks at once, 
    // special care should taken to ensure the msx-receive buffer is does not overflow.
    // for example by making sure there are never more then 256/9 = 28 blocks out-standing.
}

void BlockWrite::receiveData(byte data)
{
    //DBERR("BlockWrite::receiveData, receiveIndex: %u, currentBlockSize: %u\n", receiveIndex, currentBlockSize);

    if (receiveIndex == 0)
    {
        sequenceNrHeader = data;
        receiveIndex++;
        if (SequenceNrQueue.front() != sequenceNrHeader)
        {
            // sequenceNrHeader if not the sequenceNr
            // todo: maybe implement a retry mechanism
            // abort entire transfer, for now
            DBERR("BlockWrite::receiveData, sequenceNrHeader: 0x%02X != 0x%02X\n", sequenceNrHeader, SequenceNrQueue.front());
            cancelWithCode(BLOCKWRITE_ERROR);
        }
        else
        {
            //DBERR("BlockWrite::receiveData, BLOCKSTART [0x%02X]\n", data);
        }
    } 
    else if (receiveIndex <= currentBlockSize)
    {
        //DBERR("BlockWrite::receiveData, %u: 0x%02X, bytesLeft: %u\n", processedData, data, bytesLeft);
        buffer[processedData] = data;
        receiveIndex++;
        processedData++;
        bytesLeft--;
    }
    else
    {
        sequenceNrTail = data;
        if (sequenceNrHeader == sequenceNrTail)
        {
            //DBERR("BlockWrite::receiveData, BLOCKEND [0x%02X] block OK\n", data);
            SequenceNrQueue.pop_front();
            continueWithNextBlock();
            receiveIndex = 0;
        }
        else
        {
            DBERR("BlockWrite::receiveData, sequenceNrTail: 0x%02X != 0x%02X\n", sequenceNrTail, sequenceNrHeader);

            DBERR("BlockWrite::receiveData, block ERROR!\n");
            cancelWithCode(BLOCKWRITE_ERROR);
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
