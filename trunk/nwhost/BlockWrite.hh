#ifndef BLOCKWRITE_HH
#define BLOCKWRITE_HH

#include "NowindTypes.hh"
#include <vector>
#include <deque>

namespace nwhost {

static const word TWOBANKLIMIT = 0x8000;

class NowindHostSupport;
class DataBlock;

class BlockWrite
{
public:
	BlockWrite();
	virtual ~BlockWrite();
	void initialize(NowindHostSupport* aSupport);

    void init(word aStartAddress, word aSize, std::vector<byte>* data, byte aReturnCode);
    void blockWrite(word startAddress, word size);
    void blockWriteHelper(word startAddress, word size);
    void receiveData(byte data);
    bool isDone() const;
    void cancelWithCode(byte);
   
private:

	word startAddress;
	word processedData;
	word transferSize;
    word blockSequenceNr;
    word receiveIndex;
    byte sequenceNrHeader;
    byte sequenceNrTail;

    bool transferingToPage01;
    NowindHostSupport* nwhSupport;
    std::vector<byte> buffer;                  // work buffer for current tranfer
    std::vector<byte>* receiveBuffer;
	bool done;
	byte returnCode;
    word bytesLeft;
    
};

} // namespace nwhost

#endif // BLOCKWRITE_HH
