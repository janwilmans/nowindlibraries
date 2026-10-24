# nwhost review

Review of `nwhost` (C++23), October 2026. It looks for bugs, undefined behaviour, memory safety, protocol errors and error handling. The host is checked against the MSX side of the protocol in the firmware ([`msxsrc/common.asm`](msxsrc/common.asm), [`nowindDriver.asm`](msxsrc/nowindDriver.asm), [`device.asm`](msxsrc/device.asm), [`extendedBios.asm`](msxsrc/extendedBios.asm), [`init.asm`](msxsrc/init.asm)).

**Not reviewed, because the shipped firmware never reaches them:**

- `BDOSProxy`: the firmware is built without `BDOS_NOWIND` ([`nowind.asm:5`](msxsrc/nowind.asm)), so the MSX never sends the BDOS commands.
- `STATE_BLOCKWRITE` / `BlockWrite`: `USE_OLD_DISKWRITE` is defined in `NowindHost.cc`, so the old disk write path is used.
- GETDATE: its only callers in the firmware are commented out ([`nowind.asm:148-149`](msxsrc/nowind.asm)).

`NwhostService::hostImage` handles USB and `nwhost` on a single thread, so there are no concurrency findings.

Line numbers refer to the sources at the time of the review. Every finding except 12 has been fixed since, each in its own commit; see the status column.

## Findings

| # | Severity | Where | Summary | Status |
|---|---|---|---|---|
| 1 | High | `nwhost/Image.cpp` | One failed read or write disables the drive | fixed, `07ef476` |
| 2 | High | `nwhost/NowindHost.cc` | `API_NOWMAP` data: unterminated string and buffer overflow | fixed, `a874d38` |
| 3 | High | `nwhost/NowindHost.cc`, `nwhost/Image.cpp` | NOWMAP: stack overflow, out-of-bounds partition table read, stale disk cache | fixed, `0e383d9` |
| 4 | Medium | `nwhost/NowindHost.cc` | A failed disk write is reported as success | fixed, `077fc56` |
| 5 | Medium | `nwhost/NowindHost.cc` | GETDPB divides by zero | fixed, `1347385` |
| 6 | Medium | `nwhost/BlockRead.cc` | 32 KB chunk: block count wraps to 0 | fixed, `586b4f7` |
| 7 | Medium | `nwhost/NowindHost.cc` | DSKIO read of 0 sectors | fixed, `d74c8ac` |
| 8 | Medium | `nwhost/Image.cpp` | 32-bit offsets on large hard disk images | fixed, `e08a7b4` |
| 9 | Medium | `msxsrc/msxdostools/nowmap.asm` | `NOWMAP` without arguments always fails | fixed, `da09715` |
| 10 | Low | `nwhost/BlockRead.cc` | Host and MSX disagree on a transfer starting at `$8000` | fixed, `77d5cdf` |
| 11 | Low | `nwhost/Device.cc` | `Device::read` leaves out the `0xFF` dummy byte of the header | fixed, `8b8e184` |
| 12 | Low | `nwhost/Image.cpp` | A failed `CALL IMAGE` ejects the current disk | not changed: intended behaviour |
| 13 | Low | `nwhost/NowindHost.cc` | GETDPB rounds the directory size down | fixed, `47f930e` |
| 14 | Low | `nwhost/Image.cpp` | A missing hard disk image gives garbage partitions | fixed, `23cadb4` |
| 15 | High | `nwhost/NowindHost.cc` | `C_COMMAND`: the data of an API command is never received | found while fixing 2, fixed, `47594e7` |

## High

### 1. One failed read or write disables the drive

**Where:** [`nwhost/Image.cpp:165`](nwhost/Image.cpp) (`readSectors`), [`:189`](nwhost/Image.cpp) (`writeSectors`)

After a failed `read` or `write`, the stream keeps `failbit` set, and nothing calls `stream->clear()`. Since C++11, `seekg`/`seekp` and the following read or write do nothing while `failbit` is set.

**Impact:** truncated `.dsk` images are common, and with them DOS reads a sector past the end of the file. That read fails, and so does every DSKIO, DSKCHG media read and GETDPB on that drive afterwards. The MSX sees *Disk offline* until the image is inserted again.

**Fix:** call `stream->clear()` before each seek. Optionally check the request against `fileSize`/`length` first.

### 2. `API_NOWMAP` data: unterminated string and buffer overflow

**Where:** [`nwhost/NowindHost.cc:561`](nwhost/NowindHost.cc), [`:577`](nwhost/NowindHost.cc) (`apiReceiveData`)

The MSX sends exactly B bytes with `LDIR`, with no terminating zero (`sendCommandWithData` in `extendedBios.asm`). The host builds the arguments with `string(reinterpret_cast<char*>(&extraData[0]))`. That reads until it finds a zero, so it takes in stale bytes left by earlier commands. If none of the 242 bytes is zero, it reads past the end of the vector.

The bytes are also written with `extraData[recvCount] = data` for `recvCount < reg_b`. B can be up to 255, while `extraData` holds 242 bytes. The firmware documents B as "max 240", but neither side checks it.

**Impact:** NOWMAP gets wrong arguments: "no 2 arguments found", or the wrong partition. Any MSX program that uses EXTBIO function 4 can corrupt the host's heap.

**Fix:**

- Use `string(reinterpret_cast<char*>(&command.extraData[0]), recvCount)`.
- Reject `reg_b > 240` in `apiCommand`, or size `extraData` to 256.

### 3. NOWMAP: stack overflow, out-of-bounds partition table read, stale disk cache

**Where:** [`nwhost/NowindHost.cc:607-661`](nwhost/NowindHost.cc) (`nowMap`), [`nwhost/Image.cpp:87`](nwhost/Image.cpp), [`:197-201`](nwhost/Image.cpp) (`SetActivePartition`, `ReadPartitionInfo`)

There are four problems in this path:

- **Stack overflow.** `sprintf` writes `image->getDescription()` (the full image path, of any length) into `char temp[250]`.
- **Out-of-bounds read.** The partition number comes from `atoi` on MSX input and is not checked. `tableEntry = 0x1EE - partitionNumber*16` wraps for partition numbers of 31 and up, and the code reads the 512-byte `mbr` buffer out of bounds. Every call also leaks `mbr`.
- **Uninitialized flag.** `Image::harddiskPartition` is not set in the constructor ([`Image.cpp:16`](nwhost/Image.cpp)) and not reset in `openDiskImage`. `isHarddiskPartition()` therefore reads an uninitialized `bool`. A floppy inserted with `CALL IMAGE` after a hard disk partition still counts as a partition.
- **Stale disk cache.** `SetActivePartition` changes `offset`/`length` but does not set `diskChanged`, so the MSX keeps the FAT and DPB of the previous partition. Its next writes put that FAT into the new partition.

**Impact:** a long image path, or `NOWMAP 0 40`, corrupts the host's stack or reads its heap out of bounds. Switching partitions corrupts the file system of the hard disk image.

**Fix:**

- Use `snprintf`, or build the strings with `std::string`.
- Check the partition number (for example 0 to 29) and that its entry is valid before switching.
- Initialize `harddiskPartition` (and `offset`, `length`, `fileSize`, `partitionNr`) in the constructor, and reset it in `openDiskImage`.
- Set `diskChanged = true` in `SetActivePartition`.
- Use a `std::vector` for `mbr`.

## Medium

### 4. A failed disk write is reported as success

**Where:** [`nwhost/NowindHost.cc:762-768`](nwhost/NowindHost.cc) (`doDiskWrite1`)

If `writeSectors` fails, through an I/O error or the failbit of finding 1, the host only logs it. It still replies `255` (done).

**Impact:** the MSX believes the write succeeded, so the data is lost silently.

**Fix:** reply with the error frame that `blockWrite01`/`blockWrite23` already handle: header, `1`, error code `10` (write fault).

### 5. GETDPB divides by zero

**Where:** [`nwhost/NowindHost.cc:959-962`](nwhost/NowindHost.cc)

With `bytesPerSector == 0` or `sectorsPerCluster == 0` in the boot sector, the host divides by zero. The MSX only asks the host for a DPB for media descriptor `F0` (`GETDPB.makeDPB` in `nowindDriver.asm`). So this happens for a hard disk or `F0` image whose boot sector is zero or corrupt.

**Impact:** the host crashes (SIGFPE).

**Fix:** if either value is 0, or `bytesPerSector` is not 512, send no reply (the MSX times out) or send an error.

### 6. 32 KB chunk: block count wraps to 0

**Where:** [`nwhost/BlockRead.cc:95`](nwhost/BlockRead.cc), against [`msxsrc/common.asm:466`](msxsrc/common.asm)

`byte blocks = size / 128` wraps to 0 for a chunk of 32 KB. That is a DSKIO read of 64 sectors or more starting at `0x0000` or `0x8000`. The host then sends `FASTTRANSFER` with 0 blocks. The MSX loads the count with `ld b,(hl)` and counts it down with `dec b`, so it waits for 256 blocks.

**Impact:** the MSX reads garbage into memory or hangs. On the host the queue is empty, so `ack()` reads `dataBlockQueue[0]` of an empty deque (undefined behaviour).

**Fix:** split chunks into at most 255 blocks (32640 bytes).

### 7. DSKIO read of 0 sectors

**Where:** [`nwhost/NowindHost.cc:677-679`](nwhost/NowindHost.cc) (`diskReadInit`), `BlockRead::blockReadHelper`

A DSKIO read with B = 0 does `&buffer[0]` on an empty vector, then sends a slow transfer of size 0. On the MSX, `slowTransfer` ([`common.asm:528`](msxsrc/common.asm)) does `ldir` with BC = 0, which copies 65536 bytes.

**Impact:**

- **On the MSX:** it overwrites its whole memory with bytes read from the USB port and crashes.
- **On the host:** undefined behaviour.

**Fix:** for B = 0, reply straight away with `BLOCKREAD_EXIT`, or never send a transfer of size 0.

### 8. 32-bit offsets on large hard disk images

**Where:** [`nwhost/Image.cpp:71`](nwhost/Image.cpp), [`:87`](nwhost/Image.cpp), [`:155`](nwhost/Image.cpp), [`:174`](nwhost/Image.cpp), and `fileSize = tellg()` in `reOpenDiskImage`

Byte offsets are computed in `unsigned int` and wrap at 4 GB.

**Impact:** on a hard disk image larger than 4 GB, partitions that start beyond 4 GB read and write the wrong sectors, which corrupts data.

**Fix:** use `std::uint64_t` / `std::streamoff` for offsets and sizes.

### 9. `NOWMAP` without arguments always fails

**Where:** [`msxsrc/msxdostools/nowmap.asm:15-23`](msxsrc/msxdostools/nowmap.asm) (MSX side)

With an empty command line, the tool points HL at `list` ("/L") but sets `ld a,3`, and the `xor a` that follows overwrites it. The length is in B, and B stays 0. The host rejects a command with no data (`apiCommand`, "without commandline") and does not reply, so the following `readData` times out.

**Impact:** `NOWMAP` without arguments prints "No connection with host!" instead of the drive list.

**Fix:** `ld b,2` (the length of "/L") instead of `ld a,3`.

## Low

### 10. Host and MSX disagree on a transfer starting at `$8000`

**Where:** [`nwhost/BlockRead.cc:38`](nwhost/BlockRead.cc)

The host sets `transferingToPage01 = (startAddress <= 0x8000)`. The MSX chooses its mode by bit 7 of the address ([`common.asm:384`](msxsrc/common.asm), `rlca`), so at `0x8000` it starts in page 2/3 mode. If such a transfer needs a second chunk, which happens when its size is not a multiple of 128, the host sends `BLOCKREAD_EXIT_MORE_DATE_AHEAD` (0). `blockRead23` treats 0 as the end of the transfer.

**Impact:** the rest of the data is lost, and host and MSX lose sync. Today this is only reachable through the EXTBIO API with an odd size; DSKIO sizes are multiples of 512.

**Fix:** use `<` instead of `<=`.

### 11. `Device::read` leaves out the `0xFF` dummy byte of the header

**Where:** [`nwhost/Device.cc:122`](nwhost/Device.cc)

`Device::read` sends `AF 05` without the dummy `0xFF` that `Response::sendHeader` puts in front of every other reply. According to the comment in `sendHeader`, that byte is there because the "first read might fail (hardware design choise)".

**Impact:** if the MSX's first read fails, it misses the header, and a `NOW:` read ends with *Device I/O error*.

**Fix:** use `nwhSupport->sendHeader()` followed by the dummy `0x00`, as `Device::open` does.

### 12. A failed `CALL IMAGE` ejects the current disk

**Where:** [`nwhost/Image.cpp:33-40`](nwhost/Image.cpp) (`openDiskImage`, called from `NowindHost::callImage`)

The current stream is deleted, and `containsDisk` cleared, before the new file is opened.

**Impact:** a typo in `CALL IMAGE` leaves the drive empty, under a program that may still be running from it.

**Fix:** open the new file first, and swap it in only when that succeeds.

**Status:** not changed. Ejecting the disk when `CALL IMAGE` fails is the intended behaviour.

### 13. GETDPB rounds the directory size down

**Where:** [`nwhost/NowindHost.cc:960`](nwhost/NowindHost.cc)

The directory size is computed as `rootDirEntries / 16` in integer arithmetic. That is only correct for 512-byte sectors, and it rounds down where it should round up; the code already has a `TODO: should be rounded up!?` there.

**Impact:** when the number of root entries is not a multiple of 16, FIRREC and MAXCLUS are one sector off, so data sectors are read and written in the wrong place.

**Fix:** `(rootDirEntries * 32 + bytesPerSector - 1) / bytesPerSector`.

### 14. A missing hard disk image gives garbage partitions

**Where:** [`nwhost/Image.cpp:226`](nwhost/Image.cpp) (`GetNewPartitionTable`), used by `NowindHost_InsertHarddiskPartitions` in [`nwhost/nowindusb.cpp`](nwhost/nowindusb.cpp)

When the image cannot be opened, the read fails, and the uninitialized buffer from `new nw_byte[512]` is parsed as the partition table.

**Impact:** a wrong path inserts random "partitions".

**Fix:** check that opening and reading succeeded, and return no partitions otherwise.

## Found while fixing

### 15. `C_COMMAND`: the data of an API command is never received

**Where:** `NowindHost::executeCommand` in [`nwhost/NowindHost.cc`](nwhost/NowindHost.cc)

For `C_COMMAND`, `executeCommand` calls `apiCommand()`, which switches to `STATE_RECEIVE_DATA`. `executeCommand` then ends with `setState(nextState)`, and `nextState` is still its default `STATE_SYNC1`. So the command line of `NOWMAP`, and the data of any other EXTBIO API command, was never received, and the MSX timed out waiting for the reply.

**Impact:** `NOWMAP` never worked, with or without arguments.

**Fix:** `executeCommand` returns right after `apiCommand()`, which sets the next state itself.

## Parts of the protocol that match

The host and the MSX side agree on:

- **Header framing:** `FF AF 05`, which `getHeaderInPage0`/`getHeaderInPage2` look for.
- **Block reads:** `blockRead01`/`blockRead23` codes 0 to 3, fast and slow transfers, and resending a failed block.
- **Disk writes:** the request (address, size, sequence number), `255` for done, `254` to continue in page 2/3, and `1, 0` for write protected.
- **DSKCHG:** the changed flag followed by the media descriptor.
- **GETDPB:** the destination address followed by 18 bytes.
- **DRIVES:** phantom drives, then the flag byte, then the number of drives.
- **INIENV:** the drive number of the ROM disk.
- **GETDOSVERSION:** `1` for DOS1, `2` for DOS2.
- **AUXIN:** two headers.
- **`NOW:` device:** open and read framing.
- **`CHANGEIMAGE`:** the name ends with `0` or `:`.
