#ifndef BLOCKREAD_HH
#define BLOCKREAD_HH

#include "NowindTypes.hh"
#include "NwhostExports.h"
#include <vector>

namespace nwhost {

static const byte HARDCODED_READ_DATABLOCK_SIZE = 128;	// hardcoded in blockRead (common.asm)
static const word TWOBANKLIMIT = 0x8000;

class NWHOST_API BlockRead
{
public:
	BlockRead();
	virtual ~BlockRead();

    void Init(word startAddress, word size, const std::vector <byte >& data);  // just wraps the the first blockRead() and initializes some vars
    
private:
    bool transferingToPage01;
    
};

} // namespace nwhost

#endif // BLOCKREAD_HH
