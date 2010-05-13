#ifndef DATABLOCK_HH
#define DATABLOCK_HH

#include "NowindTypes.hh"
#include <vector>

namespace nwhost {

class DataBlock {
public:
    DataBlock(unsigned int aNumber, const std::vector <byte >& sourceData, unsigned int offset, word aTransferAddress, word size);

    word number;
    byte header;
    word size;
    bool fastTransfer;
    word transferAddress;
    std::vector < byte > data;

    ~DataBlock();

};

#endif // DATABLOCK_HH

} // namespace
