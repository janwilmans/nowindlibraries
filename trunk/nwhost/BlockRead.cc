
#define NWHOST_API_EXPORT
#include "BlockRead.hh"
#include "NowindHostSupport.hh"

#define DBERR nwhSupport->debugMessage

namespace nwhost {

BlockRead::BlockRead()
{
}

void BlockRead::Initialize(NowindHostSupport* aSupport)
{
    nwhSupport = aSupport;
}

BlockRead::~BlockRead()
{
}

void BlockRead::Init(word startAddress, word size, const std::vector <byte >& data)
{
    DBERR("BlockRead::Init(startAddress: 0x%04x, size: 0x%04x\n", startAddress, size);
    
//    transferingToPage01 = (startAddress >= TWOBANKLIMIT);
}

} // namespace nwhost
