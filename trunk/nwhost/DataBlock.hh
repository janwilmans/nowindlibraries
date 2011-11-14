#ifndef DATABLOCK_HH
#define DATABLOCK_HH

#include "NowindTypes.hh"
#include <vector>
#include <deque>

namespace nwhost {

class DataBlock {
public:
    DataBlock(unsigned int aNumber, const std::vector <byte >& sourceData, unsigned int offset, word aTransferAddress, word aSize);
    ~DataBlock();
  
    word number;
    byte header;
    word size;
    bool fastTransfer;
    word offset;
    word transferAddress;
    std::deque<byte> data;

private:
    DataBlock();

};

#endif // DATABLOCK_HH

} // namespace
