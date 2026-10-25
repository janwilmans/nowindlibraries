// SupportTest.cpp, checks the test helpers themselves

#include "support/Fat12Image.h"
#include "support/HostFixture.h"

#include <gtest/gtest.h>

using namespace nwtest;

TEST(Fat12Image, BootSectorHoldsTheGeometry)
{
    Fat12Image image(format720k());
    ASSERT_EQ(image.data.size(), 1440u * 512);
    const std::uint8_t* boot = image.data.data();
    EXPECT_EQ(boot[11] | (boot[12] << 8), 512);
    EXPECT_EQ(boot[13], 2);
    EXPECT_EQ(boot[16], 2);
    EXPECT_EQ(boot[17] | (boot[18] << 8), 112);
    EXPECT_EQ(boot[19] | (boot[20] << 8), 1440);
    EXPECT_EQ(boot[21], 0xf9);
    EXPECT_EQ(boot[22], 3);
    EXPECT_EQ(image.firstDirSector(), 7u);
    EXPECT_EQ(image.firstDataSector(), 14u);
    // the FAT starts with the media descriptor
    EXPECT_EQ(image.data[512], 0xf9);
    EXPECT_EQ(image.data[513], 0xff);
}

TEST(Fat12Image, ReopensAnImageFromItsBootSector)
{
    Fat12Image original(msxFloppyFormats()[0]);
    original.addFile("A.TXT", "abc");
    Fat12Image reopened(original.data);
    EXPECT_EQ(reopened.format.media, 0xf8);
    EXPECT_EQ(reopened.format.totalSectors, 720);
    EXPECT_EQ(reopened.firstDataSector(), original.firstDataSector());
    EXPECT_EQ(reopened.fileNames(), std::vector<std::string>{"A.TXT"});
}

TEST(Fat12Image, FilesRoundTripOverSeveralClusters)
{
    Fat12Image image;
    std::vector<std::uint8_t> big(5000);
    for (std::size_t i = 0; i < big.size(); ++i) big[i] = std::uint8_t(i * 7);
    image.addFile("small.txt", "hello");
    image.addFile("BIG.BIN", big);

    EXPECT_EQ(image.fileNames(), (std::vector<std::string>{"SMALL.TXT", "BIG.BIN"}));
    auto small = image.readFile("SMALL.TXT");
    ASSERT_TRUE(small);
    EXPECT_EQ(std::string(small->begin(), small->end()), "hello");
    EXPECT_EQ(image.readFile("big.bin"), big);
    EXPECT_FALSE(image.readFile("MISSING.TXT"));
    // 1 cluster for SMALL.TXT (cluster 2), 5 for BIG.BIN (3..7), odd and even FAT12 entries
    EXPECT_EQ(image.getFat(2), 0xfffu);
    EXPECT_EQ(image.getFat(3), 4u);
    EXPECT_EQ(image.getFat(4), 5u);
    EXPECT_EQ(image.getFat(7), 0xfffu);
    EXPECT_EQ(image.getFat(8), 0u);
}

TEST(MemoryDisk, ReadsAndWritesWholeSectors)
{
    MemoryDisk disk(4);
    std::vector<nw_byte> buffer(1024, 0x5a);
    EXPECT_EQ(disk.writeSectors(buffer.data(), 2, 2), 0);
    EXPECT_EQ(disk.sector(2)[0], 0x5a);
    EXPECT_EQ(disk.sector(3)[511], 0x5a);
    EXPECT_EQ(disk.sector(1)[511], 0);

    std::vector<nw_byte> back(1024);
    EXPECT_EQ(disk.readSectors(back.data(), 2, 2), 0);
    EXPECT_EQ(back, buffer);
    EXPECT_NE(disk.readSectors(back.data(), 3, 2), 0) << "past the end";
    EXPECT_TRUE(disk.diskChanged());
    EXPECT_FALSE(disk.diskChanged()) << "the change flag is reset once read";
}

class MsxSideTest : public HostFixture {};

TEST_F(MsxSideTest, ACommandFrameGetsAReplyWithAHeader)
{
    createHost();
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 1);
    EXPECT_FALSE(msx->hasData());
}
