// DataBlockTest.cpp

#include "DataBlock.hh"

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

using nwhost::byte;
using nwhost::DataBlock;

namespace {

std::vector<byte> counting(unsigned size, byte first = 0)
{
    std::vector<byte> data(size);
    for (unsigned i = 0; i < size; ++i) data[i] = byte(first + i);
    return data;
}

} // namespace

TEST(DataBlock, HeaderIsTheLowestByteValueNotInTheData)
{
    // the header doubles as the tail, the MSX compares them to detect a broken transfer
    std::vector<byte> data = counting(10); // 0..9
    DataBlock block(0, data, 0, 0xc000, 10);
    EXPECT_EQ(block.header, 10);

    std::vector<byte> noZero(10, 0x55);
    DataBlock other(0, noZero, 0, 0xc000, 10);
    EXPECT_EQ(other.header, 0);
}

TEST(DataBlock, HeaderOnlyLooksAtTheBytesOfThisBlock)
{
    std::vector<byte> data = counting(20); // 0..19
    DataBlock block(0, data, 10, 0xc000, 5); // 10..14
    EXPECT_EQ(block.header, 0);
    EXPECT_EQ(std::vector<byte>(block.data.begin(), block.data.end()), (std::vector<byte>{10, 11, 12, 13, 14}));
}

TEST(DataBlock, SlowBlocksKeepTheOrderAndTheStartAddress)
{
    std::vector<byte> data = counting(100, 1);
    DataBlock block(0, data, 0, 0xc000, 100);
    EXPECT_FALSE(block.fastTransfer);
    EXPECT_EQ(block.transferAddress, 0xc000u);
    EXPECT_EQ(std::vector<byte>(block.data.begin(), block.data.end()), data);
}

TEST(DataBlock, FastBlocksAreReversedAndAddressedAtTheirEnd)
{
    // blockReadTranfer (common.asm) sets SP to the end address and pushes the data downwards
    std::vector<byte> data = counting(128, 1);
    DataBlock block(3, data, 0, 0xc000, 128);
    EXPECT_TRUE(block.fastTransfer);
    EXPECT_EQ(block.number, 3u);
    EXPECT_EQ(block.transferAddress, 0xc080u);
    std::vector<byte> reversed(data.rbegin(), data.rend());
    EXPECT_EQ(std::vector<byte>(block.data.begin(), block.data.end()), reversed);
}

TEST(DataBlock, OnlyMultiplesOf128AreFast)
{
    std::vector<byte> data(256, 1);
    EXPECT_TRUE(DataBlock(0, data, 0, 0, 256).fastTransfer);
    EXPECT_FALSE(DataBlock(0, data, 0, 0, 127).fastTransfer);
    EXPECT_FALSE(DataBlock(0, data, 0, 0, 129).fastTransfer);
}
