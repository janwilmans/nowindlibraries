// DeviceTest.cpp, the BASIC NOW: device (msxsrc/device.asm) against the host's Device class, through NowindHost

#include "support/HostFixture.h"
#include "support/TempDir.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace nwtest;

namespace {

const word FCB = 0xe000;

// the FCB of DISK BASIC: +0 mode, +4 device, +6 position in the buffer, +9 the 256 byte buffer
class DeviceTest : public HostFixture {
protected:
    DeviceTest()
        : cwd(dir.path)
    {
        createHost();
    }

    // open (device.asm): D = device code, E = file mode, HL = FCB, then the 11 byte FILNAM ("NAME    EXT")
    void open(const std::string& name, byte mode, byte device = 0x24)
    {
        msx->sendCommand(Regs().setDE(device * 256 + mode).setHL(FCB), nwhost::C_DEVICEOPEN);
        std::string filnam = name;
        filnam.resize(11, ' ');
        msx->send(std::vector<byte>(filnam.begin(), filnam.end()));
    }

    struct Reply {
        byte error = 0xff;
        word address = 0;
        std::vector<byte> data;
    };

    // the ROM reads: A (error, 0 = ok), then address and length for an ldir
    Reply readReply()
    {
        Reply reply;
        EXPECT_TRUE(msx->readHeader());
        reply.error = msx->read();
        if (reply.error == 0)
        {
            reply.address = msx->read16();
            reply.data = msx->read(msx->read16());
        }
        return reply;
    }

    void write(char c) { msx->sendCommand(Regs().setBC(byte(c)).setHL(FCB), nwhost::C_DEVICEWRITE); }
    void close() { msx->sendCommand(Regs().setHL(FCB), nwhost::C_DEVICECLOSE); }

    TempDir dir;
    CurrentDirectory cwd;
};

std::vector<byte> bytes(const std::string& text)
{
    return std::vector<byte>(text.begin(), text.end());
}

} // namespace

TEST_F(DeviceTest, OpenForInputSendsTheFcbAndTheFirstBufferWithEof)
{
    dir.write("DATA.TXT", bytes("hello"));
    open("DATA    TXT", 1);
    Reply reply = readReply();
    ASSERT_EQ(reply.error, 0);
    EXPECT_EQ(reply.address, FCB);
    // mode, 0, 0, 0, device, 0, 0 (position), 0, 0, then the data and the end-of-file marker
    std::vector<byte> expected = {1, 0, 0, 0, 0x24, 0, 0, 0, 0, 'h', 'e', 'l', 'l', 'o', 0x1a};
    EXPECT_EQ(reply.data, expected);
    EXPECT_FALSE(msx->hasData());
}

TEST_F(DeviceTest, FileNamesAreUpperCased)
{
    // the MSX sends the name in capitals anyway; the host opens the upper case name
    dir.write("UP.TXT", bytes("x"));
    open("up      txt", 1);
    EXPECT_EQ(readReply().error, 0);
}

TEST_F(DeviceTest, ReadSendsTheNext256Bytes)
{
    std::string content(300, 'a');
    for (std::size_t i = 0; i < content.size(); ++i) content[i] = char('a' + i % 26);
    dir.write("LONG.TXT", bytes(content));
    open("LONG    TXT", 1);
    Reply first = readReply();
    ASSERT_EQ(first.data.size(), 9u + 256) << "a full buffer, no end-of-file yet";

    // read (device.asm), when the buffer position wraps: HL = FCB; the reply is a dummy byte, FCB + 9 and the data
    msx->sendCommand(Regs().setHL(FCB), nwhost::C_DEVICEREAD);
    Reply next = readReply();
    EXPECT_EQ(next.error, 0) << "the dummy byte";
    EXPECT_EQ(next.address, FCB + 9);
    std::vector<byte> rest(content.begin() + 256, content.end());
    rest.push_back(0x1a);
    EXPECT_EQ(next.data, rest);
}

TEST_F(DeviceTest, AFileOfExactly256BytesEndsWithAnEofOnlyRead)
{
    dir.write("EXACT.TXT", std::vector<byte>(256, 'z'));
    open("EXACT   TXT", 1);
    EXPECT_EQ(readReply().data.size(), 9u + 256);
    msx->sendCommand(Regs().setHL(FCB), nwhost::C_DEVICEREAD);
    EXPECT_EQ(readReply().data, std::vector<byte>{0x1a});
}

TEST_F(DeviceTest, OpenOfAMissingFileIsError53)
{
    open("MISSING TXT", 1);
    EXPECT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 53) << "file not found";
}

TEST_F(DeviceTest, OutputCreatesTheFileAndWritesCharacters)
{
    open("OUT     TXT", 2);
    Reply reply = readReply();
    ASSERT_EQ(reply.error, 0);
    EXPECT_EQ(reply.data, (std::vector<byte>{2, 0, 0, 0, 0x24, 0, 0, 0, 0})) << "no buffer for output";
    write('h');
    write('i');
    close();
    EXPECT_EQ(dir.read("OUT.TXT"), bytes("hi"));
}

TEST_F(DeviceTest, AppendAddsToTheFile)
{
    dir.write("LOG.TXT", bytes("ab"));
    open("LOG     TXT", 8);
    EXPECT_EQ(readReply().error, 0);
    write('c');
    close();
    EXPECT_EQ(dir.read("LOG.TXT"), bytes("abc"));
}

TEST_F(DeviceTest, RandomModeIsError58)
{
    open("ANY     TXT", 4);
    EXPECT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 58) << "sequential I/O only";
}

TEST_F(DeviceTest, AnUnknownModeIsAnError)
{
    open("ANY     TXT", 3);
    EXPECT_TRUE(msx->readHeader());
    EXPECT_NE(msx->read(), 0);
}

TEST_F(DeviceTest, WritesAfterCloseAreIgnored)
{
    open("OUT     TXT", 2);
    readReply();
    write('a');
    close();
    write('b');
    EXPECT_EQ(dir.read("OUT.TXT"), bytes("a"));
}

TEST_F(DeviceTest, DevicesAreKeptApartByTheirFcb)
{
    open("ONE     TXT", 2);
    readReply();
    msx->sendCommand(Regs().setDE(0x2402).setHL(FCB + 0x200), nwhost::C_DEVICEOPEN);
    std::string filnam = "TWO     TXT";
    msx->send(std::vector<byte>(filnam.begin(), filnam.end()));
    readReply();
    write('1');
    msx->sendCommand(Regs().setBC('2').setHL(FCB + 0x200), nwhost::C_DEVICEWRITE);
    close();
    msx->sendCommand(Regs().setHL(FCB + 0x200), nwhost::C_DEVICECLOSE);
    EXPECT_EQ(dir.read("ONE.TXT"), bytes("1"));
    EXPECT_EQ(dir.read("TWO.TXT"), bytes("2"));
}

TEST_F(DeviceTest, AnMsxResetClosesTheDevices)
{
    open("OUT     TXT", 2);
    readReply();
    write('a');
    msx->send({0xaf, 0xff}); // INIHRD
    write('b');
    EXPECT_EQ(dir.read("OUT.TXT"), bytes("a"));
}
