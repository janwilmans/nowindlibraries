#include "DataBlockWrite.hh"

namespace nwhost {

DataBlockWrite::DataBlockWrite(unsigned int aSequenceNumber, unsigned int aOffset, word aTransferAddress, word aSize)
{
    mPage23 = false;
    mSequenceNr = aSequenceNumber;
    mOffset = aOffset;
    mSize = aSize;
    mTransferAddress = aTransferAddress;
    if (mTransferAddress >= 0x8000)
    {
        mPage23 = true;
    }
}

DataBlockWrite::~DataBlockWrite()
{
}

} // namespace