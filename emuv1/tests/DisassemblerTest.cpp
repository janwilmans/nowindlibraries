// DisassemblerTest.cpp

#include "cpu/Disassembler.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

struct Disassembled {
    std::string text;
    int size;
};

// disAsm gets the two words at pc and pc + 2
Disassembled disassemble(std::vector<nw_byte> code, nw_word pc = 0x4000)
{
    code.resize(4, 0);
    Disassembler* disassembler = Disassembler::Instance();
    std::string text = disassembler->disAsm(pc, code[0] | (code[1] << 8), code[2] | (code[3] << 8));
    return {text, disassembler->getSize()};
}

} // namespace

TEST(Disassembler, PlainInstructions)
{
    EXPECT_EQ(disassemble({0x00}).text, "nop");
    EXPECT_EQ(disassemble({0x00}).size, 1);
    EXPECT_EQ(disassemble({0x3e, 0x42}).text, "ld   a,42");
    EXPECT_EQ(disassemble({0x3e, 0x42}).size, 2);
    EXPECT_EQ(disassemble({0x01, 0x34, 0x12}).text, "ld   bc,1234");
    EXPECT_EQ(disassemble({0x01, 0x34, 0x12}).size, 3);
    EXPECT_EQ(disassemble({0x2a, 0x34, 0x12}).text, "ld   hl,(1234)");
    EXPECT_EQ(disassemble({0x77}).text, "ld   (hl),a");
}

TEST(Disassembler, RelativeJumpsShowTheTarget)
{
    EXPECT_EQ(disassemble({0x18, 0xfe}, 0x4000).text, "jr   4000");
    EXPECT_EQ(disassemble({0x10, 0x05}, 0x4000).text, "djnz 4007");
    EXPECT_EQ(disassemble({0x18, 0xfe}).size, 2);
}

TEST(Disassembler, PrefixedInstructions)
{
    EXPECT_EQ(disassemble({0xcb, 0x7f}).text, "bit  7,a");
    EXPECT_EQ(disassemble({0xcb, 0x7f}).size, 2);
    EXPECT_EQ(disassemble({0xed, 0xb0}).text, "ldir");
    EXPECT_EQ(disassemble({0xed, 0xb0}).size, 2);
    EXPECT_EQ(disassemble({0xfd, 0x21, 0x34, 0x12}).text, "ld   iy,1234");
    EXPECT_EQ(disassemble({0xfd, 0x21, 0x34, 0x12}).size, 4);
}

TEST(Disassembler, IndexedInstructionsShowTheDisplacement)
{
    EXPECT_EQ(disassemble({0xdd, 0x7e, 0x05}).text, "ld   a,(ix+05)");
    EXPECT_EQ(disassemble({0xdd, 0x7e, 0x05}).size, 3);
    EXPECT_EQ(disassemble({0xdd, 0x36, 0x05, 0x42}).text, "ld   (ix+05),42");
    EXPECT_EQ(disassemble({0xdd, 0x36, 0x05, 0x42}).size, 4);
}

TEST(Disassembler, TheEmulatorTrapsHaveNames)
{
    // ED 00 is the emulator's own DISKIO trap (opcodesED.inc)
    EXPECT_EQ(disassemble({0xed, 0x00}).text, "[ed]DISKIO");
}
