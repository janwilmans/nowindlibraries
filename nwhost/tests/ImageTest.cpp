// ImageTest.cpp, disk and hard disk images (Image, ImageHandler) and nowmap's partition switch

#include "Image.h"
#include "ImageHandler.h"
#include "support/Fat12Image.h"
#include "support/HostFixture.h"
#include "support/TempDir.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <vector>

using namespace nwtest;

namespace {

std::vector<std::uint8_t> sectors(unsigned count, std::uint8_t seed)
{
    std::vector<std::uint8_t> data(count * 512);
    for (unsigned s = 0; s < count; ++s)
    {
        for (unsigned i = 0; i < 512; ++i) data[s * 512 + i] = std::uint8_t(seed + s * 3 + i);
    }
    return data;
}

struct PartitionEntry {
    std::uint8_t flags; // 0x80 bootable, 0x02 disabled, 0x01 read only
    std::uint32_t startLBA;
    std::uint32_t length;
};

// a nowind hard disk image: the partition table is read backwards from 0x1EE, 16 bytes per entry (Image.cpp)
std::vector<std::uint8_t> hardDisk(const std::vector<PartitionEntry>& partitions, unsigned totalSectors)
{
    std::vector<std::uint8_t> image(totalSectors * 512);
    for (unsigned s = 1; s < totalSectors; ++s) image[s * 512] = std::uint8_t(s); // sector number in the first byte
    for (std::size_t p = 0; p < partitions.size(); ++p)
    {
        std::uint8_t* entry = &image[0x1ee - p * 16];
        entry[0] = partitions[p].flags;
        for (int i = 0; i < 4; ++i)
        {
            entry[8 + i] = std::uint8_t(partitions[p].startLBA >> (8 * i));
            entry[12 + i] = std::uint8_t(partitions[p].length >> (8 * i));
        }
    }
    image[510] = 0x55;
    image[511] = 0xaa;
    return image;
}

std::uint8_t firstByteOfSector0(Image& image)
{
    std::vector<nw_byte> buffer(512);
    EXPECT_EQ(image.readSectors(buffer.data(), 0, 1), 0);
    return buffer[0];
}

class ImageTest : public ::testing::Test {
protected:
    TempDir dir;
};

} // namespace

TEST_F(ImageTest, ADiskImageReadsAndWritesSectors)
{
    std::vector<std::uint8_t> content = sectors(10, 1);
    std::string path = dir.write("disk.dsk", content);
    Image image;
    ASSERT_TRUE(image.openDiskImage(path));
    EXPECT_TRUE(image.hasDisk());
    EXPECT_FALSE(image.isWriteProtected());
    EXPECT_FALSE(image.isHarddiskPartition());
    EXPECT_EQ(image.getDescription(), path);
    EXPECT_EQ(image.getSectorSize(), 512u);

    std::vector<nw_byte> buffer(1024);
    ASSERT_EQ(image.readSectors(buffer.data(), 3, 2), 0);
    EXPECT_EQ(buffer, std::vector<nw_byte>(content.begin() + 3 * 512, content.begin() + 5 * 512));

    std::vector<nw_byte> update(512, 0xee);
    ASSERT_EQ(image.writeSectors(update.data(), 7, 1), 0);
    ASSERT_EQ(image.readSectors(buffer.data(), 7, 1), 0);
    EXPECT_EQ(std::vector<nw_byte>(buffer.begin(), buffer.begin() + 512), update);
}

TEST_F(ImageTest, WritesReachTheFile)
{
    std::string path = dir.write("disk.dsk", sectors(4, 0));
    {
        Image image;
        ASSERT_TRUE(image.openDiskImage(path));
        std::vector<nw_byte> update(512, 0x42);
        ASSERT_EQ(image.writeSectors(update.data(), 2, 1), 0);
    }
    std::vector<std::uint8_t> file = dir.read("disk.dsk");
    EXPECT_EQ(file[2 * 512], 0x42);
    EXPECT_EQ(file[3 * 512 - 1], 0x42);
}

TEST_F(ImageTest, AReadOnlyFileIsWriteProtected)
{
    std::string path = dir.write("ro.dsk", sectors(2, 0));
    std::filesystem::permissions(path, std::filesystem::perms::owner_read, std::filesystem::perm_options::replace);
    if (std::ofstream(path, std::ios::app).good()) GTEST_SKIP() << "running with rights to write read-only files";
    Image image;
    ASSERT_TRUE(image.openDiskImage(path));
    EXPECT_TRUE(image.isWriteProtected());
    std::vector<nw_byte> buffer(512);
    EXPECT_EQ(image.readSectors(buffer.data(), 1, 1), 0);
}

TEST_F(ImageTest, AMissingFileIsNoDisk)
{
    Image image;
    EXPECT_FALSE(image.openDiskImage(dir.file("missing.dsk")));
    EXPECT_FALSE(image.hasDisk());
    std::vector<nw_byte> buffer(512);
    EXPECT_NE(image.readSectors(buffer.data(), 0, 1), 0);
}

TEST_F(ImageTest, AReadPastTheEndFailsAndTheNextReadWorks)
{
    std::string path = dir.write("small.dsk", sectors(4, 9));
    Image image;
    ASSERT_TRUE(image.openDiskImage(path));
    std::vector<nw_byte> buffer(512);
    EXPECT_EQ(image.readSectors(buffer.data(), 10, 1), -3);
    EXPECT_EQ(image.readSectors(buffer.data(), 1, 1), 0) << "the stream recovers from the failed read";
    EXPECT_EQ(buffer[0], std::uint8_t(9 + 3));
}

TEST_F(ImageTest, DiskChangedIsReportedOnceAfterOpening)
{
    std::string path = dir.write("disk.dsk", sectors(2, 0));
    Image image;
    ASSERT_TRUE(image.openDiskImage(path));
    EXPECT_TRUE(image.isDiskChanged());
    EXPECT_FALSE(image.isDiskChanged());
    ASSERT_TRUE(image.openDiskImage(path));
    EXPECT_TRUE(image.isDiskChanged());
}

TEST_F(ImageTest, APartitionIsReadFromItsStartSector)
{
    std::string path = dir.write("hd.dsk", hardDisk({{0x80, 10, 20}, {0x00, 40, 20}}, 64));
    Image image;
    ASSERT_TRUE(image.openPartitionImage(0, false, path));
    EXPECT_TRUE(image.isHarddiskPartition());
    EXPECT_EQ(image.getPartitionNr(), 0u);
    EXPECT_EQ(image.getPartitionOffset(), 10u * 512);
    EXPECT_EQ(firstByteOfSector0(image), 10);

    Image second;
    ASSERT_TRUE(second.openPartitionImage(1, false, path));
    EXPECT_EQ(firstByteOfSector0(second), 40);
}

TEST_F(ImageTest, ADisabledPartitionIsOnlyOpenedWhenTheBootflagIsIgnored)
{
    std::string path = dir.write("hd.dsk", hardDisk({{0x02, 10, 20}}, 32));
    Image image;
    EXPECT_FALSE(image.openPartitionImage(0, false, path));
    EXPECT_FALSE(image.hasDisk());
    Image ignoring;
    EXPECT_TRUE(ignoring.openPartitionImage(0, true, path));
    EXPECT_EQ(firstByteOfSector0(ignoring), 10);
}

TEST_F(ImageTest, AnEmptyPartitionEntryDoesNotOpen)
{
    std::string path = dir.write("hd.dsk", hardDisk({{0x80, 10, 20}}, 32));
    Image image;
    EXPECT_FALSE(image.openPartitionImage(1, false, path));
    EXPECT_FALSE(image.openPartitionImage(Image::MAX_PARTITIONS, false, path));
}

TEST_F(ImageTest, SetActivePartitionMovesTheOffsetAndReportsADiskChange)
{
    std::string path = dir.write("hd.dsk", hardDisk({{0x80, 10, 20}, {0x00, 40, 20}}, 64));
    Image image;
    ASSERT_TRUE(image.openPartitionImage(0, false, path));
    image.isDiskChanged();
    ASSERT_TRUE(image.SetActivePartition(1));
    EXPECT_TRUE(image.isDiskChanged());
    EXPECT_EQ(image.getPartitionNr(), 1u);
    EXPECT_EQ(firstByteOfSector0(image), 40);
    EXPECT_FALSE(image.SetActivePartition(5)) << "no such partition";
    EXPECT_EQ(firstByteOfSector0(image), 40) << "unchanged";
}

TEST_F(ImageTest, GetPartitionInfoDecodesTheFlags)
{
    std::vector<std::uint8_t> mbr = hardDisk({{0x81, 0x01020304, 0x0a0b0c0d}, {0x02, 5, 6}}, 1);
    PartitionInfo first;
    Image::GetPartitionInfo(mbr.data(), &first, 0);
    EXPECT_TRUE(first.bootable);
    EXPECT_TRUE(first.readonly);
    EXPECT_FALSE(first.disabled);
    EXPECT_EQ(first.startLBA, 0x01020304u);
    EXPECT_EQ(first.length, 0x0a0b0c0du);
    PartitionInfo second;
    Image::GetPartitionInfo(mbr.data(), &second, 1);
    EXPECT_TRUE(second.disabled);
    EXPECT_FALSE(second.bootable);
}

TEST_F(ImageTest, TheImageHandlerInsertsImagesAndTheRomdisk)
{
    std::string path = dir.write("disk.dsk", sectors(2, 0));
    ImageHandler handler;
    EXPECT_EQ(handler.insertDisk(path), 0);
    EXPECT_TRUE(handler.diskChanged());
    EXPECT_FALSE(handler.diskChanged());
    EXPECT_FALSE(handler.isRomdisk());
    EXPECT_NE(handler.insertDisk(dir.file("missing.dsk")), 0);
    EXPECT_EQ(handler.insertDisk("__romdisk__"), 0);
    EXPECT_TRUE(handler.isRomdisk());
}

namespace {

// a drive with an Image, so NowindHost::nowMap can see it is a hard disk partition
class ImageDrive : public nwhost::DiskHandler {
public:
    Image image;
    nwhost::SectorMedium* getSectorMedium() override { return &image; }
    int insertDisk(std::string filename) override { return image.openDiskImage(filename) ? 0 : -1; }
    bool diskChanged() override { return image.isDiskChanged(); }
    bool isRomdisk() override { return false; }
};

} // namespace

TEST_F(ImageTest, NowmapSwitchesThePartitionOfAHardDisk)
{
    std::string path = dir.write("hd.dsk", hardDisk({{0x80, 10, 20}, {0x00, 40, 20}}, 64));
    ImageDrive drive;
    ASSERT_TRUE(drive.image.openPartitionImage(0, false, path));
    std::vector<nwhost::DiskHandler*> drives{&drive};
    nwhost::NowindHost host(drives);
    host.initialize();

    std::string list = host.nowMap("/L");
    EXPECT_NE(list.find("Drive A, driveId 0 (" + path + "), partition 0"), std::string::npos) << list;
    EXPECT_EQ(host.nowMap("0 1"), "hdd of drive id 0 was set to partition 1\n");
    EXPECT_EQ(firstByteOfSector0(drive.image), 40);
    EXPECT_EQ(host.nowMap("0 7").substr(0, 33), "error: partition 7 does not exist");
}
