// BDOSProxyTest.cpp, smoke tests for the experimental BDOS proxy (BDOS_NOWIND in msxsrc/nowind.asm, off by default)
// It serves files from the host's current directory.

#include "support/HostFixture.h"
#include "support/RomModel.h"
#include "support/TempDir.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace nwtest;

namespace {

class BDOSProxyTest : public HostFixture {
protected:
    BDOSProxyTest()
        : cwd(dir.path)
    {
        createHost();
    }

    RomModel rom()
    {
        return RomModel(UsbPort{[this] { return msx->read(); }, [this](byte value) { msx->send(value); }});
    }

    // the BDOS function in C, DE = FCB, HL = transfer address, then the 37 byte FCB
    void sendFcbCommand(byte function, const std::string& name11, word de, word hl)
    {
        msx->sendCommand(Regs().setBC(function).setDE(de).setHL(hl), function);
        std::vector<byte> fcb(37, 0);
        std::copy(name11.begin(), name11.end(), fcb.begin() + 1);
        msx->send(fcb);
    }

    TempDir dir;
    CurrentDirectory cwd;
};

} // namespace

TEST_F(BDOSProxyTest, OpenFileReturnsTheFcbWithTheFileSize)
{
    dir.write("GAME.BIN", std::vector<std::uint8_t>(1234, 1));
    sendFcbCommand(0x0f, "GAME    BIN", 0xd000, 0);
    RomModel model = rom();
    RomResult result = model.blockRead(0xd000);
    EXPECT_EQ(result.code, nwhost::BlockRead::BLOCKREAD_EXIT);
    std::vector<byte> fcb = model.memory.range(0xd000, 37);
    EXPECT_EQ(std::string(fcb.begin() + 1, fcb.begin() + 12), "GAME    BIN");
    EXPECT_EQ(fcb[0x0e] + 256 * fcb[0x0f], 512) << "record size";
    EXPECT_EQ(fcb[0x10] + 256 * fcb[0x11] + 65536 * fcb[0x12], 1234);
}

TEST_F(BDOSProxyTest, OpenFileOfAMissingFileIsAnError)
{
    sendFcbCommand(0x0f, "NOPE    BIN", 0xd000, 0);
    RomModel model = rom();
    RomResult result = model.blockRead(0xd000);
    EXPECT_EQ(result.code, nwhost::BlockRead::BLOCKREAD_ERROR);
}

TEST_F(BDOSProxyTest, FindFirstReturnsADirectoryEntry)
{
    dir.write("only.txt", std::vector<std::uint8_t>(300, 2));
    sendFcbCommand(0x11, "???????????", 0xd000, 0xd100);
    RomModel model = rom();
    RomResult result = model.blockRead(0xd100);
    EXPECT_EQ(result.code, nwhost::BlockRead::BLOCKREAD_EXIT);
    std::vector<byte> entry = model.memory.range(0xd100, 36);
    EXPECT_EQ(std::string(entry.begin() + 1, entry.begin() + 12), "ONLY    TXT");
    EXPECT_EQ(entry[0x1d] + 256 * entry[0x1e], 300);

    // FindNext: no more files
    msx->sendCommand(Regs().setBC(0x12).setHL(0xd100), 0x12);
    RomResult next = model.blockRead(0xd100);
    EXPECT_EQ(next.code, nwhost::BlockRead::BLOCKREAD_ERROR);
}
