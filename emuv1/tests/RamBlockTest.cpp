// RamBlockTest.cpp

#include <gtest/gtest.h>

#include "memory/RamBlock.h"

namespace {

// exposes the protected memory pointer, so the tests can check what write() stored
class TestableRamBlock : public RamBlock {
public:
    using RamBlock::RamBlock;

    nw_byte at(nw_word address) const { return memory[address - offset]; }
};

TEST(RamBlock, StartsZeroed)
{
    TestableRamBlock ram(0x4000);
    EXPECT_EQ(ram.getIdString(), "RamBlock");
    for (nw_word address = 0; address < 0x4000; ++address)
    {
        ASSERT_EQ(ram.at(address), 0) << "address " << address;
    }
}

TEST(RamBlock, WriteStoresRelativeToTheInstallAddress)
{
    layoutType layout;
    TestableRamBlock ram(0x8000);
    ram.install(layout, 3, 2, 0x4000);

    ram.write(0x4000, 0x12);
    ram.write(0xbfff, 0x34);

    EXPECT_EQ(ram.at(0x4000), 0x12);
    EXPECT_EQ(ram.at(0xbfff), 0x34);
}

TEST(RamBlock, InstallBindsItsBlocksOnlyInItsOwnSlot)
{
    layoutType layout; // default constructed delegates are empty
    TestableRamBlock ram(0x8000);
    ram.install(layout, 3, 2, 0x4000);

    // a device is 4 blocks of 8kb by default, installed at 0x4000 that is blocks 2 to 5
    for (unsigned int block = 0; block < BLOCKS; ++block)
    {
        const bool installed = block >= 2 && block <= 5;
        EXPECT_EQ(!layout[3][2][block].empty(), installed) << "block " << block;
    }
    EXPECT_TRUE(layout[3][1][2].empty());
    EXPECT_TRUE(layout[0][0][2].empty());
}

} // namespace
