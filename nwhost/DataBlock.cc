#include "DataBlock.hh"

namespace nwhost {

DataBlock::DataBlock(unsigned int aNumber, const std::vector <byte >& sourceData, unsigned int offset, word aTransferAddress, word aSize)
{
    //assert(size < 255);
    number = aNumber;
    size = aSize;
    fastTransfer = ((size & 0x7f) == 0);
    transferAddress = aTransferAddress + size;
    data.clear();

    bool byteInUse[256];    // 'byte in use' map
    for (int i=0;i<256;i++)
    {
        byteInUse[i] = false;
    }
    
    if (fastTransfer)
    {
        for (unsigned int i=0;i<size;i++)
        {
            byte currentByte = sourceData[offset+size-1-i];         // reverse the data order
            data.push_back(currentByte);              
            byteInUse[currentByte] = true;
        }
    }
    else
    {
        for (unsigned int i=0;i<size;i++)
        {
            byte currentByte = sourceData[i];         // normal data order
            data.push_back(currentByte);              
            byteInUse[currentByte] = true;
        }    
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