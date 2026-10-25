//! Z80TestAccess.h
#ifndef Z80TESTACCESS_H
#define Z80TESTACCESS_H

#include "cpu/Z80.h"
#include "devices/SlotSelector.h"

#include <array>
#include <initializer_list>

// runs the emuv1 Z80 on 64 kB of plain RAM, without the slot selector and the other MSX hardware
// (a friend of Z80, like EmulatorTester)
class Z80TestAccess {
public:
    Z80TestAccess()
        : cpu(Z80::Instance())
    {
        ram.fill(0);
        // readMem/writeMem ask the slot selector about 0xFFFF and the nowind interface; unconfigured it has neither
        cpu->slotSelector = SlotSelector::Instance();
        for (unsigned block = 0; block < 8; ++block)
        {
            cpu->readBlock[block] = &ram[block * 0x2000];
            cpu->writeFunc[block].bind(this, &Z80TestAccess::write);
        }
        cpu->reg_a = cpu->reg_f = cpu->reg_b = cpu->reg_c = 0;
        cpu->reg_de = cpu->reg_hl = cpu->reg_ix = cpu->reg_iy = cpu->reg_wz = 0;
        cpu->shadow_af = cpu->shadow_b = cpu->shadow_c = cpu->shadow_de = cpu->shadow_hl = 0;
        cpu->reg_sp = 0xf000;
        cpu->reg_pc = 0xc000;
        cpu->IFF1 = cpu->IFF2 = false;
        cpu->emuTime = 0;
    }

    // puts code at the program counter
    void load(std::initializer_list<nw_byte> code, nw_word address = 0xc000)
    {
        for (nw_byte value : code) ram[address++] = value;
    }

    // executes one instruction, returns its number of cycles
    unsigned step()
    {
        emuTimeType before = cpu->emuTime;
        cpu->nextInterrupt = cpu->emuTime + 1; // executeInstructions always runs at least one instruction
        cpu->executeInstructions();
        return unsigned(cpu->emuTime - before);
    }

    void step(unsigned count)
    {
        while (count--) step();
    }

    // executes until the program counter reaches the address
    bool runUntil(nw_word address, unsigned maxInstructions = 100000)
    {
        while (maxInstructions--)
        {
            if (cpu->reg_pc == address) return true;
            step();
        }
        return false;
    }

    void write(nw_word address, nw_byte value) { ram[address] = value; }

    nw_word& a() { return cpu->reg_a; }
    nw_word& f() { return cpu->reg_f; }
    nw_word& b() { return cpu->reg_b; }
    nw_word& c() { return cpu->reg_c; }
    nw_word& de() { return cpu->reg_de; }
    nw_word& hl() { return cpu->reg_hl; }
    nw_word& ix() { return cpu->reg_ix; }
    nw_word& iy() { return cpu->reg_iy; }
    nw_word& sp() { return cpu->reg_sp; }
    nw_word& pc() { return cpu->reg_pc; }
    nw_word& wz() { return cpu->reg_wz; }
    nw_word& shadowAf() { return cpu->shadow_af; }
    nw_word bc() const { return (cpu->reg_b << 8) | cpu->reg_c; }
    void setBC(nw_word value)
    {
        cpu->reg_b = value >> 8;
        cpu->reg_c = value & 0xff;
    }
    bool iff1() const { return cpu->IFF1; }

    nw_word flagSZ(int i) const { return cpu->flagSZ[i]; }
    nw_word flagSZP(int i) const { return cpu->flagSZP[i]; }
    nw_word flagInc(int i) const { return cpu->flagInc[i]; }
    nw_word flagDec(int i) const { return cpu->flagDec[i]; }

    Z80* cpu;
    std::array<nw_byte, 0x10000> ram;
};

#endif // Z80TESTACCESS_H
