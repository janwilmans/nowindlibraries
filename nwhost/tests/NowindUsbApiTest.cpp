// NowindUsbApiTest.cpp, the nowindusb_* C API (nowindusb.h) that nwhostapp and emuv1 use; it works on globals

#include "nowindusb.h"
#include "nwhost.h"
#include "support/Fat12Image.h"
#include "support/TempDir.h"

#include "DiskHandler.hh"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace nwtest;
using nwhost::byte;

// defined in nowindusb.cpp
extern std::vector<nwhost::DiskHandler*> drives;

namespace {

std::vector<std::string> debugMessages;
void collectDebug(const char* message)
{
    debugMessages.push_back(message);
}

void sendCommand(byte a, byte b, byte c, byte command)
{
    for (byte value : {byte(0xaf), byte(0x05), c, b, byte(0), byte(0), byte(0), byte(0), byte(0), a, command})
    {
        nowindusb_write(value, 0);
    }
}

std::vector<byte> readReply()
{
    std::vector<byte> reply;
    while (nowindusb_is_data_available()) reply.push_back(nowindusb_read());
    return reply;
}

// one host for the whole suite: nowindusb_startup/cleanup work on globals
class NowindUsbApiTest : public ::testing::Test {
protected:
    static void SetUpTestSuite()
    {
        nowindusb_set_debug_callback(&collectDebug);
        nowindusb_startup();
    }
    static void TearDownTestSuite()
    {
        nowindusb_cleanup();
        nowindusb_set_debug_callback(nullptr);
    }
    TempDir dir;
};

} // namespace

TEST_F(NowindUsbApiTest, DebugMessagesArePrefixed)
{
    debugMessages.clear();
    nowindusb_set_image(0, dir.file("missing.dsk").c_str());
    ASSERT_FALSE(debugMessages.empty());
    EXPECT_EQ(debugMessages[0].rfind("[nwhost] ", 0), 0u) << debugMessages[0];
}

TEST_F(NowindUsbApiTest, SetImageInsertsADiskImage)
{
    Fat12Image floppy;
    std::string path = dir.write("disk.dsk", floppy.data);
    EXPECT_TRUE(nowindusb_set_image(0, path.c_str()));
    EXPECT_FALSE(nowindusb_set_image(0, dir.file("missing.dsk").c_str()));
}

TEST_F(NowindUsbApiTest, AtMost8Drives)
{
    Fat12Image floppy;
    std::string path = dir.write("disk.dsk", floppy.data);
    EXPECT_TRUE(nowindusb_set_image(7, path.c_str()));
    EXPECT_FALSE(nowindusb_set_image(8, path.c_str()));
    EXPECT_FALSE(nowindusb_set_romdisk(8));
}

TEST_F(NowindUsbApiTest, DrivesSeesTheDrivesAndAttributes)
{
    nowindusb_attribute(nwhost::enable_phantom_drives, true);
    nowindusb_attribute(nwhost::allow_other_diskroms, true);
    sendCommand(0, 0, 0, 0x85); // C_DRIVES
    std::vector<byte> reply = readReply();
    ASSERT_EQ(reply.size(), 6u);
    EXPECT_EQ(reply[3], 2) << "phantom drives";
    EXPECT_EQ(reply[4], 0) << "other diskroms allowed";
    EXPECT_EQ(reply[5], drives.size());
    nowindusb_attribute(nwhost::enable_phantom_drives, false);
    nowindusb_attribute(nwhost::allow_other_diskroms, false);
}

TEST_F(NowindUsbApiTest, EnableDos2IsUsedForGetDosVersion)
{
    nowindusb_attribute(nwhost::enable_dos2, true);
    sendCommand(1, 0, 0, 0x92); // C_GETDOSVERSION on an MSX2
    EXPECT_EQ(readReply().back(), 2);
    nowindusb_attribute(nwhost::enable_dos2, false);
    sendCommand(1, 0, 0, 0x92);
    EXPECT_EQ(readReply().back(), 1);
}

TEST_F(NowindUsbApiTest, RequestsAreQueuedWithTheirLengthByte)
{
    // the format is LL (length, counting itself) followed by the command bytes
    const byte request[] = {3, 0x0a, 0x0b};
    nowindusb_clear_startup_requests();
    nowindusb_add_startup_request(request);
    sendCommand(0, 0, 0, 0x93); // C_CMDREQUEST, startup, first
    EXPECT_EQ(readReply(), (std::vector<byte>{0xff, 0xaf, 0x05, 3, 0x0a, 0x0b}));
    nowindusb_clear_startup_requests();

    nowindusb_add_request(request);
    sendCommand(0, 1, 0, 0x93); // anytime
    EXPECT_EQ(readReply(), (std::vector<byte>{0xff, 0xaf, 0x05, 0, 3, 0x0a, 0x0b}));
    nowindusb_add_request(request);
    nowindusb_clear_requests();
    sendCommand(0, 1, 0, 0x93);
    EXPECT_EQ(readReply(), (std::vector<byte>{0xff, 0xaf, 0x05, 0}));
}

TEST_F(NowindUsbApiTest, HarddiskAutodetectInsertsTheBootablePartitionFirst)
{
    // partition number 0xFF: all partitions, the bootable one in the first drive
    std::vector<std::uint8_t> image(64 * 512);
    auto entry = [&](std::size_t p, std::uint8_t flags, std::uint8_t start) {
        image[0x1ee - p * 16] = flags;
        image[0x1ee - p * 16 + 8] = start;
        image[0x1ee - p * 16 + 12] = 10;
    };
    entry(0, 0x00, 10);
    entry(1, 0x80, 30);
    std::string path = dir.write("hd.dsk", image);
    EXPECT_EQ(nowindusb_set_harddisk_image(1, 0xff, false, path.c_str()), 2u);
    EXPECT_EQ(nowindusb_set_harddisk_image(3, 0, false, path.c_str()), 1u);
    EXPECT_EQ(nowindusb_set_harddisk_image(4, 5, false, path.c_str()), 0u) << "no such partition";
}

TEST(NowindUsbApiLifetime, CleanupForgetsTheDrives)
{
    // nowindusb_cleanup() used to delete the drives but keep the pointers, the next startup used them
    nowindusb_startup();
    nowindusb_set_romdisk(3); // drives 0..3
    nowindusb_cleanup();
    EXPECT_TRUE(drives.empty()) << drives.size() << " drive pointers left";

    nowindusb_startup();
    sendCommand(0, 0, 0, 0x85); // C_DRIVES
    std::vector<byte> reply = readReply();
    ASSERT_EQ(reply.size(), 6u);
    EXPECT_EQ(reply[5], 1) << "a fresh start has one drive";
    nowindusb_cleanup();
}
