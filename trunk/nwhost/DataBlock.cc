#include "DataBlock.hh"

namespace nwhost {

DataBlock::DataBlock(unsigned int aNumber, const std::vector <byte >& sourceData, unsigned int offset, word aTransferAddress, word size)
{
    //assert(size < 255);
    number = aNumber;
    transferAddress = aTransferAddress + size;

    bool byteInUse[256];    // byte in use map
    for (int i=0;i<256;i++)
    {
        byteInUse[i] = false;
    }

    for (unsigned int i=0;i<size;i++)
    {
        byte currentByte = sourceData[offset+size-1-i];      // reverse the data order
        data.push_back(currentByte);
        byteInUse[currentByte] = true;
    }

    for (int i=0;i<256;i++)
    {
        if (byteInUse[i] == false)
        {
            // found our header (first byte not 'used' by the data)
            header = i;
            break;
        }
    }
}

DataBlock::~DataBlock()
{
}

} // namespace