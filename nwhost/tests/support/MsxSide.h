//! MsxSide.h
#ifndef NWTEST_MSXSIDE_H
#define NWTEST_MSXSIDE_H

#include "NowindHost.hh"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <vector>

namespace nwtest {

using nwhost::byte;
using nwhost::word;

// the Z80 registers the ROM sends with a command (sendRegisters in msxsrc/common.asm)
struct Regs {
    byte a = 0, f = 0, b = 0, c = 0, d = 0, e = 0, h = 0, l = 0;

    Regs& setA(byte value)
    {
        a = value;
        return *this;
    }
    Regs& setF(byte value)
    {
        f = value;
        return *this;
    }
    Regs& setBC(word value)
    {
        b = byte(value >> 8);
        c = byte(value);
        return *this;
    }
    Regs& setDE(word value)
    {
        d = byte(value >> 8);
        e = byte(value);
        return *this;
    }
    Regs& setHL(word value)
    {
        h = byte(value >> 8);
        l = byte(value);
        return *this;
    }
};

// plays the MSX side of the protocol against a NowindHost: sends what the ROM sends and reads the replies
class MsxSide {
public:
    explicit MsxSide(nwhost::NowindHost& aHost)
        : host(aHost)
    {
    }

    // emulated time in ms, NowindHost resets to sync after a gap of 500 ms
    void advance(unsigned ms) { time += ms; }

    void send(byte value) { host.write(value, time); }
    void send(std::initializer_list<byte> values)
    {
        for (byte value : values) send(value);
    }
    void send(const std::vector<byte>& values)
    {
        for (byte value : values) send(value);
    }

    // AF 05 c b e d l h f a cmd, the frame sendRegisters + 'ld (hl),cmd' produce
    void sendCommand(const Regs& regs, byte command)
    {
        send({0xaf, 0x05, regs.c, regs.b, regs.e, regs.d, regs.l, regs.h, regs.f, regs.a, command});
    }

    bool hasData() const { return host.isDataAvailable(); }
    byte read() { return host.read(); }
    word read16()
    {
        word low = read();
        return low + 256 * read();
    }
    std::vector<byte> read(unsigned count)
    {
        std::vector<byte> result;
        for (unsigned i = 0; i < count; ++i) result.push_back(read());
        return result;
    }
    std::vector<byte> readAll()
    {
        std::vector<byte> result;
        while (hasData()) result.push_back(read());
        return result;
    }

    // every reply starts with FF AF 05 (Response::sendHeader)
    ::testing::AssertionResult readHeader()
    {
        if (!hasData()) return ::testing::AssertionFailure() << "no reply";
        std::vector<byte> header = read(3);
        if (header != std::vector<byte>{0xff, 0xaf, 0x05})
        {
            return ::testing::AssertionFailure() << "expected header FF AF 05, got " << ::testing::PrintToString(header);
        }
        return ::testing::AssertionSuccess();
    }

    nwhost::NowindHost& host;
    unsigned time = 0;
};

} // namespace nwtest

#endif // NWTEST_MSXSIDE_H
