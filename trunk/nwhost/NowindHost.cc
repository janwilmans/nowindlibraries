#include "DiskHandler.hh"
#include "SectorMedium.hh"
#include <fstream>
#include <algorithm>
#include <cassert>
#include <ctype.h>
#include <time.h>
#include <iostream>

#define NWHOST_API_EXPORT
#include "NowindHost.hh"

/*
For debugging:

commands send for the msx to the host look like:

AF 05 cc bb ee dd ll hh ff aa CC

first the 2 header bytes AF 05 are sent, then all registers are send (8 bytes) and finally the command is sent (1 byte)
*/

#define DBERR debugMessage
#define PROTOCOL_V2

using std::string;
using std::vector;
using std::fstream;
using std::ios;

namespace nwhost {

NowindHost::NowindHost(const vector<DiskHandler*>& drives_)
	: drives(drives_)
	, lastTime(0)
	, state(STATE_SYNC1)
	, romdisk(1)
	, allowOtherDiskroms(true)
	, enablePhantomDrives(false)
	, enableMSXDOS2(false)
{
    vector<byte> requestWait;
    requestWait.push_back(1);
    requestWait.push_back(0);

    addStartupRequest(requestWait);
}

NowindHost::~NowindHost()
{
}


// send:  msx -> pc
void NowindHost::write(byte data, unsigned int time)
{
	unsigned duration = time - lastTime;
	lastTime = time;
	if ((duration >= 500) && (state != STATE_SYNC1)) {
		// timeout (500ms), start looking for AF05
        DBERR("Protocol timeout occurred in state %d, purge buffers and switch back to STATE_SYNC1\n", state);
		purge();
		state = STATE_SYNC1;
	}
    //DBERR("received: 0x%02x (in state: %d)\n", data, state);
	switch (state) {
	case STATE_SYNC1:
		if (data == 0xAF) state = STATE_SYNC2;
		break;
	case STATE_SYNC2:
		switch (data) {
		case 0x05: state = STATE_COMMAND; recvCount = 0; break;
		case 0xAF: state = STATE_SYNC2; break;
		case 0xFF: state = STATE_SYNC1; msxReset(); break;
		default:   state = STATE_SYNC1; break;
		}
		break;
	case STATE_COMMAND:
		assert(recvCount < 9);
		cmdData[recvCount] = data;
		if (++recvCount == 9) {
			executeCommand();
		}
		break;
	case STATE_DISKREAD:
		assert(recvCount < 2);
		extraData[recvCount] = data;
		if (++recvCount == 2) {
			doDiskRead2();
		}
		break;
	case STATE_DISKWRITE:
		assert(recvCount < (transferSize + 2));
		extraData[recvCount] = data;
		if (++recvCount == (transferSize + 2)) {
			doDiskWrite2();
		}
		break;
	case STATE_DEVOPEN:
		assert(recvCount < 11);
		extraData[recvCount] = data;
		if (++recvCount == 11) {
			deviceOpen();
		}
		break;
	case STATE_IMAGE:
		assert(recvCount < 40);
		extraData[recvCount] = data;
		if ((data == 0) || (data == ':') ||
		    (++recvCount == 40)) {
			char* data = reinterpret_cast<char*>(extraData);
			callImage(string(data, recvCount));
			state = STATE_SYNC1;
		}
		break;
	case STATE_MESSAGE:
		assert(recvCount < (240 - 1));
		extraData[recvCount] = data;
		if ((data == 0) || (++recvCount == (240 - 1))) {
			dumpRegisters();
			extraData[recvCount] = 0;
			DBERR("DBG MSG: %s\n", reinterpret_cast<char*>(extraData));
			state = STATE_SYNC1;
		}
		break;
	
	case STATE_BLOCKREAD:
		blockReadAck(data);
		break;

	default:
		assert(false);
	}
}

void NowindHost::executeCommand()
{
	assert(recvCount == 9);
	byte cmd = cmdData[8];
	switch (cmd) {
    /*
	case 0x0D: BDOS_0DH_DiskReset();
	case 0x0F: BDOS_0FH_OpenFile();
	case 0x10: BDOS_10H_CloseFile();
	case 0x11: BDOS_11H_FindFirst();
	case 0x12: BDOS_12H_FindNext();
	case 0x13: BDOS_13H_DeleteFile();
	case 0x14: BDOS_14H_ReadSeq();
	case 0x15: BDOS_15H_WriteSeq();
	case 0x16: BDOS_16H_CreateFile();
	case 0x17: BDOS_17H_RenameFile();
	case 0x21: BDOS_21H_ReadRandomFile();
	case 0x22: BDOS_22H_WriteRandomFile();
	case 0x23: BDOS_23H_GetFileSize();
	case 0x24: BDOS_24H_SetRandomRecordField();
	case 0x26: BDOS_26H_WriteRandomBlock();
	case 0x27: BDOS_27H_ReadRandomBlock();
	case 0x28: BDOS_28H_WriteRandomFileWithZeros();
	case 0x2A: BDOS_2AH_GetDate();
	case 0x2B: BDOS_2BH_SetDate();
	case 0x2C: BDOS_2CH_GetTime();
	case 0x2D: BDOS_2DH_SetTime();
	case 0x2E: BDOS_2EH_Verify();
	case 0x2F: BDOS_2FH_ReadLogicalSector();
	case 0x30: BDOS_30H_WriteLogicalSector();
    */

	case 0x80: { // DSKIO
		SectorMedium* disk = getDisk();
		if (!disk) {
			// no such drive or no disk inserted
			// (causes a timeout on the MSX side)
			state = STATE_SYNC1;
			return;
		}
		byte reg_f = cmdData[6];
		if (reg_f & 1) { // carry flag
			diskWriteInit(*disk);
		} else {
			diskReadInit(*disk);
		}
		break;
	}

	case 0x81: DSKCHG();      state = STATE_SYNC1; break;
	case 0x82: GETDPB();	  state = STATE_SYNC1; break;
	//case 0x83: CHOICE();
	//case 0x84: DSKFMT();
	case 0x85: DRIVES();      state = STATE_SYNC1; break;
	case 0x86: INIENV();      state = STATE_SYNC1; break;
	case 0x87: setDateMSX();  state = STATE_SYNC1; break;

	case 0x88: state = STATE_DEVOPEN; recvCount = 0; break;
	case 0x89: deviceClose(); state = STATE_SYNC1; break;
	//case 0x8A: deviceRandomIO(fcb);
	case 0x8B: deviceWrite(); state = STATE_SYNC1; break;
	case 0x8C: deviceRead();  state = STATE_SYNC1; break;
	//case 0x8D: deviceEof(fcb);
	case 0x8E: auxIn();       state = STATE_SYNC1; break;
	case 0x8F: auxOut();      state = STATE_SYNC1; break;
	case 0x90: state = STATE_MESSAGE; recvCount = 0; break;
	case 0x91: state = STATE_IMAGE;   recvCount = 0; break;

    case 0x92: getDosVersion(); state = STATE_SYNC1; break;
	case 0x93: commandRequested(); state = STATE_SYNC1; break;
	//case 0xFF: vramDump();
	case 0x94: blockReadCmd(); break;
    case 0x95: blockWrite(); break;
	default:
		// Unknown USB command!
		state = STATE_SYNC1;
		break;
	}
}


void NowindHost::diskReadInit(SectorMedium& disk)
{
    readRetries = 0;
	unsigned sectorAmount = getSectorAmount();
	buffer.resize(sectorAmount * 512);
	unsigned startSector = getStartSector();
    //DBERR("startSector: %u\n", startSector);
    if (disk.readSectors(&buffer[0], startSector, sectorAmount)) {
		// read error
		state = STATE_SYNC1;
		return;
	}

	transferred = 0;
	retryCount = 0;

#ifdef PROTOCOL_V2
    unsigned int size = sectorAmount * 512;
    unsigned address = getCurrentAddress();
    newBlockTransfer(address, size);
#else
    doDiskRead1();
#endif
}

void NowindHost::doDiskRead1()
{
	//DBERR("doDiskRead1\n");
    unsigned bytesLeft = unsigned(buffer.size()) - transferred;
	if (bytesLeft == 0) {
		sendHeader();
		send(0x01); // end of receive-loop
		send(0x00); // no more data
		state = STATE_SYNC1;
        DBERR("finished. (%d retries)\n", readRetries);
		return;
	}

    state = STATE_DISKREAD;

    static const unsigned NUMBEROFBLOCKS = 128; // 64 * 64 bytes = 4192 bytes
	transferSize = std::min(bytesLeft, NUMBEROFBLOCKS * 64); // hardcoded in firmware

	unsigned address = getCurrentAddress();
	if (address >= 0x8000) {
		if (transferSize & 0x003F) {
			transferSectors(address, transferSize);
		} else {
			transferSectorsBackwards(address, transferSize);
		}
	} else {
		// transfer below 0x8000
		unsigned endAddress = address + transferSize;
		if (endAddress <= 0x8000) {
			transferSectorsBackwards(address, transferSize);
		} else {
			transferSize = 0x8000 - address;
			transferSectors(address, transferSize);
		}
	}
	recvCount = 0;
}

void NowindHost::doDiskRead2()
{
	// diskrom sends back the last two bytes read
	//DBERR("doDiskRead2\n");
    assert(recvCount == 2);
	byte tail1 = extraData[0];
	byte tail2 = extraData[1];
	if ((tail1 == 0xAF) && (tail2 == 0x07)) {
		transferred += transferSize;
		retryCount = 0;

		unsigned address = getCurrentAddress();
		size_t bytesLeft = buffer.size() - transferred;
		if ((address == 0x8000) && (bytesLeft > 0)) {
			sendHeader();
			send(0x01); // end of receive-loop
			send(0xff); // more data for page 2/3
		}

		// continue the rest of the disk read
		doDiskRead1();
	} else {
		purge();
		if (++retryCount == 500) {
			// do nothing, timeout on MSX
			// too many retries, aborting readDisk()
            DBERR("Too many retries, returning to STATE_SYNC1\n");
			state = STATE_SYNC1;
			return;
		}

        readRetries++;
		doDiskRead1(); // try again
		recvCount = 0;
	}
}

// sends "02" + "transfer_addr" + "amount" + "data" + "0F 07"
void NowindHost::transferSectors(unsigned transferAddress, unsigned amount)
{
	sendHeader();
	send(0x00); // don't exit command, (more) data is coming
	send16(transferAddress);
	send16(amount);

	const byte* bufferPointer = &buffer[transferred];
	for (unsigned i = 0; i < amount; ++i) {
		send(bufferPointer[i]);
	}
    sendTrailer(); // used for validation
}

void NowindHost::sendTrailer()
{
	send(0xAF);
    /*
    static int failOnPurpose = 0;
    failOnPurpose++;
    if (failOnPurpose == 5)
    {
        // to test the protocols error-correction, make every 5th transfer fail :)
        failOnPurpose = 0;
        DBERR("Fail on purpose!\n");
    	send(0xFF); // screw up the transmission on purpose
    }
    */
    send(0x07);
}

 // sends "02" + "transfer_addr" + "amount" + "data" + "0F 07"
void NowindHost::transferSectorsBackwards(unsigned transferAddress, unsigned amount)
{
	DBERR("tranferAddr: 0x%04x  amount %d (retries: %u)\n", transferAddress, amount, readRetries);
	sendHeader();
	send(0x02); // don't exit command, (more) data is coming            // jan: doesn't '0x02' mean 'backwards' transfer
	send16(transferAddress + amount);
	send(amount / 64);

	const byte* bufferPointer = &buffer[transferred];
	for (int i = amount - 1; i >= 0; --i) {
		send(bufferPointer[i]);
	}
	sendTrailer(); // used for validation
}


void NowindHost::diskWriteInit(SectorMedium& disk)
{
	DBERR("diskWriteInit\n");
	if (disk.isWriteProtected()) {
		sendHeader();
		send(1);
		send(0); // WRITEPROTECTED
		state = STATE_SYNC1;
		return;
	}

	unsigned sectorAmount = std::min(128u, getSectorAmount());
	buffer.resize(sectorAmount * 512);
	transferred = 0;
	doDiskWrite1();
}

void NowindHost::doDiskWrite1()
{
	DBERR("doDiskWrite1\n");
	unsigned bytesLeft = unsigned(buffer.size()) - transferred;
	if (bytesLeft == 0) {
		// All data transferred!
		unsigned sectorAmount = unsigned(buffer.size()) / 512;
		unsigned startSector = getStartSector();
		if (SectorMedium* disk = getDisk()) {
			if (disk->writeSectors(&buffer[0], startSector, sectorAmount)) {
				// TODO write error
			}
		}
		sendHeader();
		send(255);
		state = STATE_SYNC1;
		return;
	}

	static const unsigned BLOCKSIZE = 240;
	transferSize = std::min(bytesLeft, BLOCKSIZE);

	unsigned address = getCurrentAddress();
	unsigned endAddress = address + transferSize;
	if ((address ^ endAddress) & 0x8000) {
		// would cross page 1-2 boundary -> limit to page 1
		transferSize = 0x8000 - address;
	}

	sendHeader();
	send(0);          // data ahead!
	send16(address);
	send16(transferSize);
	send(0xaa);

	// wait for data
	state = STATE_DISKWRITE;
	recvCount = 0;
}

void NowindHost::doDiskWrite2()
{
	DBERR("doDiskWrite2\n");
	assert(recvCount == (transferSize + 2));
	for (unsigned i = 0; i < transferSize; ++i) {
		buffer[i + transferred] = extraData[i + 1];
	}

	byte seq1 = extraData[0];
	byte seq2 = extraData[transferSize + 1];
	if ((seq1 == 0xaa) && (seq2 == 0xaa)) {
		// good block received
		transferred += transferSize;

		unsigned address = getCurrentAddress();
		size_t bytesLeft = buffer.size() - transferred;
		if ((address == 0x8000) && (bytesLeft > 0)) {
			sendHeader();
			send(254); // more data for page 2/3
		}
	} else {
		// ERROR!!!
		// This situation is still not handled correctly!
		purge();
	}

	// continue the rest of the disk write
	doDiskWrite1();
}

// quick and dirty split NowindHost.cc into more files, todo: create different classes

#include "NowindHostNewProtocol.hh"
#include "NowindHostSupport.hh"

} // namespace nowind
