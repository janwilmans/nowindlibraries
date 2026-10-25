// BlockReadTest.cpp, BlockRead (host -> MSX transfers) against the model of blockRead in msxsrc/common.asm

#include "BlockRead.hh"
#include "NowindHostSupport.hh"
#include "support/RomModel.h"

#include <gtest/gtest.h>

#include <vector>

using namespace nwtest;
using nwhost::BlockRead;
using nwhost::byte;
using nwhost::word;

namespace {

std::vector<byte> pattern(unsigned size)
{
    std::vector<byte> data(size);
    for (unsigned i = 0; i < size; ++i) data[i] = byte((i * 31) ^ (i >> 8));
    return data;
}

class BlockReadTest : public ::testing::Test {
protected:
    BlockReadTest()
        : rom(UsbPort{[this] { return support.read(); }, [this](byte tail) { blockRead.ack(tail); }})
    {
        blockRead.initialize(&support);
    }

    // the host starts the transfer, the ROM receives it at the same address
    RomResult transfer(word address, const std::vector<byte>& data, byte returnCode = BlockRead::BLOCKREAD_EXIT)
    {
        blockRead.init(address, word(data.size()), data, returnCode);
        return rom.blockRead(address);
    }

    nwhost::NowindHostSupport support;
    BlockRead blockRead;
    RomModel rom;
};

} // namespace

TEST_F(BlockReadTest, LessThan128BytesIsOneSlowBlock)
{
    std::vector<byte> data = pattern(100);
    RomResult result = transfer(0xc000, data);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(result.slowBlocks, 1u);
    EXPECT_EQ(result.fastBlocks, 0u);
    EXPECT_EQ(rom.memory.range(0xc000, 100), data);
    EXPECT_TRUE(blockRead.isDone());
    EXPECT_FALSE(support.isDataAvailable());
}

TEST_F(BlockReadTest, MultiplesOf128AreFastBlocks)
{
    std::vector<byte> data = pattern(512);
    RomResult result = transfer(0xc000, data);
    EXPECT_EQ(result.fastBlocks, 4u);
    EXPECT_EQ(result.slowBlocks, 0u);
    EXPECT_EQ(result.resyncs, 0u);
    EXPECT_EQ(rom.memory.range(0xc000, 512), data);
    EXPECT_TRUE(blockRead.isDone());
}

TEST_F(BlockReadTest, TheRestAfterTheFastBlocksIsASlowBlock)
{
    std::vector<byte> data = pattern(3 * 128 + 50);
    RomResult result = transfer(0xc000, data);
    EXPECT_EQ(result.fastBlocks, 3u);
    EXPECT_EQ(result.slowBlocks, 1u);
    EXPECT_EQ(rom.memory.range(0xc000, unsigned(data.size())), data);
}

TEST_F(BlockReadTest, ATransferAcrossPage1And2SwitchesTheRomToPage23)
{
    // blockRead01 runs with nowind in page 2, it exits on 0 and blockRead23 continues with nowind in page 0
    std::vector<byte> data = pattern(0x200);
    RomResult result = transfer(0x7f00, data);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(result.fastBlocks, 4u);
    EXPECT_EQ(rom.memory.range(0x7f00, 0x200), data);
    EXPECT_TRUE(blockRead.isDone());
}

TEST_F(BlockReadTest, ATransferThatEndsAt0x8000StaysInPage01)
{
    std::vector<byte> data = pattern(0x100);
    RomResult result = transfer(0x7f00, data);
    EXPECT_EQ(result.code, BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(rom.memory.range(0x7f00, 0x100), data);
}

TEST_F(BlockReadTest, ATransferStartingAt0x8000IsPage23FromTheStart)
{
    std::vector<byte> data = pattern(300);
    RomResult result = transfer(0x8000, data);
    EXPECT_EQ(result.code, BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(rom.memory.range(0x8000, 300), data);
}

TEST_F(BlockReadTest, MoreThan255BlocksAreSentInSeveralChunks)
{
    // the MSX gets the block count in one byte; 32 kB is 256 blocks, so 255 + 1
    std::vector<byte> data = pattern(0x8000);
    RomResult result = transfer(0x8000, data);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(result.fastBlocks, 256u);
    EXPECT_EQ(rom.memory.range(0x8000, 0x8000), data);
}

TEST_F(BlockReadTest, TheReturnCodeEndsTheTransfer)
{
    std::vector<byte> data = pattern(10);
    RomResult result = transfer(0xc000, data, 0x80);
    EXPECT_EQ(result.code, 0x80);
    EXPECT_EQ(rom.memory.range(0xc000, 10), data);
}

TEST_F(BlockReadTest, AWrongTailResendsAFastBlock)
{
    std::vector<byte> data = pattern(128);
    blockRead.init(0xc000, 128, data);
    std::vector<byte> first;
    while (support.isDataAvailable()) first.push_back(support.read());
    // FF AF 05, 01 (fast), end address, 1 block, header, 128 bytes, tail
    ASSERT_EQ(first.size(), 3u + 1 + 2 + 1 + 1 + 128 + 1);
    byte header = first[7];

    blockRead.ack(byte(header + 1));
    std::vector<byte> resend;
    while (support.isDataAvailable()) resend.push_back(support.read());
    ASSERT_EQ(resend.size(), first.size());
    EXPECT_EQ(std::vector<byte>(resend.begin(), resend.begin() + 4), (std::vector<byte>{0xff, 0xaf, 0x05, 1}));
    EXPECT_EQ(resend[4] + 256 * resend[5], 0xc080);
    EXPECT_EQ(resend[6], 1) << "one block";
    EXPECT_EQ(std::vector<byte>(resend.begin() + 7, resend.end()), std::vector<byte>(first.begin() + 7, first.end()));
    EXPECT_FALSE(blockRead.isDone());

    blockRead.ack(header);
    EXPECT_TRUE(blockRead.isDone());
}

TEST_F(BlockReadTest, AWrongTailResendsASlowBlock)
{
    std::vector<byte> data = pattern(10);
    blockRead.init(0xc000, 10, data);
    std::vector<byte> first;
    while (support.isDataAvailable()) first.push_back(support.read());
    // FF AF 05, 02 (slow), address, length, header, 10 bytes, tail
    ASSERT_EQ(first.size(), 3u + 1 + 2 + 2 + 1 + 10 + 1);
    byte header = first[8];

    blockRead.ack(byte(header + 1));
    std::vector<byte> resend;
    while (support.isDataAvailable()) resend.push_back(support.read());
    EXPECT_EQ(resend, first);
}

TEST_F(BlockReadTest, ExtraFFBytesInTheStreamAreSkippedBeforeAHeader)
{
    // invalidHeader in common.asm skips 0xFF (an empty fifo) where a block header is expected
    std::vector<byte> data = pattern(256);
    blockRead.init(0xc000, 256, data);
    std::vector<byte> stream;
    while (support.isDataAvailable()) stream.push_back(support.read());
    // between the two fast blocks: header + 128 bytes + tail after the 7 byte preamble
    stream.insert(stream.begin() + 7 + 130, {0xff, 0xff, 0xff});

    std::size_t position = 0;
    RomModel model(UsbPort{[&] { return position < stream.size() ? stream[position++] : byte(0xff); },
                           [&](byte tail) {
                               blockRead.ack(tail);
                               // the host only queues the exit code after the last ack
                               while (support.isDataAvailable()) stream.push_back(support.read());
                           }});
    RomResult result = model.blockRead(0xc000);
    EXPECT_EQ(result.code, BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(result.resyncs, 0u);
    EXPECT_EQ(model.memory.range(0xc000, 256), data);
}

TEST_F(BlockReadTest, AnUnderrunInsideABlockIsRecoveredByTheResend)
{
    // an underrun makes the MSX read 0xFF bytes that the host never sent (the host has commented-out code in
    // BlockRead::sendDataBlock that simulates this); the tail then does not match, the ROM acks the wrong tail
    // and waits for the header value (.errorInPage1/2), the host resends the block
    std::vector<byte> data = pattern(256);
    blockRead.init(0xc000, 256, data);
    std::vector<byte> stream;
    while (support.isDataAvailable()) stream.push_back(support.read());
    stream.insert(stream.begin() + 7 + 1, {0xff, 0xff, 0xff, 0xff}); // right after the first block's header

    std::size_t position = 0;
    RomModel model(UsbPort{[&] { return position < stream.size() ? stream[position++] : byte(0xff); },
                           [&](byte tail) {
                               blockRead.ack(tail);
                               while (support.isDataAvailable()) stream.push_back(support.read());
                           }});
    model.readBudget = 100000;
    RomResult result = model.blockRead(0xc000);
    EXPECT_GT(result.resyncs, 0u) << "the underrun should have broken the first block";
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(model.memory.range(0xc000, 256), data);
    EXPECT_TRUE(blockRead.isDone());
}
