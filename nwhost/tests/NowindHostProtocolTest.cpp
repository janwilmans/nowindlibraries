// NowindHostProtocolTest.cpp, the NowindHost state machine and its commands, as the ROM (msxsrc) uses them

#include "support/Fat12Image.h"
#include "support/HostFixture.h"
#include "support/RomModel.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace nwtest;
using nwhost::NowindHost;

namespace {

const byte CARRY = 1;

class NowindHostProtocolTest : public HostFixture {
protected:
    // the ROM's side of block transfers, talking to the host
    RomModel rom()
    {
        return RomModel(UsbPort{[this] { return msx->read(); }, [this](byte value) { msx->send(value); }});
    }

    std::vector<byte> pattern(unsigned size, byte seed = 1)
    {
        std::vector<byte> data(size);
        for (unsigned i = 0; i < size; ++i) data[i] = byte(seed + i * 7 + (i >> 9));
        return data;
    }

    void fillSectors(MemoryDisk& target, unsigned first, const std::vector<byte>& data)
    {
        std::copy(data.begin(), data.end(), target.sector(first));
    }

    // DSKIO read (nowindDriver.asm: sendRegisters, ld (hl),C_DSKIO, jp nc,blockRead)
    RomResult dskioRead(RomModel& model, byte drive, byte sectors, word sector, word address, byte media = 0xf9)
    {
        msx->sendCommand(Regs().setA(drive).setBC(sectors * 256 + media).setDE(sector).setHL(address), nwhost::C_DSKIO);
        return model.blockRead(address);
    }

    RomResult dskioWrite(RomModel& model, byte drive, byte sectors, word sector, word address, byte media = 0xf9)
    {
        msx->sendCommand(Regs().setA(drive).setF(CARRY).setBC(sectors * 256 + media).setDE(sector).setHL(address),
                         nwhost::C_DSKIO);
        return model.blockWrite(address);
    }
};

} // namespace

// --- synchronisation -------------------------------------------------------------------------------------------

TEST_F(NowindHostProtocolTest, BytesBeforeTheAF05HeaderAreIgnored)
{
    createHost();
    msx->send({0x00, 0x12, 0xaf, 0x00, 0x05});
    EXPECT_FALSE(msx->hasData());
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 1);
}

TEST_F(NowindHostProtocolTest, AFAF05StillSynchronises)
{
    createHost();
    msx->send(0xaf);
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION); // AF AF 05 ...
    EXPECT_TRUE(msx->readHeader());
}

TEST_F(NowindHostProtocolTest, AFFFIsAnMsxResetAndReturnsToSync)
{
    // INIHRD (nowindDriver.asm) sends AF FF
    createHost();
    msx->send({0xaf, 0xff});
    EXPECT_FALSE(msx->hasData());
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    EXPECT_TRUE(msx->readHeader());
}

TEST_F(NowindHostProtocolTest, AnUnknownCommandGetsNoReplyAndReturnsToSync)
{
    createHost();
    msx->sendCommand(Regs(), 0x42);
    EXPECT_FALSE(msx->hasData());
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    EXPECT_TRUE(msx->readHeader());
}

TEST_F(NowindHostProtocolTest, AHalfReceivedCommandTimesOutAfter500ms)
{
    createHost();
    msx->send({0xaf, 0x05, 1, 2, 3}); // 3 of the 9 register/command bytes
    msx->advance(600);
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 1);
}

TEST_F(NowindHostProtocolTest, ATimeoutPurgesAnUnreadReply)
{
    // a DSKIO read waits for the acks of its blocks; when the MSX gives up, the next command must not see the old data
    createHost();
    msx->sendCommand(Regs().setA(0).setBC(0x01f9).setDE(0).setHL(0xc000), nwhost::C_DSKIO);
    EXPECT_TRUE(msx->hasData());
    msx->advance(600);
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 1);
    EXPECT_FALSE(msx->hasData());
}

// --- DRIVES / INIENV -------------------------------------------------------------------------------------------

TEST_F(NowindHostProtocolTest, DrivesRepliesFlagsAndTheNumberOfDrives)
{
    // drivesCommand (nowindDriver.asm): LASTDRV (phantom drives), DEVICE (allow other diskroms), number of drives
    createHost(3);
    msx->sendCommand(Regs().setA(2), nwhost::C_DRIVES); // 2 drives already installed by other diskroms
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 0) << "no phantom drives";
    EXPECT_EQ(msx->read(), 2 | 0x80) << "other diskroms not allowed";
    EXPECT_EQ(msx->read(), 3);
    EXPECT_FALSE(msx->hasData());
}

TEST_F(NowindHostProtocolTest, DrivesReportsTheAttributes)
{
    createHost(1);
    host->setEnablePhantomDrives(true);
    host->setAllowOtherDiskroms(true);
    msx->sendCommand(Regs().setA(1), nwhost::C_DRIVES);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 2);
    EXPECT_EQ(msx->read(), 1);
    EXPECT_EQ(msx->read(), 1);
}

TEST_F(NowindHostProtocolTest, DrivesReportsAtLeastOneDrive)
{
    // MSXDOS1 cannot handle 0 drives
    createHost(0);
    msx->sendCommand(Regs(), nwhost::C_DRIVES);
    ASSERT_TRUE(msx->readHeader());
    msx->read(2);
    EXPECT_EQ(msx->read(), 1);
}

TEST_F(NowindHostProtocolTest, InienvRepliesTheRomdiskDriveFoundByDrives)
{
    addDrive();
    addDrive().romdisk = true;
    startHost();
    msx->sendCommand(Regs(), nwhost::C_INIENV);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 255) << "no romdisk known before DRIVES";

    msx->sendCommand(Regs(), nwhost::C_DRIVES);
    msx->readAll();
    msx->sendCommand(Regs(), nwhost::C_INIENV);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 1);
}

// --- DSKIO read --------------------------------------------------------------------------------------------------

TEST_F(NowindHostProtocolTest, DskioReadsSectorsIntoMemory)
{
    createHost();
    std::vector<byte> data = pattern(3 * 512);
    fillSectors(disk(), 10, data);
    RomModel model = rom();
    RomResult result = dskioRead(model, 0, 3, 10, 0xc000);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, nwhost::BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(model.memory.range(0xc000, 3 * 512), data);
    EXPECT_FALSE(msx->hasData());
}

TEST_F(NowindHostProtocolTest, DskioReadsAcrossThePage1And2Boundary)
{
    createHost();
    std::vector<byte> data = pattern(4 * 512);
    fillSectors(disk(), 0, data);
    RomModel model = rom();
    RomResult result = dskioRead(model, 0, 4, 0, 0x7c00);
    EXPECT_EQ(result.code, nwhost::BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(model.memory.range(0x7c00, 4 * 512), data);
}

TEST_F(NowindHostProtocolTest, DskioReadsTheSecondDrive)
{
    createHost(2);
    std::vector<byte> data = pattern(512, 99);
    fillSectors(disk(1), 5, data);
    RomModel model = rom();
    dskioRead(model, 1, 1, 5, 0xc000);
    EXPECT_EQ(model.memory.range(0xc000, 512), data);
}

TEST_F(NowindHostProtocolTest, DskioUsesRegisterCAsSectorBits16To23BelowMedia0x80)
{
    // FAT16 partitions: the ROM passes the high bits of the sector number in C instead of the media descriptor
    createHost();
    msx->sendCommand(Regs().setA(0).setBC(0x0102).setDE(0x0304).setHL(0xc000), nwhost::C_DSKIO);
    EXPECT_EQ(disk().lastReadSector, 0x020304u);
    msx->sendCommand(Regs().setA(0).setBC(0x01f9).setDE(0x0304).setHL(0xc000), nwhost::C_DSKIO);
    EXPECT_EQ(disk().lastReadSector, 0x0304u);
}

TEST_F(NowindHostProtocolTest, DskioReadOfZeroSectorsEndsAtOnce)
{
    // a slow transfer of 0 bytes would make the ROM's ldir copy 64 kB
    createHost();
    RomModel model = rom();
    RomResult result = dskioRead(model, 0, 0, 0, 0xc000);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, nwhost::BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(result.slowBlocks + result.fastBlocks, 0u);
}

TEST_F(NowindHostProtocolTest, DskioOfAMissingDriveTimesOut)
{
    // no reply: the ROM times out in getHeader and DSKIO returns 'not ready'
    createHost(1);
    RomModel model = rom();
    RomResult result = dskioRead(model, 3, 1, 0, 0xc000);
    EXPECT_TRUE(result.timeout);
}

// DISABLED: for a missing drive (or no disk) executeCommand() returns from the C_DSKIO case without setState(), so
// the host stays in STATE_EXECUTING_COMMAND and runs DSKIO again for every byte it receives, until 500 ms without
// data reset it to STATE_SYNC1. The ROM's getHeader timeout is about 0.7 s at 3.58 MHz (common.asm), but half of
// that on a turbo R (R800), so the next command arrives before the host's 500 ms and is swallowed.
// Host: nwhost/NowindHost.cc executeCommand() 'case C_DSKIO' ('return;'). ROM: msxsrc/common.asm getHeaderInPage0.
TEST_F(NowindHostProtocolTest, DISABLED_TheCommandAfterADskioOfAMissingDriveIsHandled)
{
    createHost(1);
    RomModel model = rom();
    EXPECT_TRUE(dskioRead(model, 3, 1, 0, 0xc000).timeout);
    msx->advance(350); // the ROM's timeout on an R800
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 1);
}

TEST_F(NowindHostProtocolTest, DskioReadErrorTimesOut)
{
    createHost();
    disk().failReads = true;
    RomModel model = rom();
    RomResult result = dskioRead(model, 0, 1, 0, 0xc000);
    EXPECT_TRUE(result.timeout);
}

// --- DSKIO write (USE_OLD_DISKWRITE: doDiskWrite1/2) ----------------------------------------------------------

TEST_F(NowindHostProtocolTest, DskioWritesMemoryToSectors)
{
    createHost();
    RomModel model = rom();
    std::vector<byte> data = pattern(2 * 512, 3);
    std::copy(data.begin(), data.end(), model.memory.bytes.begin() + 0xc000);
    RomResult result = dskioWrite(model, 0, 2, 20, 0xc000);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, 255) << "done";
    EXPECT_EQ(std::vector<byte>(disk().sector(20), disk().sector(22)), data);
    EXPECT_FALSE(msx->hasData());
}

TEST_F(NowindHostProtocolTest, DskioWritesAcrossThePage1And2Boundary)
{
    // the host limits blocks to page 1, then sends 254 (more data for page 2/3), the ROM goes on in blockWrite23
    createHost();
    RomModel model = rom();
    std::vector<byte> data = pattern(4 * 512, 5);
    std::copy(data.begin(), data.end(), model.memory.bytes.begin() + 0x7c00);
    RomResult result = dskioWrite(model, 0, 4, 0, 0x7c00);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, 255);
    EXPECT_EQ(std::vector<byte>(disk().sector(0), disk().sector(4)), data);
}

TEST_F(NowindHostProtocolTest, DskioWriteToAWriteProtectedDiskIsError0)
{
    // blockWrite01: code 1..127 is an error, the next byte is the DSKIO error code, 0 = write protected
    createHost();
    disk().writeProtected = true;
    RomModel model = rom();
    RomResult result = dskioWrite(model, 0, 1, 0, 0xc000);
    EXPECT_FALSE(result.timeout);
    EXPECT_TRUE(RomModel::isWriteError(result.code));
    EXPECT_EQ(model.lastErrorCode, 0);
    EXPECT_EQ(disk().writeCount, 0u);
}

TEST_F(NowindHostProtocolTest, DskioWriteFailureIsWriteFault)
{
    createHost();
    disk().failWrites = true;
    RomModel model = rom();
    RomResult result = dskioWrite(model, 0, 1, 0, 0xc000);
    EXPECT_TRUE(RomModel::isWriteError(result.code));
    EXPECT_EQ(model.lastErrorCode, 10) << "write fault";
}

// --- DSKCHG / GETDPB ---------------------------------------------------------------------------------------------

TEST_F(NowindHostProtocolTest, DskchgRepliesChangedAndTheNewMediaDescriptor)
{
    // dskchgCommand (nowindDriver.asm): A = changed (0 = no), C = media descriptor
    createHost();
    disk().sector(1)[0] = 0xf9; // first byte of the FAT
    msx->sendCommand(Regs(), nwhost::C_DSKCHG);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 255);
    EXPECT_EQ(msx->read(), 0xf9);

    msx->sendCommand(Regs(), nwhost::C_DSKCHG);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read(), 0) << "not changed";
    EXPECT_EQ(msx->read(), 255);
}

TEST_F(NowindHostProtocolTest, DskchgOfAMissingDriveGetsNoReply)
{
    createHost(1);
    msx->sendCommand(Regs().setA(5), nwhost::C_DSKCHG);
    EXPECT_FALSE(msx->hasData());
}

TEST_F(NowindHostProtocolTest, GetdpbRepliesTheDestinationAndTheDpbOf720k)
{
    // getdpbCommand (nowindDriver.asm): DE = destination, then 18 bytes are copied with ldir
    createHost();
    Fat12Image::writeBootSector(disk().sector(0), format720k());
    msx->sendCommand(Regs().setDE(0xf000), nwhost::C_GETDPB);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->read16(), 0xf000u);
    std::vector<byte> dpb = msx->read(18);
    EXPECT_FALSE(msx->hasData());
    std::vector<byte> expected = {
        0xf9,       // media
        0x00, 0x02, // sector size 512
        15, 4,      // DIRMSK, DIRSHFT
        1, 2,       // CLUSMSK, CLUSSHFT
        1, 0,       // first FAT sector
        2,          // FAT count
        112,        // root directory entries
        14, 0,      // first data sector
        0xca, 0x02, // highest cluster number (1440 - 14) / 2 + 1 = 714
        3,          // sectors per FAT
        7, 0,       // first directory sector
    };
    EXPECT_EQ(dpb, expected);
}

TEST_F(NowindHostProtocolTest, GetdpbOfAnInvalidBootSectorGetsNoReply)
{
    createHost();
    msx->sendCommand(Regs().setDE(0xf000), nwhost::C_GETDPB); // all zero boot sector
    EXPECT_FALSE(msx->hasData());
}

// --- GETDOSVERSION / CMDREQUEST ------------------------------------------------------------------------------------

TEST_F(NowindHostProtocolTest, GetDosVersionPicksDos2OnlyOnMsx2And2PlusWhenEnabled)
{
    // init.asm: A = 1 boots DOS1, anything else DOS2; A = IDBYTE_2D (0 MSX1, 1 MSX2, 2 MSX2+, 3 turbo R)
    createHost();
    for (bool dos2 : {false, true})
    {
        host->setEnableMSXDOS2(dos2);
        for (byte msxVersion = 0; msxVersion < 4; ++msxVersion)
        {
            msx->sendCommand(Regs().setA(msxVersion), nwhost::C_GETDOSVERSION);
            ASSERT_TRUE(msx->readHeader());
            byte expected = (dos2 && (msxVersion == 1 || msxVersion == 2)) ? 2 : 1;
            EXPECT_EQ(msx->read(), expected) << "msx version " << int(msxVersion) << ", dos2 " << dos2;
        }
    }
}

TEST_F(NowindHostProtocolTest, StartupCommandRequestsReplayTheStartupQueue)
{
    // getBootArgs (init.asm): B = 0, C = 0 for the first request, C = 1 for the next ones; a reply of 0 ends it.
    // The host queues a {1, 0} request by default (NowindHost constructor, 'test for requestWait').
    createHost();
    host->addStartupRequest({3, 0x10, 0x20});
    msx->sendCommand(Regs().setBC(0x0000), nwhost::C_CMDREQUEST);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->readAll(), (std::vector<byte>{1, 0}));
    msx->sendCommand(Regs().setBC(0x0001), nwhost::C_CMDREQUEST);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->readAll(), (std::vector<byte>{3, 0x10, 0x20}));
    msx->sendCommand(Regs().setBC(0x0001), nwhost::C_CMDREQUEST);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->readAll(), std::vector<byte>{0});

    // the next boot starts again (C = 0), the queue is kept
    msx->sendCommand(Regs().setBC(0x0000), nwhost::C_CMDREQUEST);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->readAll(), (std::vector<byte>{1, 0}));
}

TEST_F(NowindHostProtocolTest, ClearedStartupQueueRepliesNoCommand)
{
    createHost();
    host->clearStartupRequests();
    msx->sendCommand(Regs().setBC(0x0000), nwhost::C_CMDREQUEST);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->readAll(), std::vector<byte>{0});
}

TEST_F(NowindHostProtocolTest, AnytimeCommandRequestsAreSentOnce)
{
    // B = 1; the ROM does not send these (only getBootArgs uses C_CMDREQUEST), this documents the host side
    createHost();
    host->addRequest({2, 0xaa});
    host->addRequest({2, 0xbb});
    msx->sendCommand(Regs().setBC(0x0100), nwhost::C_CMDREQUEST);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->readAll(), (std::vector<byte>{1, 2, 0xaa})) << "1 = more requests waiting";
    msx->sendCommand(Regs().setBC(0x0100), nwhost::C_CMDREQUEST);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->readAll(), (std::vector<byte>{0, 2, 0xbb}));
    msx->sendCommand(Regs().setBC(0x0100), nwhost::C_CMDREQUEST);
    ASSERT_TRUE(msx->readHeader());
    EXPECT_EQ(msx->readAll(), std::vector<byte>{0});
}

// --- CHANGEIMAGE / MESSAGE / CPUINFO / STDOUT ----------------------------------------------------------------------

TEST_F(NowindHostProtocolTest, ChangeImageInsertsTheQuotedFilenameUpToTheColon)
{
    // changeImage (nowindDriver.asm) sends the CALL IMAGE text up to and including ':' or 0
    createHost(2);
    msx->sendCommand(Regs().setA(1), nwhost::C_CHANGEIMAGE);
    std::string text = "(\"games.dsk\"):";
    msx->send(std::vector<byte>(text.begin(), text.end()));
    ASSERT_EQ(disk(1).insertedFilenames.size(), 1u);
    EXPECT_EQ(disk(1).insertedFilenames[0], "games.dsk");
    EXPECT_TRUE(disk(0).insertedFilenames.empty());

    msx->sendCommand(Regs().setA(0), nwhost::C_CHANGEIMAGE);
    text = "\"next.dsk\"";
    msx->send(std::vector<byte>(text.begin(), text.end()));
    msx->send(0);
    ASSERT_EQ(disk(0).insertedFilenames.size(), 1u);
    EXPECT_EQ(disk(0).insertedFilenames[0], "next.dsk");
}

TEST_F(NowindHostProtocolTest, ChangeImageForAMissingDriveIsIgnored)
{
    createHost(1);
    msx->sendCommand(Regs().setA(4), nwhost::C_CHANGEIMAGE);
    std::string text = "\"x.dsk\"";
    msx->send(std::vector<byte>(text.begin(), text.end()));
    msx->send(0);
    EXPECT_TRUE(disk(0).insertedFilenames.empty());
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    EXPECT_TRUE(msx->readHeader()) << "back in sync";
}

TEST_F(NowindHostProtocolTest, MessageCpuInfoAndStdoutGetNoReplyAndReturnToSync)
{
    createHost();
    msx->sendCommand(Regs(), nwhost::C_MESSAGE); // sendMessage (common.asm): text up to 0
    std::string text = "hello";
    msx->send(std::vector<byte>(text.begin(), text.end()));
    msx->send(0);

    msx->sendCommand(Regs(), nwhost::C_CPUINFO); // sendCpuInfo: ix, iy, sp, slots (5 bytes), 34 stack bytes
    msx->send(std::vector<byte>(2 + 2 + 2 + 5 + 34, 0x11));

    msx->sendCommand(Regs().setA('x'), nwhost::C_STDOUT);
    msx->sendCommand(Regs().setA('\r'), nwhost::C_STDOUT);

    EXPECT_FALSE(msx->hasData());
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    EXPECT_TRUE(msx->readHeader());
}

// --- C_COMMAND, API_NOWMAP (extendedBios.asm sendCommandWithData + readData, msxdostools/nowmap.asm) -------------

namespace {

class NowMapTest : public NowindHostProtocolTest {
protected:
    // EXTBIO function 4: C = command, B = length, A = 4 (the function, ld a,e), DE = IY = transfer address,
    // then B bytes; function 6 reads the reply with blockRead
    std::string nowmap(const std::string& arguments, RomResult* result = nullptr)
    {
        msx->sendCommand(Regs().setA(4).setBC(byte(arguments.size()) * 256 + NowindHost::API_NOWMAP).setDE(0xc000),
                         nwhost::C_COMMAND);
        msx->send(std::vector<byte>(arguments.begin(), arguments.end()));
        RomModel model = rom();
        RomResult r = model.blockRead(0xc000);
        if (result) *result = r;
        return std::string(reinterpret_cast<const char*>(&model.memory.bytes[0xc000]));
    }
};

} // namespace

TEST_F(NowMapTest, ListShowsTheDrives)
{
    createHost(2);
    RomResult result;
    std::string reply = nowmap("/L", &result);
    EXPECT_FALSE(result.timeout);
    EXPECT_EQ(result.code, nwhost::BlockRead::BLOCKREAD_EXIT);
    EXPECT_EQ(reply, "Nowind Map v1.2\r\nDrive A, driveId 0\r\nDrive B, driveId 1\r\n\r\n");
}

TEST_F(NowMapTest, ListCountsTheDrivesOfOtherDiskroms)
{
    createHost(1);
    msx->sendCommand(Regs().setA(1), nwhost::C_DRIVES); // one drive of another diskrom
    msx->readAll();
    EXPECT_EQ(nowmap("/l"), "Nowind Map v1.2\r\nDrive A: DiskRom\r\nDrive B, driveId 0\r\n\r\n");
}

TEST_F(NowMapTest, WrongArgumentsAreReported)
{
    createHost(1);
    EXPECT_EQ(nowmap("1").substr(0, 30), "error: no 2 arguments found\nus");
    EXPECT_EQ(nowmap("3 1").substr(0, 24), "error: drive_id invalid!");
    EXPECT_EQ(nowmap("0 1").substr(0, 25), "error: drive is not a hdd");
}

TEST_F(NowMapTest, TheByteAfterTheArgumentsFromNowmapComIsIgnored)
{
    // nowmap.asm sends B = CMD_LENGTH bytes from CMD_LINE (0x82, after the leading space), so one byte past the
    // arguments: the end of the MSX-DOS command line
    createHost(1);
    EXPECT_EQ(nowmap(std::string("0 1\0", 4)).substr(0, 25), "error: drive is not a hdd");
    EXPECT_EQ(nowmap("0 1\r").substr(0, 25), "error: drive is not a hdd");
    EXPECT_EQ(nowmap(std::string("/l\0", 3)).substr(0, 15), "Nowind Map v1.2");
}

TEST_F(NowMapTest, MoreThan240BytesIsRejected)
{
    // EXTBIO sendCommandWithData allows at most 240 bytes (the hardware buffer)
    createHost(1);
    std::string tooLong(241, 'x');
    msx->sendCommand(Regs().setA(4).setBC(byte(241) * 256 + NowindHost::API_NOWMAP).setDE(0xc000), nwhost::C_COMMAND);
    msx->send(std::vector<byte>(tooLong.begin(), tooLong.end()));
    EXPECT_FALSE(msx->hasData());
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    EXPECT_TRUE(msx->readHeader());
}

TEST_F(NowMapTest, OnlyExtbioFunction4IsAccepted)
{
    createHost(1);
    msx->sendCommand(Regs().setA(3).setBC(0x0200 + NowindHost::API_NOWMAP).setDE(0xc000), nwhost::C_COMMAND);
    EXPECT_FALSE(msx->hasData());
    msx->sendCommand(Regs().setA(1), nwhost::C_GETDOSVERSION);
    EXPECT_TRUE(msx->readHeader());
}
