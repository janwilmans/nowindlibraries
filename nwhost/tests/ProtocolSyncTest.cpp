// ProtocolSyncTest.cpp, checks that the host (nwhost) and the ROM (msxsrc, and the ROM assembled from it) agree.
// The sources are read at test time, so a change on one side that is not made on the other fails here.

#include "BlockRead.hh"
#include "BlockWrite.hh"
#include "NowindHost.hh"
#include "ReceiveRegisters.hh"
#include "NowindHostSupport.hh"
#include "support/Fat12Image.h"
#include "support/HostFixture.h"

#include <gtest/gtest.h>

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace nwtest;

namespace {

const std::string SOURCE_DIR = NOWIND_SOURCE_DIR;
const std::string MSXSRC = SOURCE_DIR + "/msxsrc/";

std::string readText(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot read " + path);
    std::stringstream text;
    text << in.rdbuf();
    return text.str();
}

std::vector<byte> readBinary(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot read " + path);
    return std::vector<byte>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

// the code of a routine: from 'label:' up to the next label at the start of a line
std::string routine(const std::string& source, const std::string& label)
{
    std::string::size_type begin = source.find("\n" + label + ":");
    if (begin == std::string::npos) throw std::runtime_error("label not found: " + label);
    std::regex nextLabel("\n[A-Za-z_][A-Za-z0-9_]*:");
    std::smatch match;
    std::string rest = source.substr(begin + 1 + label.size() + 1);
    if (std::regex_search(rest, match, nextLabel)) rest = rest.substr(0, match.position());
    return rest;
}

// the instructions of a piece of code, comments and blank lines removed, whitespace collapsed
std::vector<std::string> instructions(const std::string& code)
{
    std::vector<std::string> result;
    std::istringstream lines(code);
    std::string line;
    while (std::getline(lines, line))
    {
        line = line.substr(0, line.find(';'));
        line = line.substr(0, line.find("//"));
        line = std::regex_replace(line, std::regex("^\\s*\\.?[A-Za-z_][A-Za-z0-9_.]*:"), ""); // a label before the code
        line = std::regex_replace(line, std::regex("\\s+"), " ");
        line = std::regex_replace(line, std::regex("^ | $"), "");
        if (!line.empty()) result.push_back(line);
    }
    return result;
}

// the command codes in the host, by their ROM name
const std::map<std::string, int>& hostCommands()
{
    static const std::map<std::string, int> commands = {
        {"C_DSKIO", nwhost::C_DSKIO},
        {"C_DSKCHG", nwhost::C_DSKCHG},
        {"C_GETDPB", nwhost::C_GETDPB},
        {"C_CHOICE", nwhost::C_CHOICE},
        {"C_DSKFMT", nwhost::C_DSKFMT},
        {"C_DRIVES", nwhost::C_DRIVES},
        {"C_INIENV", nwhost::C_INIENV},
        {"C_GETDATE", nwhost::C_GETDATE},
        {"C_DEVICEOPEN", nwhost::C_DEVICEOPEN},
        {"C_DEVICECLOSE", nwhost::C_DEVICECLOSE},
        {"C_DEVICERNDIO", nwhost::C_DEVICERNDIO},
        {"C_DEVICEWRITE", nwhost::C_DEVICEWRITE},
        {"C_DEVICEREAD", nwhost::C_DEVICEREAD},
        {"C_DEVICEEOF", nwhost::C_DEVICEEOF},
        {"C_AUXIN", nwhost::C_AUXIN},
        {"C_AUXOUT", nwhost::C_AUXOUT},
        {"C_MESSAGE", nwhost::C_MESSAGE},
        {"C_CHANGEIMAGE", nwhost::C_CHANGEIMAGE},
        {"C_GETDOSVERSION", nwhost::C_GETDOSVERSION},
        {"C_CMDREQUEST", nwhost::C_CMDREQUEST},
        {"C_BLOCKREAD", nwhost::C_BLOCKREAD},
        {"C_BLOCKWRITE", nwhost::C_BLOCKWRITE},
        {"C_CPUINFO", nwhost::C_CPUINFO},
        {"C_COMMAND", nwhost::C_COMMAND},
        {"C_STDOUT", nwhost::C_STDOUT},
    };
    return commands;
}

// NAME equ $nn in labels.asm
std::map<std::string, int> romEquates()
{
    std::map<std::string, int> result;
    std::string labels = readText(MSXSRC + "labels.asm");
    // $nn is hex, a plain number decimal
    std::regex equ("\n([A-Za-z_0-9]+)\\s+equ\\s+(\\$?)([0-9A-Fa-f]+)");
    for (auto it = std::sregex_iterator(labels.begin(), labels.end(), equ); it != std::sregex_iterator(); ++it)
    {
        result[(*it)[1]] = std::stoi((*it)[3], nullptr, (*it)[2].length() ? 16 : 10);
    }
    return result;
}

// the .asm files assembled into nowind.rom (the includes of nowind.asm)
std::vector<std::string> romSourceFiles()
{
    std::vector<std::string> files;
    std::string main = readText(MSXSRC + "nowind.asm");
    std::regex include("\n\\s*include \"([^\"]+)\"");
    std::set<std::string> seen;
    for (auto it = std::sregex_iterator(main.begin(), main.end(), include); it != std::sregex_iterator(); ++it)
    {
        if (seen.insert((*it)[1]).second) files.push_back((*it)[1]);
    }
    return files;
}

// a host that remembers its debug messages, to see whether a command ended up as 'Unknown command'
class RecordingHost : public nwhost::NowindHost {
public:
    using NowindHost::NowindHost;
    void debugMessage(const char* format, ...) const override
    {
        char text[1024];
        va_list args;
        va_start(args, format);
        std::vsnprintf(text, sizeof(text), format, args);
        va_end(args);
        messages.push_back(text);
    }
    mutable std::vector<std::string> messages;
};

// the ROM symbols from sjasm's .sym file: "NAME: equ 0000XXXXh"
std::map<std::string, unsigned> romSymbols()
{
    std::map<std::string, unsigned> result;
    std::ifstream in(NOWIND_ROM_SYMBOLS);
    std::string line;
    std::regex symbol("^([^:]+): equ ([0-9A-Fa-f]+)h");
    std::smatch match;
    while (std::getline(in, line))
    {
        if (std::regex_search(line, match, symbol)) result[match[1]] = unsigned(std::stoul(match[2], nullptr, 16));
    }
    return result;
}

} // namespace

TEST(ProtocolSync, CommandCodesAreTheSameInRomAndHost)
{
    std::map<std::string, int> rom = romEquates();
    for (const auto& [name, value] : hostCommands())
    {
        ASSERT_TRUE(rom.count(name)) << name << " is not in msxsrc/labels.asm";
        EXPECT_EQ(rom[name], value) << name;
    }
    for (const auto& [name, value] : rom)
    {
        if (name.rfind("C_", 0) == 0) EXPECT_TRUE(hostCommands().count(name)) << name << " is not known by the host";
    }
    EXPECT_EQ(rom.at("API_NOWMAP"), nwhost::NowindHost::API_NOWMAP);
}

TEST(ProtocolSync, TheRomSourceFilesAreFound)
{
    std::vector<std::string> files = romSourceFiles();
    EXPECT_GE(files.size(), 10u);
    for (const std::string& file : files) EXPECT_NO_THROW(readText(MSXSRC + file)) << file;
}

TEST(ProtocolSync, EveryCommandTheRomSendsIsHandledByTheHost)
{
    std::set<std::string> sent;
    std::regex command("ld \\(hl\\),(C_[A-Z]+)");
    for (const std::string& file : romSourceFiles())
    {
        std::string source = readText(MSXSRC + file);
        for (auto it = std::sregex_iterator(source.begin(), source.end(), command); it != std::sregex_iterator(); ++it)
        {
            sent.insert((*it)[1]);
        }
    }
    ASSERT_GE(sent.size(), 15u);
    for (const std::string& name : sent)
    {
        ASSERT_TRUE(hostCommands().count(name)) << name;
        if (name == "C_AUXIN") continue; // the host reads a character from std::cin
        MemoryDisk disk;
        std::vector<nwhost::DiskHandler*> drives{&disk};
        RecordingHost host(drives);
        host.initialize();
        MsxSide msx(host);
        msx.sendCommand(Regs(), byte(hostCommands().at(name)));
        for (const std::string& message : host.messages)
        {
            EXPECT_EQ(message.find("Unknown command"), std::string::npos) << name << ": " << message;
        }
    }
}

TEST(ProtocolSync, TheRegisterFrameOrderIsTheOneTheHostReads)
{
    // sendRegisters (common.asm) writes AF 05 c b e d l h f a; Command (Command.hh) reads cmdData in that order
    std::vector<std::string> code = instructions(routine(readText(MSXSRC + "common.asm"), "sendRegisters"));
    std::vector<std::string> writes;
    for (const std::string& line : code)
    {
        if (line.rfind("ld (hl),", 0) == 0) writes.push_back(line.substr(8));
    }
    // h goes through a ('ld a,h' first), f and a through de ('pop de' of the pushed af)
    EXPECT_EQ(writes, (std::vector<std::string>{"$af", "$05", "c", "b", "e", "d", "l", "a", "e", "d"}));
    EXPECT_NE(std::find(code.begin(), code.end(), "ld a,h"), code.end());

    nwhost::Command command;
    for (byte i = 0; i < 9; ++i) command.cmdData[i] = byte(0x10 + i);
    EXPECT_EQ(command.getC(), 0x10);
    EXPECT_EQ(command.getB(), 0x11);
    EXPECT_EQ(command.getE(), 0x12);
    EXPECT_EQ(command.getD(), 0x13);
    EXPECT_EQ(command.getL(), 0x14);
    EXPECT_EQ(command.getH(), 0x15);
    EXPECT_EQ(command.getF(), 0x16);
    EXPECT_EQ(command.getA(), 0x17);
    EXPECT_EQ(command.getCommand(), 0x18);
}

TEST(ProtocolSync, ReceiveRegistersSendsInTheOrderTheRomPops)
{
    // receiveRegisters (common.asm): after the header, 6 x (ld c,(hl) / ld b,(hl) / push bc), then
    // pop iy, ix, hl, de, bc, af: the first pair is AF (F first), the last IY
    std::vector<std::string> code = instructions(routine(readText(MSXSRC + "common.asm"), "receiveRegisters"));
    std::vector<std::string> pops;
    for (const std::string& line : code)
    {
        if (line.rfind("pop ", 0) == 0 && line != "pop bc") pops.push_back(line.substr(4));
    }
    // the 'pop bc' lines are .retry throwing data away, the return values are popped in .done
    EXPECT_EQ(pops, (std::vector<std::string>{"iy", "ix", "hl", "de", "af"}));

    nwhost::NowindHostSupport support;
    nwhost::ReceiveRegisters registers;
    registers.initialize(&support);
    registers.setF(0x10);
    registers.setA(0x21);
    registers.setBC(0x3332);
    registers.setDE(0x4544);
    registers.setHL(0x5756);
    registers.setIX(0x6968);
    registers.setIY(0x7b7a);
    registers.send();
    std::vector<byte> stream;
    while (support.isDataAvailable()) stream.push_back(support.read());
    // FF AF 05, a dummy FF (read by getHeaderInPage0 into A), header, 12 bytes, tail
    ASSERT_EQ(stream.size(), 3u + 1 + 1 + 12 + 1);
    EXPECT_EQ(stream[3], 0xff);
    std::vector<byte> data(stream.begin() + 5, stream.begin() + 17);
    EXPECT_EQ(data, (std::vector<byte>{0x10, 0x21, 0x32, 0x33, 0x44, 0x45, 0x56, 0x57, 0x68, 0x69, 0x7a, 0x7b}));
    EXPECT_EQ(stream[4], stream[17]) << "header == tail";
}

TEST(ProtocolSync, TheFastBlockSizeMatchesBlockReadTranfer)
{
    // blockReadTranfer: 'repeat 64' of 'ld d,(hl) / ld e,(hl) / push de' = 128 bytes per block
    std::string code = routine(readText(MSXSRC + "common.asm"), "blockReadTranfer");
    std::smatch match;
    ASSERT_TRUE(std::regex_search(code, match, std::regex("repeat (\\d+)\\s*;[^\n]*\n\\s*ld d,\\(hl\\)\\s*\n\\s*ld e,\\(hl\\)\\s*\n\\s*push de")));
    EXPECT_EQ(std::stoi(match[1]) * 2, nwhost::HARDCODED_READ_DATABLOCK_SIZE);
}

TEST(ProtocolSync, BlockReadCodesMatchBlockRead01And23)
{
    // blockRead01: 1 = fast, 0 = exit (more data for page 2/3), 2 = slow, anything else ends the transfer
    std::vector<std::string> code01 = instructions(routine(readText(MSXSRC + "common.asm"), "blockRead01"));
    std::vector<std::string> expected01 = {"call getHeaderInPage2", "ret c", "cp 1", "jr z,.fastTransfer", "and a",
                                           "ret z", "cp 2", "ret nz"};
    ASSERT_GE(code01.size(), expected01.size());
    EXPECT_EQ(std::vector<std::string>(code01.begin(), code01.begin() + expected01.size()), expected01);
    std::vector<std::string> code23 = instructions(routine(readText(MSXSRC + "common.asm"), "blockRead23"));
    std::vector<std::string> expected23 = {"call getHeaderInPage0", "ret c", "cp 1", "jr z,.fastTransfer", "cp 2",
                                           "ret nz"};
    ASSERT_GE(code23.size(), expected23.size());
    EXPECT_EQ(std::vector<std::string>(code23.begin(), code23.begin() + expected23.size()), expected23);

    EXPECT_EQ(nwhost::BlockRead::BLOCKREAD_EXIT_MORE_DATE_AHEAD, 0);
    EXPECT_EQ(nwhost::BlockRead::BLOCKREAD_FASTTRANSFER, 1);
    EXPECT_EQ(nwhost::BlockRead::BLOCKREAD_SLOWTRANSFER, 2);
    EXPECT_GT(nwhost::BlockRead::BLOCKREAD_EXIT, 2);
}

TEST(ProtocolSync, BlockWriteCodesMatchBlockWrite01And23)
{
    // blockWrite01/23: 'or a / ret m' (0x80..0xFF end), 'jr nz,.error' (1..127, error code follows), 0 = data;
    // blockWrite: 'ret pe' after blockWrite01: 0xFF (even parity) is done, 0xFE (odd) continues in page 2/3
    std::string common = readText(MSXSRC + "common.asm");
    for (const char* name : {"blockWrite01", "blockWrite23"})
    {
        std::vector<std::string> code = instructions(routine(common, name));
        ASSERT_GE(code.size(), 5u) << name;
        EXPECT_EQ(std::vector<std::string>(code.begin() + 1, code.begin() + 5),
                  (std::vector<std::string>{"ret c", "or a", "ret m", "jr nz,.error"}))
            << name;
    }
    std::vector<std::string> outer = instructions(routine(common, "blockWrite"));
    EXPECT_NE(std::find(outer.begin(), outer.end(), "ret pe"), outer.end());

    EXPECT_EQ(nwhost::BlockWrite::BLOCKWRITE_END, 0xff);
    EXPECT_EQ(nwhost::BlockWrite::BLOCKWRITE_PAGE23_DATA, 0xfe);
    EXPECT_EQ(nwhost::BlockWrite::BLOCKWRITE_EXIT, 0) << "0 = a data request follows";
}

TEST(ProtocolSync, TheRomModelStillMatchesCommonAsm)
{
    // support/RomModel.h copies these details; when one of them changes, update the model too
    std::string common = readText(MSXSRC + "common.asm");
    std::vector<std::string> header = instructions(routine(common, "getHeaderInPage0"));
    EXPECT_NE(std::find(header.begin(), header.end(), "ld b,HIGH 65535"), header.end()) << "timeout count";
    EXPECT_NE(std::find(header.begin(), header.end(), "cp $af"), header.end());
    EXPECT_NE(std::find(header.begin(), header.end(), "cp $05"), header.end());
    std::vector<std::string> slow = instructions(routine(common, "slowTransfer"));
    EXPECT_EQ(slow, (std::vector<std::string>{"ld e,(hl)", "ld d,(hl)", "ld c,(hl)", "ld b,(hl)", "ld a,(hl)", "ldir",
                                              "ld b,(hl)", "cp a", "ld a,b", "ret"}));
    std::vector<std::string> invalid = instructions(routine(common, "invalidHeader"));
    EXPECT_NE(std::find(invalid.begin(), invalid.end(), "cp 255"), invalid.end()) << "0xFF headers are skipped";
}

TEST(ProtocolSync, TheHostDpbMatchesTheRomsSupportedMedia)
{
    // the ROM has a DPB table for media F8..FF (supportedMedia, MAKEDPB in nowindDriver.asm) and asks the host
    // (GETDPB) for anything else; for the standard formats both must give the same DPB
    std::vector<byte> rom = readBinary(NOWIND_ROM);
    std::map<std::string, unsigned> symbols = romSymbols();
    struct Module {
        const char* symbol;
        unsigned bank;
    };
    for (Module module : {Module{"MSXDOS2_MODULE.supportedMedia", 0}, Module{"MSXDOS1_MODULE.supportedMedia", 4}})
    {
        ASSERT_TRUE(symbols.count(module.symbol)) << module.symbol;
        unsigned offset = module.bank * 0x4000 + symbols[module.symbol] - 0x4000;
        for (std::size_t i = 0; i < msxFloppyFormats().size(); ++i)
        {
            const FloppyFormat& format = msxFloppyFormats()[i];
            std::vector<byte> romDpb(rom.begin() + offset + i * 18, rom.begin() + offset + (i + 1) * 18);

            MemoryDisk disk;
            Fat12Image::writeBootSector(disk.sector(0), format);
            std::vector<nwhost::DiskHandler*> drives{&disk};
            nwhost::NowindHost host(drives);
            host.initialize();
            MsxSide msx(host);
            msx.sendCommand(Regs().setDE(0xf000), nwhost::C_GETDPB);
            ASSERT_TRUE(msx.readHeader());
            msx.read16();
            std::vector<byte> hostDpb = msx.read(18);
            EXPECT_EQ(hostDpb, romDpb) << module.symbol << " media 0x" << std::hex << int(format.media);
        }
    }
}

// DISABLED: msxsrc/nowind.rom (committed 2013) is not what msxsrc assembles to today (the asm was changed in
// 2024); about 1200 bytes differ, starting in bank 5 (init.asm). Which one is flashed on real interfaces?
TEST(ProtocolSync, DISABLED_TheCommittedRomIsTheAssembledSource)
{
    std::vector<byte> assembled = readBinary(NOWIND_ROM);
    std::vector<byte> committed = readBinary(MSXSRC + "nowind.rom");
    ASSERT_EQ(assembled.size(), committed.size());
    std::size_t differences = 0;
    std::size_t first = assembled.size();
    for (std::size_t i = 0; i < assembled.size(); ++i)
    {
        if (assembled[i] != committed[i])
        {
            ++differences;
            first = std::min(first, i);
        }
    }
    EXPECT_EQ(differences, 0u) << "first difference at 0x" << std::hex << first << " (bank " << first / 0x4000 << ")";
}
