// BootTest.cpp, boots the emulated MSX2 with the nowind ROM assembled from msxsrc against the real nwhost.
//
// The emulator is built from singletons, so every test needs a process of its own: ctest runs each test separately
// (run the executable directly with --gtest_filter=<one test>).

#include "Debug.h"
#include "Emulator.h"
#include "cpu/Z80.h"
#include "devices/SlotSelector.h"
#include "memory/NowindInterface.h"
#include "nowindusb.h"
#include "support/Fat12Image.h"
#include "support/TempDir.h"
#include "video/V9938.h"

#include "SDL.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace nwtest;

namespace {

const std::string STAGE = EMUV1_STAGE;           // emu/, roms/, msxsrc/nowind.rom, disks/dos2.dsk
const std::string DISKS = NOWIND_SOURCE_DIR "/disks/";

std::vector<std::string> hostMessages;
void captureHostMessage(const char* message)
{
    hostMessages.push_back(message);
}

std::vector<std::uint8_t> readFile(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

enum class Dos { dos1, dos2 };

class BootTest : public ::testing::Test {
protected:
    // starts the emulator with this disk image in drive A (a copy, the test may write to it)
    void startEmulator(const std::vector<std::uint8_t>& image, Dos dos)
    {
        static bool started = false;
        ASSERT_FALSE(started) << "one integration test per process, run it with ctest or --gtest_filter";
        started = true;

        SDL_putenv(const_cast<char*>("SDL_VIDEODRIVER=dummy"));
        SDL_putenv(const_cast<char*>("SDL_AUDIODRIVER=dummy"));
        ASSERT_EQ(SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO), 0) << SDL_GetError();

        // the ROMs are loaded relative to Debug::getPath(), the disk images relative to the current directory
        std::filesystem::current_path(STAGE + "/emu");
        Debug::Instance()->setPath(STAGE + "/emu/");
        Emulator::Instance()->initialize(); // configures the MSX2 with the nowind interface in slot 2-0

        imagePath = dir.write("drive_a.dsk", image);
        NowindInterface* nowind = SlotSelector::Instance()->usbInterface;
        ASSERT_NE(nowind, nullptr);
        nowind->insertDisk(0, imagePath);
        nowind->setAttribute(nwhost::enable_dos2, dos == Dos::dos2);
        nowindusb_set_debug_callback(&captureHostMessage);
    }

    // the text on the screen in TEXT1 (40 columns) or TEXT2 (80 columns)
    std::string screen() const
    {
        const V9938* vdp = V9938::Instance();
        const nw_byte* vram = vdp->getVideoRam();
        bool text2 = (vdp->getRegister(0) & 0x08) != 0;
        unsigned columns = text2 ? 80 : 40;
        unsigned base = text2 ? (vdp->getRegister(2) & 0x7c) << 10 : (vdp->getRegister(2) & 0x0f) << 10;
        std::string text;
        for (unsigned row = 0; row < 24; ++row)
        {
            std::string line(reinterpret_cast<const char*>(vram + base + row * columns), columns);
            for (char& c : line)
                if (c < 32 || c > 126) c = ' ';
            line.erase(line.find_last_not_of(' ') + 1);
            text += line + "\n";
        }
        return text;
    }

    bool screenContains(const std::string& text) const { return screen().find(text) != std::string::npos; }

    // runs until the text is on the screen, at most maxMs emulated milliseconds
    bool runUntilScreenShows(const std::string& text, unsigned maxMs = 60000)
    {
        for (unsigned ms = 0; ms < maxMs; ms += 100)
        {
            if (screenContains(text)) return true;
            Emulator::Instance()->runFor(100);
        }
        return screenContains(text);
    }

    // the prompt, after a command was typed
    bool runUntilPrompt(unsigned promptsBefore, unsigned maxMs = 30000)
    {
        for (unsigned ms = 0; ms < maxMs; ms += 100)
        {
            if (countPrompts() > promptsBefore) return true;
            Emulator::Instance()->runFor(100);
        }
        return false;
    }

    unsigned countPrompts() const
    {
        std::string text = screen();
        unsigned count = 0;
        for (std::string::size_type at = text.find("A>"); at != std::string::npos; at = text.find("A>", at + 1)) ++count;
        return count;
    }

    // types a command at the prompt and waits for the next prompt
    bool command(const std::string& line)
    {
        unsigned prompts = countPrompts();
        Emulator::Instance()->typeText(line + "\n");
        return runUntilPrompt(prompts);
    }

    bool hostSaid(const std::string& text) const
    {
        return std::any_of(hostMessages.begin(), hostMessages.end(),
                           [&](const std::string& message) { return message.find(text) != std::string::npos; });
    }

    std::string hostLog() const
    {
        std::string log;
        for (const std::string& message : hostMessages) log += message;
        return log;
    }

    // NWTEST_SHOW_SCREEN=1 prints the screen at the end of each test, for debugging
    void TearDown() override
    {
        if (std::getenv("NWTEST_SHOW_SCREEN") && V9938::Instance())
        {
            std::cout << "after " << Z80::Instance()->emuTime / double(Z80::Instance()->cpuFrequency)
                      << " s emulated:\n" << screen() << std::endl;
        }
    }

    TempDir dir;
    std::string imagePath;
};

} // namespace

TEST_F(BootTest, BootsMsxDos1FromTheNowindDrive)
{
    startEmulator(readFile(DISKS + "DOS2.dsk"), Dos::dos1);
    ASSERT_TRUE(runUntilScreenShows("A>")) << screen() << hostLog();
    EXPECT_TRUE(hostSaid("MSX IDBYTE_2D contains: 1 (MSX2 detected)")) << hostLog();
    EXPECT_TRUE(hostSaid("DOS1 enabled as requested.")) << hostLog();
    EXPECT_FALSE(hostSaid("Unknown command")) << hostLog();
    EXPECT_FALSE(hostSaid("Protocol timeout")) << hostLog();
}

TEST_F(BootTest, BootsMsxDos2WhenTheHostEnablesIt)
{
    startEmulator(readFile(DISKS + "DOS2.dsk"), Dos::dos2);
    ASSERT_TRUE(runUntilScreenShows("A>")) << screen() << hostLog();
    EXPECT_TRUE(hostSaid("DOS2 enabled as requested.")) << hostLog();
    EXPECT_TRUE(command("ver")) << screen();
    EXPECT_TRUE(screenContains("MSX-DOS")) << screen();
    EXPECT_TRUE(screenContains("2.2")) << "the DOS 2.2 kernel of the nowind ROM\n" << screen();
}

TEST_F(BootTest, TheBootGoesThroughTheRomStartupSequence)
{
    // init.asm: getBootArgs (CMDREQUEST), GETDOSVERSION; the diskrom: INIHRD (AF FF = MSX reset), DRIVES, INIENV,
    // then DSKIO reads of the boot sector and the system files
    startEmulator(readFile(DISKS + "DOS2.dsk"), Dos::dos1);
    ASSERT_TRUE(runUntilScreenShows("A>")) << screen() << hostLog();
    std::string log = hostLog();
    std::vector<std::string> sequence = {"MSX requests command at startup", "MSX IDBYTE_2D contains", "MSX reset",
                                         "INIENV (romdrv nr: 1)", "NowindHost::diskRead, startSector: 0"};
    std::string::size_type at = 0;
    for (const std::string& step : sequence)
    {
        std::string::size_type found = log.find(step, at);
        EXPECT_NE(found, std::string::npos) << "'" << step << "' after position " << at << "\n" << log;
        if (found != std::string::npos) at = found;
    }
    EXPECT_FALSE(hostSaid("block") && hostSaid("failed!")) << "no block read had to be resent\n" << log;
}

TEST_F(BootTest, DirListsTheDisk)
{
    startEmulator(readFile(DISKS + "DOS2.dsk"), Dos::dos1);
    ASSERT_TRUE(runUntilScreenShows("A>")) << screen() << hostLog();
    ASSERT_TRUE(command("dir /w")) << screen();
    EXPECT_TRUE(screenContains("COMMAND")) << screen();
    EXPECT_TRUE(screenContains("NOWMAP")) << screen();
}

TEST_F(BootTest, TheRomdiskIsDriveB)
{
    // SlotSelector::configure puts the romdisk in drive 1; its content is disks/DOS2.dsk, built into the ROM by
    // INCLUDE_ROMDISK_360KB (labels.asm). Drive A gets a different disk, so B: can only come from the ROM.
    Fat12Image floppy(msxFloppyFormats()[0]);
    floppy.addFile("ONLY_A.TXT", "drive a\r\n");
    std::vector<std::uint8_t> bootable = readFile(DISKS + "DOS2.dsk");
    Fat12Image boot(bootable);
    for (const char* name : {"MSXDOS.SYS", "COMMAND.COM"}) floppy.addFile(name, *boot.readFile(name));
    std::copy_n(bootable.begin(), 11, floppy.data.begin()); // the boot code jump and OEM name
    std::copy(bootable.begin() + 30, bootable.begin() + 512, floppy.data.begin() + 30); // the boot code

    startEmulator(floppy.data, Dos::dos1);
    ASSERT_TRUE(runUntilScreenShows("A>")) << screen() << hostLog();
    ASSERT_TRUE(command("dir b:/w")) << screen();
    EXPECT_TRUE(screenContains("COMMAND2")) << screen();
    EXPECT_FALSE(screenContains("ONLY_A")) << screen();
}

TEST_F(BootTest, CopyWritesThroughToTheHostImage)
{
    // DSKIO writes end to end: COPY on the MSX, the file in the image on the host
    startEmulator(readFile(DISKS + "DOS2.dsk"), Dos::dos1);
    ASSERT_TRUE(runUntilScreenShows("A>")) << screen() << hostLog();
    ASSERT_TRUE(command("copy nw1.asc copy.asc")) << screen();
    EXPECT_TRUE(screenContains("1 file")) << screen();

    // inserting another disk closes (flushes) the image file
    SlotSelector::Instance()->usbInterface->insertDisk(0, dir.write("other.dsk", Fat12Image().data));
    Fat12Image written(readFile(imagePath));
    auto original = written.readFile("NW1.ASC");
    auto copy = written.readFile("COPY.ASC");
    ASSERT_TRUE(original);
    ASSERT_TRUE(copy) << "COPY.ASC is not in the image";
    EXPECT_EQ(*copy, *original);
}

// drive A with NOWMAP.COM assembled from msxsrc/msxdostools/nowmap.asm added as NWMAP.COM
std::vector<std::uint8_t> dos2DskWithAssembledNowmap()
{
    Fat12Image image(readFile(DISKS + "DOS2.dsk"));
    image.addFile("NWMAP.COM", readFile(NOWMAP_COM));
    return image.data;
}

// DISABLED: EXTBIO function 4 (sendCommandWithData, extendedBios.asm) takes the transfer address for the reply from
// IY ('push iy / pop de'), but IY does not survive the call through the EXTBIO hook: the hook is RST 30h (CALLF),
// which loads IY with the slot byte and the flags for CALSLT. NWMAP.COM loads IY with its buffer (0x0219), the host
// gets DE = 0x0210 (slot 0x02, F = 0x10) and sends the reply there, nothing is printed. ROM: msxsrc/extendedBios.asm
// sendCommandWithData, installExtendedBios (the hook), msxdostools/nowmap.asm 'ld iy,message'.
// Host: NowindHost::apiReceiveData uses reg_de as the destination.
TEST_F(BootTest, DISABLED_NowmapListsTheDrivesThroughExtbio)
{
    // NOWMAP.COM (msxdostools/nowmap.asm) uses the EXTBIO API (extendedBios.asm), C_COMMAND / API_NOWMAP and blockRead
    startEmulator(dos2DskWithAssembledNowmap(), Dos::dos1);
    ASSERT_TRUE(runUntilScreenShows("A>")) << screen() << hostLog();
    ASSERT_TRUE(command("nwmap /l")) << screen();
    EXPECT_TRUE(screenContains("Nowind Map v1.2")) << screen() << hostLog();
    EXPECT_TRUE(screenContains("driveId 0")) << screen();
}

// DISABLED: the NOWMAP.COM on disks/DOS2.dsk (and so in the ROM disk) is older than msxdostools/nowmap.asm: it sends
// C = 3, B = 0x97 with C_COMMAND, the host only knows API_NOWMAP (C = 0) and does not reply ('apiCommand_API_??'),
// NOWMAP.COM then says 'No connection with host!'. disks/DOS2.dsk NOWMAP.COM vs msxsrc/msxdostools/nowmap.asm.
TEST_F(BootTest, DISABLED_TheNowmapComOnDos2DskTalksToTheHost)
{
    startEmulator(readFile(DISKS + "DOS2.dsk"), Dos::dos1);
    ASSERT_TRUE(runUntilScreenShows("A>")) << screen() << hostLog();
    ASSERT_TRUE(command("nowmap /l")) << screen();
    EXPECT_TRUE(screenContains("Nowind Map v1.2")) << screen() << hostLog();
}
