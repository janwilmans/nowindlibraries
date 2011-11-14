
#include "BlockWrite.hh"
#include "DataBlockWrite.hh"
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

void BlockWrite::copyData(std::vector<byte>& aDestinationData)
{
    for (unsigned int i=0; i< mBuffer.size();i++)
    {
        aDestinationData[i] = mBuffer[i];
    }
    mBuffer.clear();
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
    return mDone;
}

void BlockWrite::init(unsigned int mMsTime, word aStartAddress, word aSize, std::vector<byte>* aReceiveBuffer, byte aReturnCode)
{
    DBERR("BlockWrite::init(startAddress: 0x%04x, size: 0x%04x\n", aStartAddress, aSize);
    mTransferingToPage23 = false;
    mBlockSequenceNr = 0;
    mReceiveIndex = 0;
    mReceivedData = 0;
    mRequestedData = 0;
    mDone = false;
    mDataBlockQueue.clear();

    mBeginTime = mMsTime;
    mStartAddress = aStartAddress;
    mTransferSize = aSize;
    mDefaultReturnCode = aReturnCode;

    mBuffer.clear();
    mBuffer.resize(aSize);
    
    mReceiveBuffer = aReceiveBuffer;
    mReceiveBuffer->clear();
    continueWithNextBlock();
}

// todo:
// - add all blocks to receive from msx to a queue
// - send several 9-byte requests for data 
// - receive blocks of 240+2 bytes, check the markers, request again

void BlockWrite::continueWithNextBlock()
{
    if (getBytesLeft() == 0)
    {
        //DBERR("BlockWrite::continueWithNextBlock, DONE!\n");

        nwhSupport->sendHeader();
        nwhSupport->send(mDefaultReturnCode);
        mDone = true;
        return;
    }

    unsigned int startAddr = mStartAddress + mRequestedData;
    DBERR("BlockWrite::continueWithNextBlock, startAddress: 0x%04X, size: 0x%04X\n", startAddr, getBytesLeft());

    // make sure the transfer does not cross the TWOBANKLIMIT
    if (startAddr < TWOBANKLIMIT)
    {
        word endAddress = std::min(TWOBANKLIMIT-1, startAddr + getBytesLeft());
        word transferSize = endAddress - startAddr;
        requestBlock(startAddr, mTransferSize);
    }
    else
    {
        requestBlock(startAddr, getBytesLeft());
    }  
}

void BlockWrite::requestBlock(word aStartAddress, word aSize)
{
	word lBlockSize = std::min(aSize, BLOCKWRITE_SIZE);
	unsigned int endAddress = aStartAddress + aSize;

    DBERR("BlockWrite::requestBlock, address: 0x%04x, transferSize: 0x%04X \n", aStartAddress, lBlockSize);
    
    if (aStartAddress >= TWOBANKLIMIT && mTransferingToPage23 == false && mDataBlockQueue.size() > 0)
    {
        DBERR("BlockWrite::requestBlock, delaying page23 block until all page01 blocks are done.\n");
        return;
    }

    if (aStartAddress >= TWOBANKLIMIT && mTransferingToPage23 == false)
    {
        // switch to page23 tranfer mode
		nwhSupport->sendHeader();
		nwhSupport->send(BLOCKWRITE_PAGE23_DATA);
        mTransferingToPage23 = true;
    }

    // request the next block of data (consists of 3+1+2+2+1 = 9 bytes)
	nwhSupport->sendHeader();
	nwhSupport->send(0);                // transfer not done
	nwhSupport->send16(aStartAddress);
	nwhSupport->send16(lBlockSize);
	nwhSupport->send(mBlockSequenceNr);
    
    DataBlockWrite* lData = new DataBlockWrite(mBlockSequenceNr, mRequestedData, aStartAddress, lBlockSize);

    DBERR("DataBlockWrite: mBlockSequenceNr: %u, aStartAddress: 0x%04X, lBlockSize: %u\n", mBlockSequenceNr, aStartAddress, lBlockSize);
    mDataBlockQueue.push_back(lData);
    mBlockSequenceNr = (mBlockSequenceNr+1) & 255;
    mRequestedData += lBlockSize;

    //todo: write speed can be optimized by requesting multiple blocks at once, 
    // special care should taken to ensure the msx-receive buffer is does not overflow.
    // for example by making sure there are never more then 256/9 = 28 blocks out-standing.
}

void BlockWrite::receiveData(byte data)
{
    // the datablock we expect to receive next
    DataBlockWrite* lData = mDataBlockQueue.front();

    DBERR("BlockWrite::receiveData, receiveIndex: %u, currentBlockSize: %u\n", lData->getSequenceNr(), lData->getSize());

    if (mReceiveIndex == 0)
    {
        if (lData->getSequenceNr() != data)
        {
            // sequenceNrHeader is not the correct sequenceNr
            // todo: maybe implement a retry mechanism,
            // abort entire transfer for now
            DBERR("BlockWrite::receiveData, sequenceNrHeader: 0x%02X != 0x%02X\n", data, lData->getSequenceNr());
            cancelWithCode(BLOCKWRITE_ERROR);
        }
        else
        {
            //DBERR("BlockWrite::receiveData, BLOCKSTART [0x%02X]\n", data);
        }
        mReceiveIndex++;
    } 
    else if (mReceiveIndex <= lData->getSize())
    {
        DBERR("BlockWrite::receiveData, %u: 0x%02X, bytesLeft: %u\n", mReceivedData, data, getBytesLeft());
        mBuffer[mReceivedData] = data;
        mReceiveIndex++;
        mReceivedData++;
    }
    else
    {
        if (lData->getSequenceNr() != data)
        {
            DBERR("BlockWrite::receiveData, BLOCKEND [0x%02X] block OK\n", data);
            delete lData;
            mDataBlockQueue.pop_front();
            mReceiveIndex = 0;
            continueWithNextBlock();
        }
        else
        {
            DBERR("BlockWrite::receiveData, sequenceNrTail: 0x%02X != 0x%02X\n", data, lData->getSequenceNr());

            DBERR("BlockWrite::receiveData, block ERROR!\n");
            cancelWithCode(BLOCKWRITE_ERROR);
        }
    }
}

void BlockWrite::cancelWithCode(byte returnCode)
{
    nwhSupport->sendHeader();
    nwhSupport->send(returnCode);
    mDone = true;
}

} // namespace nwhost
