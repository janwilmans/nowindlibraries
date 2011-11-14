#ifndef DATABLOCKWRITE_HH
#define DATABLOCKWRITE_HH

#include "NowindTypes.hh"
#include <vector>
#include <deque>

namespace nwhost {

class DataBlockWrite {
public:
    DataBlockWrite(unsigned int aNumber, unsigned int aOffset, word aTransferAddress, word aSize);
    word getSequenceNr() { return mSequenceNr; }
    unsigned int getOffset() { return mOffset; }
    word getSize() { return mSize; }
    bool getPage23() { return mPage23; }
    word getTransferAddress() { return mTransferAddress; }
    ~DataBlockWrite();
  
private:
    word mSequenceNr;
    unsigned int mOffset;
    word mSize;
    bool mPage23;
    word mTransferAddress;

    DataBlockWrite();

};

#endif // DATABLOCKWRITE_HH

} // namespace
