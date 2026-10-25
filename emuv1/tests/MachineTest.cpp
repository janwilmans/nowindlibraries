// MachineTest.cpp, the MSX2 memory map of emuv1 (SlotSelector::configure(2)): main ROM, sub ROM, memory mapper and
// the nowind interface with the ROM assembled from msxsrc. One configured machine for all tests (singletons).

#include "Debug.h"
#include "cpu/Z80.h"
#include "devices/SlotSelector.h"
#include "memory/Mapper.h"
#include "memory/NowindInterface.h"
#include "support/Z80TestAccess.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

const std::string STAGE = EMUV1_STAGE; // emu/, roms/, msxsrc/nowind.rom, disks/dos2.dsk

std::vector<nw_byte> readFile(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    return std::vector<nw_byte>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

class MachineTest : public ::testing::Test {
protected:
    static void SetUpTestSuite()
    {
        static bool configured = false;
        if (configured) return;
        configured = true;
        // the ROMs are loaded relative to Debug::getPath(), the disk image relative to the current directory
        std::filesystem::current_path(STAGE + "/emu");
        Debug::Instance()->setPath(STAGE + "/emu/");
        SlotSelector::Instance()->configure(2);
        Z80TestAccess::useSlotSelector();
    }

    // slot per page, like port A8: page 0 in bits 0-1 ... page 3 in bits 6-7
    static void selectSlots(nw_byte primary, nw_byte secondary)
    {
        SlotSelector* slots = SlotSelector::Instance();
        // 0xFFFF selects the sub slots of the slot in page 3, so select that slot in page 3 first
        slots->setMainSlot(nw_byte((primary & 0x3f) | 0xc0));
        slots->setSubSlot(secondary);
        slots->setMainSlot(nw_byte((primary & 0x3f) | 0x80));
        slots->setSubSlot(secondary);
        slots->setMainSlot(primary);
    }

    nw_byte read(nw_word address) { return Z80::Instance()->readMemPublic(address); }
    void write(nw_word address, nw_byte value) { Z80::Instance()->writeMemPublic(address, value); }

    std::vector<nw_byte> read(nw_word address, unsigned count)
    {
        std::vector<nw_byte> result;
        for (unsigned i = 0; i < count; ++i) result.push_back(read(nw_word(address + i)));
        return result;
    }

    std::vector<nw_byte> mainRom = readFile(STAGE + "/roms/MSX2.ROM");
    std::vector<nw_byte> subRom = readFile(STAGE + "/roms/MSX2EXT.ROM");
    std::vector<nw_byte> nowindRom = readFile(STAGE + "/msxsrc/nowind.rom");
};

const nw_byte SLOT2 = 0xaa; // slot 2 in all pages
// AF 05, the registers (c b e d l h f a = 1: MSX2), C_GETDOSVERSION
const nw_byte GETDOSVERSION_FRAME[] = {0xaf, 0x05, 0, 0, 0, 0, 0, 0, 0, 1, 0x92};
const nw_byte SLOT3 = 0xff;

} // namespace

TEST_F(MachineTest, TheMainRomIsInSlot0)
{
    ASSERT_EQ(mainRom.size(), 0x8000u);
    selectSlots(0x00, 0x00);
    EXPECT_EQ(read(0x0000, 16), std::vector<nw_byte>(mainRom.begin(), mainRom.begin() + 16));
    EXPECT_EQ(read(0x7ff0, 16), std::vector<nw_byte>(mainRom.end() - 16, mainRom.end()));
}

TEST_F(MachineTest, TheSubRomIsInSlot3_0)
{
    selectSlots(SLOT3, 0x00);
    EXPECT_EQ(read(0x0000, 16), std::vector<nw_byte>(subRom.begin(), subRom.begin() + 16));
    EXPECT_EQ(read(0x0000), 'C') << "sub ROM header CD";
}

TEST_F(MachineTest, TheMapperIsRamInSlot3_2)
{
    selectSlots(SLOT3, 0xaa);
    write(0x8000, 0x12);
    write(0xc000, 0x34);
    EXPECT_EQ(read(0x8000), 0x12);
    EXPECT_EQ(read(0xc000), 0x34);
}

TEST_F(MachineTest, MapperBanksCanBeSwitchedPerPage)
{
    selectSlots(SLOT3, 0xaa);
    Mapper* mapper = Z80TestAccess::mapper();
    mapper->setPage(2, 5);
    write(0x8000, 0x55);
    mapper->setPage(1, 5);
    EXPECT_EQ(read(0x4000), 0x55) << "the same bank in page 1";
    mapper->setPage(2, 6);
    EXPECT_NE(read(0x8000), 0x55);
    mapper->setPage(2, 64 + 5);
    EXPECT_EQ(read(0x8000), 0x55) << "64 banks, the bank number wraps";
    EXPECT_EQ(mapper->getPage(2), 0xc5) << "like reading port FE: the bits above 64 banks read as 1";
    mapper->setPage(1, 2);
    mapper->setPage(2, 1);
}

TEST_F(MachineTest, ReadingFFFFInAnExpandedSlotGivesTheInvertedSubSlotRegister)
{
    selectSlots(SLOT3, 0xaa);
    EXPECT_EQ(read(0xffff), 0x55);
    write(0xffff, 0x00); // sub slot 3-0 in all pages
    EXPECT_EQ(read(0xffff), 0xff);
    EXPECT_EQ(read(0x0000), subRom[0]);
    write(0xffff, 0xaa);
}

TEST_F(MachineTest, TheNowindRomBank0IsInPage1OfSlot2_0)
{
    ASSERT_EQ(nowindRom.size(), 512u * 1024);
    selectSlots(SLOT2, 0x00);
    write(0x6001, 0);
    EXPECT_EQ(read(0x4000, 16), std::vector<nw_byte>(nowindRom.begin(), nowindRom.begin() + 16));
    EXPECT_EQ(read(0x4000), 'A');
    EXPECT_EQ(read(0x4001), 'B');
}

TEST_F(MachineTest, OddAddressesIn6000To7FFFSwitchTheRomBank)
{
    // the nowind v2 (sunrise) mapper responds to odd addresses only; 32 banks of 16 kB
    selectSlots(SLOT2, 0x00);
    write(0x6001, 5);
    EXPECT_EQ(read(0x4000, 16), std::vector<nw_byte>(nowindRom.begin() + 5 * 0x4000, nowindRom.begin() + 5 * 0x4000 + 16));
    write(0x6000, 3); // even: ignored
    EXPECT_EQ(read(0x4000), nowindRom[5 * 0x4000]);
    write(0x7fff, 0x20 + 4); // masked to 32 banks
    EXPECT_EQ(read(0x4100), nowindRom[4 * 0x4000 + 0x100]);
    write(0x6001, 0);
}

TEST_F(MachineTest, PageTwoShowsTheSecondHalfOfTheBankAboveA000)
{
    selectSlots(SLOT2, 0x00);
    write(0x6001, 5);
    EXPECT_EQ(read(0xa000, 16),
              std::vector<nw_byte>(nowindRom.begin() + 5 * 0x4000 + 0x2000, nowindRom.begin() + 5 * 0x4000 + 0x2010));
    write(0x6001, 0);
}

TEST_F(MachineTest, WritesTo4000ReachTheHostAndRepliesAreReadAt2000And8000)
{
    // sendRegisters (common.asm) writes the frame to 0x40xx, the ROM reads the reply at 0x2000 (nowind in page 0)
    // or at 0x8000 (nowind in page 2)
    selectSlots(SLOT2, 0x00);
    while (read(0x2000) != 0xff || read(0x2000) != 0xff) {} // drain anything left
    for (nw_byte value : GETDOSVERSION_FRAME) write(0x4000, value);
    EXPECT_EQ(read(0x2000, 4), (std::vector<nw_byte>{0xff, 0xaf, 0x05, 1})) << "DOS1, DOS2 is off in this machine";
    EXPECT_EQ(read(0x2000), 0xff) << "empty";

    for (nw_byte value : GETDOSVERSION_FRAME) write(0x8000, value);
    EXPECT_EQ(read(0x8000, 4), (std::vector<nw_byte>{0xff, 0xaf, 0x05, 1}));
}

TEST_F(MachineTest, TheUsbWindowIsOnlyThereWhenNowindIsSelected)
{
    selectSlots(SLOT2, 0x00);
    for (nw_byte value : GETDOSVERSION_FRAME) write(0x4000, value);
    selectSlots(nw_byte(0xaa & ~0x03), 0x00); // the main ROM in page 0
    EXPECT_EQ(read(0x2000), mainRom[0x2000]);
    selectSlots(SLOT2, 0x01); // nowind's slot in page 0, but another sub slot
    EXPECT_NE(read(0x2000, 4), (std::vector<nw_byte>{0xff, 0xaf, 0x05, 1}));
    selectSlots(SLOT2, 0x00);
    EXPECT_EQ(read(0x2000, 4), (std::vector<nw_byte>{0xff, 0xaf, 0x05, 1})) << "the reply waited";
}
