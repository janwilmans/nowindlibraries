#ifndef BLOCKWRITE_HH
#define BLOCKWRITE_HH

#include "NowindTypes.hh"
#include <vector>
#include <deque>

namespace nwhost {
class NowindHostSupport;
class DataBlockWrite;

class BlockWrite
{
    static const word TWOBANKLIMIT = 0x8000;

public:
	BlockWrite();
	virtual ~BlockWrite();
	void initialize(NowindHostSupport* aSupport);

    void init(unsigned int aTimeMs, word aStartAddress, word aSize, std::vector<byte>* aReceiveBuffer, byte aReturnCode);
    void continueWithNextBlock();
    void requestBlock(word startAddress, word aSize);
    void receiveData(byte aData);
    bool isDone() const;
    void cancelWithCode(byte aReturncode);
    word getTransferSize() { return mTransferSize; }
   
    void copyData(std::vector<byte>& aDestinationData);

	enum {
		BLOCKWRITE_EXIT = 0,
		BLOCKWRITE_ERROR = 128,
        BLOCKWRITE_PAGE23_DATA = 0xfe,
        BLOCKWRITE_END = 255,
	};

private:
    word getBytesLeft() { return mTransferSize-mRequestedData; }

    unsigned int mBeginTime;                // used for calculating speeds (information only)
    unsigned int mEndTime;
	word mStartAddress;                     // start address in msx memory to receive data from
	word mTransferSize;                     // size of the entire transfer
	word mRequestedData;                    // amount of bytes for which requests have been done
    word mReceivedData;                     // amount of bytes received 
    word mBlockSequenceNr;                  // range 0-255, looping
    word mReceiveIndex;                     // per block index, 0-(BLOCKWRITE_SIZE+1)
    bool mTransferingToPage23;              // false while outstanding requested blocks are in Page01

    NowindHostSupport* nwhSupport;
    std::vector<byte> mBuffer;              // work buffer for receiving all blocks
    std::vector<byte>* mReceiveBuffer;      // destination buffer to copy data when all blocks are transfered successfully
    std::deque<DataBlockWrite*> mDataBlockQueue;    // contains information on outstanding requests
	bool mDone;
	byte mDefaultReturnCode;
    
};

} // namespace nwhost

#endif // BLOCKWRITE_HH
