#include "DataBlock.hh"

namespace nwhost {

DataBlock::DataBlock(unsigned int aNumber, const std::vector <byte >& sourceData, unsigned int offset, word aTransferAddress, word aSize)
{
    number = aNumber;
    size = aSize;
    transferAddress = aTransferAddress;
    // TODO: this assumes blocksize is 128
    fastTransfer = ((size & 0x7f) == 0);

    bool byteInUse[256];    // 'byte in use' map
    for (int i=0;i<256;i++)
    {
        byteInUse[i] = false;
    }
    
    data.clear();
    if (fastTransfer)
    {
        transferAddress += size;
        for (unsigned int i=0;i<size;i++)
        {
            byte currentByte = sourceData[offset+i];         
            data.push_front(currentByte);               // reverse the data order
            byteInUse[currentByte] = true;
        }
    }
    else
    {
        for (unsigned int i=0;i<size;i++)
        {
            byte currentByte = sourceData[offset+i];         
            data.push_back(currentByte);                // normal data order
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