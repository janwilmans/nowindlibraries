// BlockWriteTest.cpp, BlockWrite (MSX -> host transfers) against the model of blockWrite in msxsrc/common.asm
//
// BlockWrite is the newer write protocol; NowindHost does not use it yet (USE_OLD_DISKWRITE in NowindHost.cc
// selects doDiskWrite1/2, see NowindHostProtocolTest). Both use the same ROM routine.

#include "BlockWrite.hh"
#include "NowindHostSupport.hh"
#include "support/RomModel.h"

#include <gtest/gtest.h>

#include <vector>

using namespace nwtest;
using nwhost::BlockWrite;
using nwhost::byte;
using nwhost::word;

namespace {

class BlockWriteTest : public ::testing::Test {
protected:
    BlockWriteTest()
        : rom(UsbPort{[this] { return support.read(); }, [this](byte value) { blockWrite.receiveData(value); }})
    {
        blockWrite.initialize(&support);
    }

    void fillMemory(word address, unsigned size)
    {
        for (unsigned i = 0; i < size; ++i) rom.memory.bytes[(address + i) & 0xffff] = byte(i * 13 + 1);
    }

    // the host requests the data, the ROM sends it
    RomResult transfer(word address, unsigned size)
    {
        fillMemory(address, size);
        blockWrite.init(0, address, word(size), &received, BlockWrite::BLOCKWRITE_END);
        RomResult result = rom.blockWrite(address);
        data.assign(size, 0);
        blockWrite.copyData(data);
        return result;
    }

    // the request frames the host sent, without the FF AF 05 headers: (code, address, length, sequence number)
    struct Request {
        byte code;
        word address;
        word length;
        byte sequence;
    };
    std::vector<Request> requests(const std::vector<byte>& stream)
    {
        std::vector<Request> result;
        for (std::size_t i = 0; i + 3 < stream.size(); ++i)
        {
            if (stream[i] != 0xaf || stream[i + 1] != 0x05) continue;
            byte code = stream[i + 2];
            if (code == 0 && i + 7 < stream.size())
            {
                result.push_back({0, word(stream[i + 3] + 256 * stream[i + 4]), word(stream[i + 5] + 256 * stream[i + 6]),
                                  stream[i + 7]});
            }
            else
            {
                result.push_back({code, 0, 0, 0});
            }
        }
        return result;
    }

    std::vector<byte> pending()
    {
        std::vector<byte> stream;
        while (support.isDataAvailable()) stream.push_back(support.read());
        return stream;
    }

    nwhost::NowindHostSupport support;
    BlockWrite blockWrite;
    RomModel rom;
    std::vector<byte> received;
    std::vector<byte> data;
};

} // namespace

TEST_F(BlockWriteTest, RequestsAreAddressLengthAndASequenceNumber)
{
    fillMemory(0xc000, 500);
    blockWrite.init(0, 0xc000, 500, &received, BlockWrite::BLOCKWRITE_END);
    std::vector<Request> frames = requests(pending());
    ASSERT_GE(frames.size(), 3u);
    EXPECT_EQ(frames[0].code, 0);
    EXPECT_EQ(frames[0].address, 0xc000u);
    EXPECT_EQ(frames[0].length, 240u);
    EXPECT_EQ(frames[0].sequence, 0);
    EXPECT_EQ(frames[1].address, 0xc000u + 240);
    EXPECT_EQ(frames[1].sequence, 1);
    EXPECT_EQ(frames[2].address, 0xc000u + 480);
    EXPECT_EQ(frames[2].length, 20u);
    EXPECT_EQ(frames[2].sequence, 2);
}

TEST_F(BlockWriteTest, TheRomSendsTheRequestedMemory)
{
    RomResult result = transfer(0xc000, 1000);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, BlockWrite::BLOCKWRITE_END);
    EXPECT_EQ(data, rom.memory.range(0xc000, 1000));
    EXPECT_TRUE(blockWrite.isDone());
}

// DISABLED: BlockWrite::init calls continueWithNextBlock() 8 times up front, and every call after the last block
// was requested sends another 'FF AF 05 FF' (END); continueWithNextBlock() also sends one after every received
// block. The ROM (blockWrite01/23, common.asm) stops at the first END, so the others stay in the fifo and the next
// command reads a stale reply. Host: nwhost/BlockWrite.cc init() and continueWithNextBlock().
// ROM: msxsrc/common.asm blockWrite01 'ret m'. Not active: NowindHost uses the old diskwrite (USE_OLD_DISKWRITE).
TEST_F(BlockWriteTest, DISABLED_NothingIsLeftInTheFifoAfterTheTransfer)
{
    transfer(0xc000, 500);
    EXPECT_FALSE(support.isDataAvailable()) << "left over: " << ::testing::PrintToString(pending());
}

// DISABLED: for a transfer that starts in page 0/1, continueWithNextBlock() computes the size up to 0x8000 but then
// calls requestBlock(startAddr, mTransferSize), so a block can run past 0x8000 while the ROM still runs
// blockWrite01 (nowind in page 2), where 0x8000-0x9FFF is the nowind window instead of RAM.
// Host: nwhost/BlockWrite.cc continueWithNextBlock(). ROM: msxsrc/common.asm blockWrite (page 2 for < 0x8000).
TEST_F(BlockWriteTest, DISABLED_NoRequestCrosses0x8000)
{
    fillMemory(0x7f80, 0x200);
    blockWrite.init(0, 0x7f80, 0x200, &received, BlockWrite::BLOCKWRITE_END);
    for (const Request& frame : requests(pending()))
    {
        if (frame.code != 0) continue;
        EXPECT_FALSE(frame.address < 0x8000 && frame.address + frame.length > 0x8000)
            << "request 0x" << std::hex << frame.address << " + 0x" << frame.length << " crosses 0x8000";
    }
}

TEST_F(BlockWriteTest, AWrongSequenceNumberEndsTheTransfer)
{
    fillMemory(0xc000, 100);
    blockWrite.init(0, 0xc000, 100, &received, BlockWrite::BLOCKWRITE_END);
    pending();
    blockWrite.receiveData(7); // the request had sequence number 0
    EXPECT_TRUE(blockWrite.isDone());
}

TEST(BlockWriteRomModel, TheRomTreatsTheReturnCodesAsCommonAsmSays)
{
    // blockWrite01/23: 'or a / ret m' exits without error for 0x80..0xFF, 'jr nz,.error' for 1..127;
    // blockWrite: 'ret pe' after blockWrite01 returns for 0xFF (even parity) and goes on in page 2/3 for 0xFE
    EXPECT_FALSE(RomModel::isWriteError(0));
    EXPECT_TRUE(RomModel::isWriteError(1));
    EXPECT_TRUE(RomModel::isWriteError(127));
    EXPECT_FALSE(RomModel::isWriteError(0x80));
    EXPECT_TRUE(RomModel::evenParity(BlockWrite::BLOCKWRITE_END));
    EXPECT_FALSE(RomModel::evenParity(BlockWrite::BLOCKWRITE_PAGE23_DATA));
}

// DISABLED: on a wrong sequence number BlockWrite::cancelWithCode() sends nothing (the send is commented out, with a
// todo), so the ROM waits in getHeader until it times out. Had it sent BLOCKWRITE_ERROR (128), the ROM would not
// see an error either: blockWrite01/23 exit with 'ret m' for 0x80..0xFF, only 1..127 are errors.
// Host: nwhost/BlockWrite.cc cancelWithCode(), BLOCKWRITE_ERROR in BlockWrite.hh. ROM: msxsrc/common.asm blockWrite01.
TEST_F(BlockWriteTest, DISABLED_AWrongSequenceNumberIsReportedToTheRom)
{
    fillMemory(0xc000, 100);
    blockWrite.init(0, 0xc000, 100, &received, BlockWrite::BLOCKWRITE_END);
    pending();
    blockWrite.receiveData(7); // the request had sequence number 0
    RomResult result = rom.blockWrite(0xc000);
    EXPECT_FALSE(result.timeout);
    EXPECT_TRUE(RomModel::isWriteError(result.code)) << "code " << int(result.code);
}
