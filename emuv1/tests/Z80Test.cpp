// Z80Test.cpp, the emuv1 Z80 core; expected flags as documented for the real Z80 (including the undocumented
// X and Y flags, bits 3 and 5, which emuv1 also emulates)

#include "support/Z80TestAccess.h"

#include <gtest/gtest.h>

namespace {

const nw_word S = 0x80, Z = 0x40, Y = 0x20, H = 0x10, X = 0x08, P = 0x04, N = 0x02, C = 0x01;

class Z80Test : public ::testing::Test {
protected:
    Z80TestAccess z80;
};

} // namespace

TEST_F(Z80Test, FlagTables)
{
    EXPECT_EQ(z80.flagSZ(0x00), Z);
    EXPECT_EQ(z80.flagSZ(0x80), S);
    EXPECT_EQ(z80.flagSZ(0x28), Y | X);
    EXPECT_EQ(z80.flagSZP(0x00), Z | P);
    EXPECT_EQ(z80.flagSZP(0x01), 0) << "odd parity";
    EXPECT_EQ(z80.flagSZP(0x03), P);
    EXPECT_EQ(z80.flagInc(0x80), S | H | P) << "0x7F + 1 overflows";
    EXPECT_EQ(z80.flagInc(0x10), H);
    EXPECT_EQ(z80.flagInc(0x00), Z | H);
    EXPECT_EQ(z80.flagDec(0x7f), Y | H | X | P | N) << "0x80 - 1 overflows";
    EXPECT_EQ(z80.flagDec(0x00), Z | N);
}

TEST_F(Z80Test, InstructionsTakeAnExtraCycleForEachM1OnMsx)
{
    z80.load({0x00, 0x06, 0x42, 0xed, 0x44}); // nop / ld b,$42 / neg
    EXPECT_EQ(z80.step(), 4u + 1);
    EXPECT_EQ(z80.step(), 7u + 1);
    EXPECT_EQ(z80.step(), 8u + 2) << "ED prefix: two M1 cycles";
}

// DISABLED: emuv1 counts 14 T-states for 'ld a,n' (0x3E), the Z80 takes 7 like the other 'ld r,n'.
// emuv1/cpu/opcodes.inc 'case 0x3E: TS(14)'.
TEST_F(Z80Test, DISABLED_LdANTakesAsLongAsTheOtherLdRN)
{
    z80.load({0x3e, 0x42}); // ld a,$42
    EXPECT_EQ(z80.step(), 7u + 1);
}

TEST_F(Z80Test, LoadsBetweenRegistersAndMemory)
{
    // ld a,$42 / ld hl,$d000 / ld (hl),a / ld b,(hl) / ld ($d001),a / ld de,($d000)
    z80.load({0x3e, 0x42, 0x21, 0x00, 0xd0, 0x77, 0x46, 0x32, 0x01, 0xd0, 0xed, 0x5b, 0x00, 0xd0});
    z80.step(6);
    EXPECT_EQ(z80.ram[0xd000], 0x42);
    EXPECT_EQ(z80.b(), 0x42u);
    EXPECT_EQ(z80.ram[0xd001], 0x42);
    EXPECT_EQ(z80.de(), 0x4242u);
    EXPECT_EQ(z80.pc(), 0xc00eu);
}

struct AluCase {
    const char* name;
    nw_byte opcode; // the 'op a,n' form
    nw_word a;
    nw_word carryIn;
    nw_byte operand;
    nw_word result;
    nw_word flags;
};

class Z80AluTest : public ::testing::TestWithParam<AluCase> {
protected:
    Z80TestAccess z80;
};

TEST_P(Z80AluTest, ResultAndFlags)
{
    const AluCase& test = GetParam();
    z80.a() = test.a;
    z80.f() = test.carryIn;
    z80.load({test.opcode, test.operand});
    z80.step();
    EXPECT_EQ(z80.a(), test.result) << test.name;
    EXPECT_EQ(z80.f(), test.flags) << test.name << " flags";
}

INSTANTIATE_TEST_SUITE_P(Z80, Z80AluTest,
                         ::testing::Values(AluCase{"add overflow", 0xc6, 0x7f, 0, 0x01, 0x80, S | H | P},
                                           AluCase{"add carry to zero", 0xc6, 0xff, 0, 0x01, 0x00, Z | H | C},
                                           AluCase{"add half carry", 0xc6, 0x0f, 0, 0x01, 0x10, H},
                                           AluCase{"adc carry in", 0xce, 0x0e, C, 0x01, 0x10, H},
                                           AluCase{"sub overflow", 0xd6, 0x80, 0, 0x01, 0x7f, Y | H | X | P | N},
                                           AluCase{"sub to zero", 0xd6, 0x01, 0, 0x01, 0x00, Z | N},
                                           AluCase{"sub borrow", 0xd6, 0x00, 0, 0x01, 0xff, S | Y | H | X | N | C},
                                           AluCase{"sbc carry in", 0xde, 0x10, C, 0x0f, 0x00, Z | H | N},
                                           AluCase{"and", 0xe6, 0xf0, 0, 0x0f, 0x00, Z | H | P},
                                           AluCase{"xor", 0xee, 0x55, C, 0xff, 0xaa, S | Y | X | P},
                                           AluCase{"or", 0xf6, 0x01, C, 0x02, 0x03, P},
                                           // cp: the result is not stored, X and Y come from the operand
                                           AluCase{"cp equal", 0xfe, 0x10, 0, 0x10, 0x10, Z | N},
                                           AluCase{"cp xy from operand", 0xfe, 0x28, 0, 0x28, 0x28, Z | Y | X | N},
                                           AluCase{"cp less", 0xfe, 0x01, 0, 0x02, 0x01, S | H | N | C}),
                         [](const ::testing::TestParamInfo<AluCase>& paramInfo) {
                             std::string name = paramInfo.param.name;
                             for (char& c : name)
                                 if (c == ' ') c = '_';
                             return name;
                         });

TEST_F(Z80Test, IncAndDecKeepTheCarry)
{
    z80.a() = 0x7f;
    z80.f() = C;
    z80.load({0x3c, 0x3d}); // inc a / dec a
    z80.step();
    EXPECT_EQ(z80.a(), 0x80u);
    EXPECT_EQ(z80.f(), S | H | P | C);
    z80.step();
    EXPECT_EQ(z80.a(), 0x7fu);
    EXPECT_EQ(z80.f(), Y | H | X | P | N | C);
}

TEST_F(Z80Test, NegIsZeroMinusA)
{
    z80.a() = 0x01;
    z80.load({0xed, 0x44});
    z80.step();
    EXPECT_EQ(z80.a(), 0xffu);
    EXPECT_EQ(z80.f(), S | Y | H | X | N | C);
}

TEST_F(Z80Test, Add16SetsHalfCarryFromBit11AndKeepsSzp)
{
    z80.hl() = 0x0fff;
    z80.setBC(0x0001);
    z80.f() = S | Z | P;
    z80.load({0x09}); // add hl,bc
    z80.step();
    EXPECT_EQ(z80.hl(), 0x1000u);
    EXPECT_EQ(z80.f(), S | Z | P | H);

    z80.hl() = 0xffff;
    z80.de() = 0x0001;
    z80.f() = 0;
    z80.load({0x19}, 0xc001); // add hl,de
    z80.step();
    EXPECT_EQ(z80.hl(), 0x0000u);
    EXPECT_EQ(z80.f(), H | C);
}

TEST_F(Z80Test, RlcaRotatesIntoCarry)
{
    z80.a() = 0x81;
    z80.f() = S | Z | P;
    z80.load({0x07});
    z80.step();
    EXPECT_EQ(z80.a(), 0x03u);
    EXPECT_EQ(z80.f(), S | Z | P | C);
}

TEST_F(Z80Test, CbBitSetResAndShift)
{
    z80.a() = 0x80;
    z80.load({0xcb, 0x7f, 0xcb, 0xc7, 0xcb, 0xbf, 0xcb, 0x3f}); // bit 7,a / set 0,a / res 7,a / srl a
    z80.step();
    EXPECT_EQ(z80.f() & Z, 0u) << "bit 7 is set";
    z80.step();
    EXPECT_EQ(z80.a(), 0x81u);
    z80.step();
    EXPECT_EQ(z80.a(), 0x01u);
    z80.step();
    EXPECT_EQ(z80.a(), 0x00u);
    EXPECT_EQ(z80.f() & (Z | C), Z | C);
}

TEST_F(Z80Test, LdirCopiesAndClearsBc)
{
    for (unsigned i = 0; i < 5; ++i) z80.ram[0xd000 + i] = nw_byte('a' + i);
    z80.hl() = 0xd000;
    z80.de() = 0xe000;
    z80.setBC(5);
    z80.load({0xed, 0xb0, 0x00}); // ldir / nop
    ASSERT_TRUE(z80.runUntil(0xc002));
    for (unsigned i = 0; i < 5; ++i) EXPECT_EQ(z80.ram[0xe000 + i], nw_byte('a' + i));
    EXPECT_EQ(z80.bc(), 0u);
    EXPECT_EQ(z80.hl(), 0xd005u);
    EXPECT_EQ(z80.de(), 0xe005u);
    EXPECT_EQ(z80.f() & P, 0u);
}

TEST_F(Z80Test, CpirStopsAtTheByte)
{
    for (unsigned i = 0; i < 5; ++i) z80.ram[0xd000 + i] = nw_byte('A' + i);
    z80.hl() = 0xd000;
    z80.setBC(5);
    z80.a() = 'C';
    z80.load({0xed, 0xb1, 0x00}); // cpir / nop
    ASSERT_TRUE(z80.runUntil(0xc002));
    EXPECT_EQ(z80.hl(), 0xd003u);
    EXPECT_EQ(z80.bc(), 2u);
    EXPECT_EQ(z80.f() & (Z | P), Z | P) << "found, bc not zero";
}

TEST_F(Z80Test, CallPushesTheReturnAddress)
{
    // call $c010 / halt ... $c010: push bc / pop de / ret
    z80.setBC(0x1234);
    z80.load({0xcd, 0x10, 0xc0, 0x76});
    z80.load({0xc5, 0xd1, 0xc9}, 0xc010);
    z80.step();
    EXPECT_EQ(z80.pc(), 0xc010u);
    EXPECT_EQ(z80.sp(), 0xeffeu);
    EXPECT_EQ(z80.ram[0xeffe] | (z80.ram[0xefff] << 8), 0xc003);
    z80.step(3);
    EXPECT_EQ(z80.de(), 0x1234u);
    EXPECT_EQ(z80.pc(), 0xc003u);
    EXPECT_EQ(z80.sp(), 0xf000u);
}

TEST_F(Z80Test, DjnzLoops)
{
    // ld b,5 / loop: inc a / djnz loop / nop
    z80.load({0x06, 0x05, 0x3c, 0x10, 0xfd, 0x00});
    ASSERT_TRUE(z80.runUntil(0xc005));
    EXPECT_EQ(z80.a(), 5u);
    EXPECT_EQ(z80.b(), 0u);
}

TEST_F(Z80Test, JrJumpsRelative)
{
    z80.load({0x18, 0x02, 0x00, 0x00, 0x18, 0xfa}); // jr +2 / ... / jr -6
    z80.step();
    EXPECT_EQ(z80.pc(), 0xc004u);
    z80.step();
    EXPECT_EQ(z80.pc(), 0xc000u);
}

TEST_F(Z80Test, IndexedLoadsWithPositiveAndNegativeDisplacement)
{
    // ld ix,$d010 / ld (ix+2),$55 / ld a,(ix+2) / ld (ix-1),a / ld iy,$d020 / ld b,(iy-$10)
    z80.load({0xdd, 0x21, 0x10, 0xd0, 0xdd, 0x36, 0x02, 0x55, 0xdd, 0x7e, 0x02, 0xdd, 0x77, 0xff, 0xfd, 0x21, 0x20,
              0xd0, 0xfd, 0x46, 0xf2});
    z80.step(6);
    EXPECT_EQ(z80.ix(), 0xd010u);
    EXPECT_EQ(z80.ram[0xd012], 0x55);
    EXPECT_EQ(z80.a(), 0x55u);
    EXPECT_EQ(z80.ram[0xd00f], 0x55);
    EXPECT_EQ(z80.b(), 0x55u) << "iy - 14 = 0xd012";
}

TEST_F(Z80Test, ExchangeInstructions)
{
    z80.a() = 0x11;
    z80.f() = 0x22;
    z80.shadowAf() = 0x3344;
    z80.de() = 0x1234;
    z80.hl() = 0x5678;
    z80.load({0x08, 0xeb}); // ex af,af' / ex de,hl
    z80.step(2);
    EXPECT_EQ(z80.a(), 0x33u);
    EXPECT_EQ(z80.f(), 0x44u);
    EXPECT_EQ(z80.shadowAf(), 0x1122u);
    EXPECT_EQ(z80.de(), 0x5678u);
    EXPECT_EQ(z80.hl(), 0x1234u);
}

// (EI and DI call Emulator::setCPUInterrupt, they need the whole emulator; the integration tests boot with them)

TEST_F(Z80Test, MemptrAfterLdAFromAddress)
{
    // the internal WZ register (MEMPTR); it shows up in the X/Y flags of BIT n,(HL)
    z80.load({0x3a, 0x34, 0x12}); // ld a,($1234)
    z80.step();
    EXPECT_EQ(z80.wz(), 0x1235u);
}

// DISABLED: ld (bc),a sets WZ to A << 8 in emuv1 (EmulatorTester.cpp even asserts that), the Z80 also puts
// (BC + 1) & 0xFF in the low byte (documented in 'MEMPTR, esoteric register of the Zilog Z80 CPU').
// emuv1/cpu/opcodes.inc, cases 0x02, 0x12 and 0x32 (ld (de),a and ld (nn),a do the same). Only visible in the
// X/Y flags of BIT n,(HL).
TEST_F(Z80Test, DISABLED_MemptrAfterLdBcFromA)
{
    // ld (bc),a: WZ low = (BC + 1) & 0xFF, WZ high = A
    z80.setBC(0x1234);
    z80.a() = 0x56;
    z80.load({0x02});
    z80.step();
    EXPECT_EQ(z80.ram[0x1234], 0x56);
    EXPECT_EQ(z80.wz(), 0x5635u);
}
