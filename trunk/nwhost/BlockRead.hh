#ifndef BLOCKREAD_HH
#define BLOCKREAD_HH

#include "NowindTypes.hh"
#include <vector>
#include <deque>

namespace nwhost {

static const byte HARDCODED_READ_DATABLOCK_SIZE = 128;	// hardcoded in blockRead (common.asm)
static const word TWOBANKLIMIT = 0x8000;

class NowindHostSupport;
class DataBlock;

class BlockRead
{
public:
	BlockRead();
	virtual ~BlockRead();
	void Initialize(NowindHostSupport* aSupport);

    void Init(word startAddress, word size, const std::vector <byte >& data);  // just wraps the the first blockRead() and initializes some vars
    void ack(byte tail);
    void blockReadInit(word startAddress, word size, const std::vector <byte >& data);  // just wraps the the first blockRead() and initializes some vars
    void blockRead(word startAddress, word size, const std::vector <byte >& data);
    void blockReadHelper(word startAddress, word size, const std::vector <byte >& data);
    void blockReadContinue();
    void sendDataBlock(unsigned int blocknr);
    void blockReadAck(byte tail);
    bool isDone() const;
    
private:
	word startAddress;
	word transferredData;
	word transferSize;

    bool transferingToPage01;
    NowindHostSupport* nwhSupport;
    std::vector<byte> buffer;                   // work buffer for current tranfer
	std::deque< DataBlock* > dataBlockQueue;    // currently send blocks that are waiting to be acknowledged
	bool done;
    
};

} // namespace nwhost

#endif // BLOCKREAD_HH
