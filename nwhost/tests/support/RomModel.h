//! RomModel.h
// C++ models of the nowind ROM's transfer routines in msxsrc/common.asm, byte for byte as the Z80 code reads and
// writes the USB fifo. The host side is tested against these, so a protocol change on either side shows up as wrong
// memory contents or a hanging transfer instead of a silently different byte stream.
#ifndef NWTEST_ROMMODEL_H
#define NWTEST_ROMMODEL_H

#include "NowindTypes.hh"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

namespace nwtest {

// the MSX memory, 64 kB
struct MsxMemory {
    std::array<nwhost::byte, 0x10000> bytes{};

    std::vector<nwhost::byte> range(unsigned address, unsigned size) const
    {
        return std::vector<nwhost::byte>(bytes.begin() + address, bytes.begin() + address + size);
    }
};

// the USB fifo as the ROM sees it: reads from the host (0xFF when empty) and writes to the host
struct UsbPort {
    std::function<nwhost::byte()> read;
    std::function<void(nwhost::byte)> write;
};

// what the ROM's routines return, see the comments in common.asm
struct RomResult {
    bool timeout = false; // carry set by getHeaderInPage0/2
    nwhost::byte code = 0;
    unsigned fastBlocks = 0;
    unsigned slowBlocks = 0;
    unsigned resyncs = 0; // .errorInPage1/2 loops: the tail did not match the header
};

class RomModel {
public:
    explicit RomModel(UsbPort aPort)
        : port(std::move(aPort))
    {
    }

    // getHeaderInPage0/2: wait for AF 05 (b = HIGH 65535, so about 65000 reads), returns the first byte after it
    std::optional<nwhost::byte> getHeader()
    {
        unsigned count = 0xff00;
        nwhost::byte a = in();
        while (true)
        {
            if (a == 0xaf)
            {
                a = in(); // .chk05
                if (a == 0x05) return in();
                continue; // jr nz,.chkaf: this byte is checked for AF again
            }
            if (--count == 0) return std::nullopt;
            a = in();
        }
    }

    // blockRead: 'rlca / jr c,.page23', blockRead01 until it returns 0 (more data for page 2/3), then blockRead23
    RomResult blockRead(nwhost::word address)
    {
        RomResult result;
        if (address < 0x8000)
        {
            if (!blockReadLoop(false, result)) return result; // done, timeout or an error code
        }
        blockReadLoop(true, result);
        return result;
    }

    // blockWrite: blockWrite01, 'ret pe' after it (parity even: 0xFF = done, 0xFE = continue in page 2/3), blockWrite23
    RomResult blockWrite(nwhost::word address)
    {
        RomResult result;
        if (address < 0x8000)
        {
            blockWriteLoop(result);
            if (result.timeout || isWriteError(result.code)) return result;
            if (evenParity(result.code)) return result; // ret pe
        }
        blockWriteLoop(result);
        return result;
    }

    static bool evenParity(nwhost::byte value)
    {
        int bits = 0;
        for (int i = 0; i < 8; ++i) bits += (value >> i) & 1;
        return (bits & 1) == 0;
    }

    // blockWrite01/23: 'or a / ret m / jr nz,.error', codes 1..127 are errors
    static bool isWriteError(nwhost::byte code) { return code != 0 && code < 0x80; }

    MsxMemory memory;
    UsbPort port;
    nwhost::byte lastErrorCode = 0; // the byte after an error code in blockWrite (ld a,(hl) in .error)

    // reads after which the model gives up; the ROM itself would hang (or time out in getHeader)
    unsigned long readBudget = 10'000'000;

private:
    nwhost::byte in()
    {
        if (readBudget-- == 0) throw std::runtime_error("ROM model: transfer makes no progress");
        return port.read();
    }

    // blockRead01 (page23 == false) and blockRead23; returns true when blockRead01 exits with 0 (more data for page 2/3)
    bool blockReadLoop(bool page23, RomResult& result)
    {
        while (true)
        {
            std::optional<nwhost::byte> a = getHeader();
            if (!a)
            {
                result.timeout = true;
                return false;
            }
            result.code = *a;
            if (*a == 1)
            {
                fastTransfer(result);
            }
            else if (*a == 0 && !page23)
            {
                return true; // and a / ret z
            }
            else if (*a == 2)
            {
                slowTransfer(result);
            }
            else
            {
                return false; // ret nz: return code 2-255 (blockRead23 also exits on 0)
            }
        }
    }

    // blockReadTranfer: the end address, the number of 128 byte blocks, then per block header, 64 x (d,e) pushed
    // from the end address downwards, tail; the tail is echoed to the host as the ack
    void fastTransfer(RomResult& result)
    {
        nwhost::word sp = in();
        sp |= in() << 8;
        nwhost::byte blocks = in();
        do
        {
            nwhost::byte header = in();
            while (header == 0xff) header = in(); // invalidHeader: skip 0xFF
            for (int i = 0; i < 64; ++i)
            {
                nwhost::byte d = in();
                nwhost::byte e = in();
                sp = (sp - 1) & 0xffff;
                memory.bytes[sp] = d;
                sp = (sp - 1) & 0xffff;
                memory.bytes[sp] = e;
            }
            nwhost::byte tail = in();
            port.write(tail);
            if (tail != header)
            {
                // .errorInPage1/2: read until the header value comes by, then go on with the next block
                ++result.resyncs;
                while (in() != header) {}
            }
            ++result.fastBlocks;
        } while (--blocks != 0);
    }

    // slowTransfer: address, length, header, the data (ldir), tail; the caller echoes the tail
    void slowTransfer(RomResult& result)
    {
        nwhost::word address = in();
        address |= in() << 8;
        unsigned length = in();
        length |= in() << 8;
        in(); // header
        if (length == 0) length = 0x10000; // ldir with bc = 0
        for (unsigned i = 0; i < length; ++i) memory.bytes[(address + i) & 0xffff] = in();
        nwhost::byte tail = in();
        port.write(tail);
        ++result.slowBlocks;
    }

    // blockWrite01/23: per request address, length, sequence number; the ROM sends seq, the data, seq
    void blockWriteLoop(RomResult& result)
    {
        while (true)
        {
            std::optional<nwhost::byte> a = getHeader();
            if (!a)
            {
                result.timeout = true;
                return;
            }
            result.code = *a;
            if (*a >= 0x80) return; // ret m
            if (*a != 0)
            {
                lastErrorCode = in(); // .error: scf / ld a,(hl)
                return;
            }
            nwhost::word address = in();
            address |= in() << 8;
            unsigned length = in();
            length |= in() << 8;
            nwhost::byte sequence = in();
            port.write(sequence);
            for (unsigned i = 0; i < length; ++i) port.write(memory.bytes[(address + i) & 0xffff]);
            port.write(sequence);
        }
    }
};

} // namespace nwtest

#endif // NWTEST_ROMMODEL_H
