#ifndef BLOCKWRITE_HH
#define BLOCKWRITE_HH

#include "NowindTypes.hh"
#include <vector>
#include <deque>

namespace nwhost {
class NowindHostSupport;
class DataBlock;

class BlockWrite
{
    static const word TWOBANKLIMIT = 0x8000;

public:
	BlockWrite();
	virtual ~BlockWrite();
	void initialize(NowindHostSupport* aSupport);

    void init(word aStartAddress, word aSize, std::vector<byte>* data, byte aReturnCode);
    void continueWithNextBlock();
    void requestBlock(word startAddress, word size);
    void receiveData(byte data);
    bool isDone() const;
    void cancelWithCode(byte);
    word getTransferSize() { return transferSize; }
   
    void copyData(std::vector<byte>& destinationData);

	enum {
		BLOCKWRITE_EXIT = 0,
		BLOCKWRITE_ERROR = 128,
        BLOCKWRITE_PAGE23_DATA = 0xfe,
        BLOCKWRITE_END = 255,
	};

private:

	word startAddress;
	word processedData;
	word transferSize;                  // size of the entire transfer
	word currentBlockSize;              // size of the current block
    word blockSequenceNr;
    word receiveIndex;
    byte sequenceNrHeader;
    byte sequenceNrTail;

    bool transferingToPage01;
    NowindHostSupport* nwhSupport;
    std::vector<byte> buffer;            // work buffer for receiving the current block
    std::vector<byte>* receiveBuffer;
    std::deque<byte> SequenceNrQueue;    // contains blocks for outstanding requests
	bool done;
	byte defaultReturnCode;
    word bytesLeft;
    
};

} // namespace nwhost

#endif // BLOCKWRITE_HH
