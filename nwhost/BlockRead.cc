
#define NWHOST_API_EXPORT
#include "BlockRead.hh"

namespace nwhost {

BlockRead::BlockRead()
{
}

BlockRead::~BlockRead()
{
}

void BlockRead::Init(word startAddress, word size, const std::vector <byte >& data)
{
    transferingToPage01 = (startAddress >= TWOBANKLIMIT);
}

} // namespace nwhost
