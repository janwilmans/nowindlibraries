
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

first the 2 header bytes AF 05 are send, then all registers are send (8 bytes) and finally the command is send (1 byte)
*/

#define DBERR debugMessage

using std::string;
using std::vector;
using std::fstream;
using std::ios;

namespace nwhost {

NowindHost::NowindHost(const vector<DiskHandler*>& drives_)
	: drives(drives_)
	, lastTime(0)
	, state(STATE_SYNC1)
	, romdisk(255)
	, allowOtherDiskroms(true)
	, enablePhantomDrives(false)
	, enableMSXDOS2(false)
{
}

NowindHost::~NowindHost()
{
}

void NowindHost::debugMessage(const char *, ...)
{
}

void NowindHost::setAllowOtherDiskroms(bool allow)
{
	allowOtherDiskroms = allow;
}
bool NowindHost::getAllowOtherDiskroms() const
{
	return allowOtherDiskroms;
}

void NowindHost::setEnablePhantomDrives(bool enable)
{
	enablePhantomDrives = enable;
}
bool NowindHost::getEnablePhantomDrives() const
{
	return enablePhantomDrives;
}

void NowindHost::setEnableMSXDOS2(bool enable)
{
	enableMSXDOS2 = enable;
}

byte NowindHost::peek() const
{
	return isDataAvailable() ? hostToMsxFifo.front() : 0xFF;
}

// receive:  msx <- pc
byte NowindHost::read()
{
	if (!isDataAvailable()) {
		return 0xff;
	}
	byte result = hostToMsxFifo.front();
	hostToMsxFifo.pop_front();
	return result;
}

bool NowindHost::isDataAvailable() const
{
	return !hostToMsxFifo.empty();
}


// send:  msx -> pc
void NowindHost::write(byte data, unsigned time)
{
	unsigned duration = time - lastTime;
	lastTime = time;
	if (duration >= 500) {
		// timeout (500ms), start looking for AF05
		purge();
		state = STATE_SYNC1;
	}

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
	default:
		assert(false);
	}
}

void NowindHost::msxReset()
{
	for (unsigned i = 0; i < MAX_DEVICES; ++i) {
		devices[i].fs.reset();
	}
	DBERR("MSX reset\n");
}

SectorMedium* NowindHost::getDisk()
{
	byte num = cmdData[7]; // reg_a
	if (num >= drives.size()) {
		DBERR("MSX requested non-existing drive, reg_a: 0x%02x (ignored)\n", num);
		return 0;
	}
	return drives[num]->getSectorMedium();
}

void NowindHost::executeCommand()
{
	assert(recvCount == 9);
	byte cmd = cmdData[8];
	switch (cmd) {
	//case 0x0D: BDOS_0DH_DiskReset();
	//case 0x0F: BDOS_0FH_OpenFile();
	//case 0x10: BDOS_10H_CloseFile();
	//case 0x11: BDOS_11H_FindFirst();
	//case 0x12: BDOS_12H_FindNext();
	//case 0x13: BDOS_13H_DeleteFile();
	//case 0x14: BDOS_14H_ReadSeq();
	//case 0x15: BDOS_15H_WriteSeq();
	//case 0x16: BDOS_16H_CreateFile();
	//case 0x17: BDOS_17H_RenameFile();
	//case 0x21: BDOS_21H_ReadRandomFile();
	//case 0x22: BDOS_22H_WriteRandomFile();
	//case 0x23: BDOS_23H_GetFileSize();
	//case 0x24: BDOS_24H_SetRandomRecordField();
	//case 0x26: BDOS_26H_WriteRandomBlock();
	//case 0x27: BDOS_27H_ReadRandomBlock();
	//case 0x28: BDOS_28H_WriteRandomFileWithZeros();
	//case 0x2A: BDOS_2AH_GetDate();
	//case 0x2B: BDOS_2BH_SetDate();
	//case 0x2C: BDOS_2CH_GetTime();
	//case 0x2D: BDOS_2DH_SetTime();
	//case 0x2E: BDOS_2EH_Verify();
	//case 0x2F: BDOS_2FH_ReadLogicalSector();
	//case 0x30: BDOS_30H_WriteLogicalSector();

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
	default:
		// Unknown USB command!
		state = STATE_SYNC1;
		break;
	}
}

void NowindHost::auxIn()
{
	char input;
	DBERR("auxIn\n");
	sendHeader();

	dumpRegisters();
	std::cin >> input;

	sendHeader();
	send(input);
    DBERR("auxIn returning 0x%02x\n", input);
}

void NowindHost::auxOut()
{
	DBERR("auxOut: %c\n", cmdData[7]);
	dumpRegisters();
	printf("%c", cmdData[7]);
}

// the MSX asks whether the host has a command  
// waiting for it to execute
void NowindHost::commandRequested()
{
    sendHeader();
    if (requestQueue.empty())
    {
        send(0);
    }
    else
    {
        std::vector<byte> command = requestQueue.front();
        requestQueue.pop_front();
		if (requestQueue.empty())
		{
			send(0);
		}
		else
		{
			send(1);
		}

        for (unsigned int i=0;i<requestQueue.size();i++)
	    {
            send(command[i]);
	    }
    }
}

void NowindHost::clearRequests()
{
    requestQueue.clear();
}

void NowindHost::addRequest(std::vector<byte> command)
{
    requestQueue.push_back(command);
}

void NowindHost::dumpRegisters()
{
	//reg_[cbedlhfa] + cmd
	DBERR("AF: 0x%04X, BC: 0x%04X, DE: 0x%04X, HL: 0x%04X, CMD: 0x%02X\n", cmdData[7] * 256 + cmdData[6], cmdData[1] * 256 + cmdData[0], cmdData[3] * 256 + cmdData[2], cmdData[5] * 256 + cmdData[4], cmdData[8]);
}

// send:  pc -> msx
void NowindHost::send(byte value)
{
	hostToMsxFifo.push_back(value);
}
void NowindHost::send16(word value)
{
	hostToMsxFifo.push_back(value & 255);
	hostToMsxFifo.push_back(value >> 8);
}

void NowindHost::purge()
{
	hostToMsxFifo.clear();
}

void NowindHost::sendHeader()
{
	send(0xFF); // needed because first read might fail (hardware design choise)!
	send(0xAF);
	send(0x05);
}

void NowindHost::DSKCHG()
{
	SectorMedium* disk = getDisk();
	if (!disk) {
		// no such drive or no disk inserted
		return;
	}

	sendHeader();
	byte num = cmdData[7]; // reg_a
	assert(num < drives.size());
	if (drives[num]->diskChanged()) {
		send(255); // changed
		// read first FAT sector (contains media descriptor)
		byte sectorBuffer[512];
		if (disk->readSectors(sectorBuffer, 1, 1)) {
			// TODO read error
			sectorBuffer[0] = 0;
		}
		send(sectorBuffer[0]); // new mediadescriptor
	} else {
		send(0); // not changed
		// TODO shouldn't we send some (dummy) byte here?
		//      nowind-diskrom seems to read it (but doesn't use it)
	}
}

void NowindHost::GETDPB()
{
	byte num = cmdData[7]; // reg_a

	DBERR("GETDPB, reg_a: 0x%02X\n", num);
	SectorMedium* disk = getDisk();
	if (!disk) {
		// no such drive or no disk inserted
		DBERR("GETDPB error no disk\n");
		return;
	}

	byte sectorBuffer[512];
	if (disk->readSectors(sectorBuffer, 0, 1)) {
		// TODO read error
		sectorBuffer[0] = 0;
		DBERR("GETDPB error reading sector 0\n");
	}

	// the actual dpb[0] (drive number) is not send
	dpbType dpb;
	word sectorSize = sectorBuffer[12]*256+sectorBuffer[11];	// normally 512 bytes per sector, 4 sectors per cluster

	dpb.ID = sectorBuffer[21];	   	         // offset 1 = 0xF0;  
	dpb.SECSIZ_L = sectorSize & 0xff;	     // offset 2 = 0x00;  
	dpb.SECSIZ_H = sectorSize >> 8;		     // offset 3 = 0x02;  
	dpb.DIRMSK = (sectorSize/32)-1;	         // offset 4 = 0x0F, (SECSIZE/32)-1

	byte dirShift;
	for(dirShift=0;dpb.DIRMSK & (1<<dirShift);dirShift++) {}

	dpb.DIRSHFT = dirShift;		             // offset 5 = 0x04, nr of 1-bits in DIRMSK
	dpb.CLUSMSK = sectorBuffer[13]-1;        // offset 6 = 0x03, nr of (sectors/cluster)-1

	byte cluShift;
	for(cluShift=0;dpb.CLUSMSK & (1<<cluShift);cluShift++) {}

	dpb.CLUSSHFT = cluShift+1;            	 // offset 7 = 0x03, nr of bits in clusterMask+1 

	word firstFATsector = sectorBuffer[15]*256+sectorBuffer[14];

	dpb.FIRFAT_L = firstFATsector & 0xFF;    // offset 8 = 0x01, sectornumber of first FAT (normally just the bootsector is reserved)
	dpb.FIRFAT_H = firstFATsector >> 8;      // offset 9 = 0x00, idem 

	if (firstFATsector != 1) {
		// todo: notice when this happens
	}

	dpb.FATCNT = sectorBuffer[16];     	     // offset 10 = 0x02, number of FATs

	byte maxEnt = 254;						
	word rootDIRentries = sectorBuffer[18]*256+sectorBuffer[17];
	if (rootDIRentries < 255) maxEnt = rootDIRentries;

	dpb.MAXENT = maxEnt;              	     // offset 11 = 0x00;  // we come up with 0xFE here, why?

	word sectorsPerFAT = sectorBuffer[23]*256+sectorBuffer[22];
	if (sectorsPerFAT > 255)
	{
		//todo: notice when this happens
	}
	word firstDIRsector = firstFATsector + (dpb.FATCNT * sectorsPerFAT);

	// the data of the disk starts at the firstDIRsector + size of the directory area
	// (the "directory" area contains max. 254 entries of 16 bytes, one entry of each file)
	word firstRecord = firstDIRsector+(maxEnt/(sectorSize/32));

	dpb.FIRREC_L = 0x21; //firstDIRsector & 0xFF;    // offset 12 = 0x21, number of first data sector
	dpb.FIRREC_H = 0; // firstDIRsector >> 8;      // offset 13 = 0x0, idem 
	
	// maxClus is the number of clusters of drive not including reserved sector, 
	// fat sectors or directory sectors, see p260 of Msx Redbook

	// bigSectors is only used for the F0 media type, it is a 32bit entry
	// for the total amount of sectors on disk
	unsigned int bigSectors = sectorBuffer[35]*256*256*256+sectorBuffer[34]*256*256+sectorBuffer[33]*256+sectorBuffer[32];
	DBERR("bigSectors: %u\n", bigSectors);		//  we come up with sectorBuffer[18] here???

	word sectorsPerCluster = sectorBuffer[13];
	word maxClus = ((bigSectors-firstRecord)/sectorsPerCluster)+1;

	dpb.MAXCLUS_L = maxClus & 0xFF;          // offset 14 = 0xF8, highest cluster number
	dpb.MAXCLUS_H = maxClus >> 8;            // offset 15 = 0x9, idem
	dpb.FATSIZ = sectorBuffer[22];           // offset 16 = 0x8, number of sectors/FAT	 

	dpb.FIRDIR_L = firstDIRsector & 0xFF;    // offset 17 = 0x11;
	dpb.FIRDIR_H = firstDIRsector >> 8;      // offset 18 = 0x00; 

	// We dont know what sectorBuffer 0x1C-1F contains on MSX harddisk images 

 	byte dpb_pre[18];
	dpb_pre[0] = 0xF0;
	dpb_pre[1] = 0x00;
	dpb_pre[2] = 0x02;
	dpb_pre[3] = 0x0F;
	dpb_pre[4] = 0x04;
	dpb_pre[5] = 0x03;
	dpb_pre[6] = 0x03;
	dpb_pre[7] = 0x01;
	dpb_pre[8] = 0x00;
	dpb_pre[9] = 0x02;
	dpb_pre[10] = 0x00;
	dpb_pre[11] = 0x21;
	dpb_pre[12] = 0x0;
	dpb_pre[13] = 0xF8;
	dpb_pre[14] = 0x9;
	dpb_pre[15] = 0x8;
	dpb_pre[16] = 0x11;
	dpb_pre[17] = 0x00;

	sendHeader();
	// send dest. address
	send(cmdData[2]);	// reg_e
	send(cmdData[3]);	// reg_d
	byte * refData = (byte *) &dpb_pre;
	byte * sendBuffer = (byte *) &dpb;

	for (int i=0;i<18;i++) {
		DBERR("GETDPB offset [%d]: 0x%02X, correct: 0x%02X\n", i+1, sendBuffer[i], refData[i]);
		send(sendBuffer[i]);
	}
}


void NowindHost::DRIVES()
{
	// at least one drive (MSXDOS1 cannot handle 0 drives)
	byte numberOfDrives = std::max<byte>(1, byte(drives.size()));

	byte reg_a = cmdData[7];
	sendHeader();
	send(getEnablePhantomDrives() ? 0x02 : 0);
	send(reg_a | (getAllowOtherDiskroms() ? 0 : 0x80));
	send(numberOfDrives);

	romdisk = 255; // no romdisk
	for (unsigned i = 0; i < drives.size(); ++i) {
		if (drives[i]->isRomdisk()) {
			romdisk = i;
			break;
		}
	}
}

void NowindHost::INIENV()
{
	sendHeader();
	send(romdisk); // calculated in DRIVES()
}

void NowindHost::setDateMSX()
{
	time_t td = time(NULL);
	struct tm* tm = localtime(&td);

	sendHeader();
	send(tm->tm_mday);          // day
	send(tm->tm_mon + 1);       // month
	send16(tm->tm_year + 1900); // year
}


unsigned NowindHost::getSectorAmount() const
{
	byte reg_b = cmdData[1];
	return reg_b;
}
unsigned NowindHost::getStartSector() const
{
	byte reg_c = cmdData[0];
	byte reg_e = cmdData[2];
	byte reg_d = cmdData[3];
	unsigned startSector = reg_e + (reg_d * 256);
	if (reg_c < 0x80) {
		// FAT16 read/write sector
		startSector += reg_c << 16;
	}
	return startSector;
}
unsigned NowindHost::getStartAddress() const
{
	byte reg_l = cmdData[4];
	byte reg_h = cmdData[5];
	return reg_h * 256 + reg_l;
}
unsigned NowindHost::getCurrentAddress() const
{
	unsigned startAdress = getStartAddress();
	return startAdress + transfered;
}


void NowindHost::diskReadInit(SectorMedium& disk)
{
	unsigned sectorAmount = getSectorAmount();
	buffer.resize(sectorAmount * 512);
	unsigned startSector = getStartSector();
	if (disk.readSectors(&buffer[0], startSector, sectorAmount)) {
		// read error
		state = STATE_SYNC1;
		return;
	}

	transfered = 0;
	retryCount = 0;
	doDiskRead1();
}

void NowindHost::doDiskRead1()
{
	unsigned bytesLeft = unsigned(buffer.size()) - transfered;
	if (bytesLeft == 0) {
		sendHeader();
		send(0x01); // end of receive-loop
		send(0x00); // no more data
		state = STATE_SYNC1;
		return;
	}

	static const unsigned NUMBEROFBLOCKS = 32; // 32 * 64 bytes = 2048 bytes
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
		// TODO shouldn't we also test for (transferSize & 0x3F)?
		unsigned endAddress = address + transferSize;
		if (endAddress <= 0x8000) {
			transferSectorsBackwards(address, transferSize);
		} else {
			transferSize = 0x8000 - address;
			transferSectors(address, transferSize);
		}
	}

	// wait for 2 bytes
	state = STATE_DISKREAD;
	recvCount = 0;
}

void NowindHost::doDiskRead2()
{
	// diskrom sends back the last two bytes read
	assert(recvCount == 2);
	byte tail1 = extraData[0];
	byte tail2 = extraData[1];
	if ((tail1 == 0xAF) && (tail2 == 0x07)) {
		transfered += transferSize;
		retryCount = 0;

		unsigned address = getCurrentAddress();
		size_t bytesLeft = buffer.size() - transfered;
		if ((address == 0x8000) && (bytesLeft > 0)) {
			sendHeader();
			send(0x01); // end of receive-loop
			send(0xff); // more data for page 2/3
		}

		// continue the rest of the disk read
		doDiskRead1();
	} else {
		purge();
		if (++retryCount == 10) {
			// do nothing, timeout on MSX
			// too many retries, aborting readDisk()
			state = STATE_SYNC1;
			return;
		}

		// try again, wait for two bytes
		state = STATE_DISKREAD;
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

	const byte* bufferPointer = &buffer[transfered];
	for (unsigned i = 0; i < amount; ++i) {
		send(bufferPointer[i]);
	}
	send(0xAF);
	send(0x07); // used for validation
}

 // sends "02" + "transfer_addr" + "amount" + "data" + "0F 07"
void NowindHost::transferSectorsBackwards(unsigned transferAddress, unsigned amount)
{
	sendHeader();
	send(0x02); // don't exit command, (more) data is coming
	send16(transferAddress + amount);
	send(amount / 64);

	const byte* bufferPointer = &buffer[transfered];
	for (int i = amount - 1; i >= 0; --i) {
		send(bufferPointer[i]);
	}
	send(0xAF);
	send(0x07); // used for validation
}


void NowindHost::diskWriteInit(SectorMedium& disk)
{
	if (disk.isWriteProtected()) {
		sendHeader();
		send(1);
		send(0); // WRITEPROTECTED
		state = STATE_SYNC1;
		return;
	}

	unsigned sectorAmount = std::min(128u, getSectorAmount());
	buffer.resize(sectorAmount * 512);
	transfered = 0;
	doDiskWrite1();
}

void NowindHost::doDiskWrite1()
{
	unsigned bytesLeft = unsigned(buffer.size()) - transfered;
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
	assert(recvCount == (transferSize + 2));
	for (unsigned i = 0; i < transferSize; ++i) {
		buffer[i + transfered] = extraData[i + 1];
	}

	byte seq1 = extraData[0];
	byte seq2 = extraData[transferSize + 1];
	if ((seq1 == 0xaa) && (seq2 == 0xaa)) {
		// good block received
		transfered += transferSize;

		unsigned address = getCurrentAddress();
		size_t bytesLeft = buffer.size() - transfered;
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


unsigned NowindHost::getFCB() const
{
	// note: same code as getStartAddress(), merge???
	byte reg_l = cmdData[4];
	byte reg_h = cmdData[5];
	return reg_h * 256 + reg_l;
}

string NowindHost::extractName(int begin, int end) const
{
	string result;
	for (int i = begin; i < end; ++i) {
		char c = extraData[i];
		if (c == ' ') break;
		result += toupper(c);
	}
	return result;
}

int NowindHost::getDeviceNum() const
{
	unsigned fcb = getFCB();
	for (unsigned i = 0; i < MAX_DEVICES; ++i) {
		if (devices[i].fs.get() &&
		    devices[i].fcb == fcb) {
			return i;
		}
	}
	return -1;
}

int NowindHost::getFreeDeviceNum()
{
	int dev = getDeviceNum();
	if (dev != -1) {
		// There already was a device open with this fcb address,
		// reuse that device.
		return dev;
	}
	// Search for free device.
	for (unsigned i = 0; i < MAX_DEVICES; ++i) {
		if (!devices[i].fs.get()) {
			return i;
		}
	}
	// All devices are in use. This can't happen when the MSX software
	// functions correctly. We'll simply reuse the first device. It would
	// be nicer if we reuse the oldest device, but that's harder to
	// implement, and actually it doesn't really matter.
	return 0;
}

void NowindHost::deviceOpen()
{
	state = STATE_SYNC1;

	assert(recvCount == 11);
	string filename = extractName(0, 8);
	string ext      = extractName(8, 11);
	if (!ext.empty()) {
		filename += '.';
		filename += ext;
	}

	unsigned fcb = getFCB();
	unsigned dev = getFreeDeviceNum();
	devices[dev].fs.reset(new fstream()); // takes care of deleting old fs
	devices[dev].fcb = fcb;

	sendHeader();
	byte errorCode = 0;
	byte openMode = cmdData[2]; // reg_e
	switch (openMode) {
	case 1: // read-only mode
		devices[dev].fs->open(filename.c_str(), ios::in  | ios::binary);
		errorCode = 53; // file not found
		break;
	case 2: // create new file, write-only
		devices[dev].fs->open(filename.c_str(), ios::out | ios::binary);
		errorCode = 56; // bad file name
		break;
	case 8: // append to existing file, write-only
		devices[dev].fs->open(filename.c_str(), ios::out | ios::binary | ios::app);
		errorCode = 53; // file not found
		break;
	case 4:
		send(58); // sequential I/O only
		return;
	default:
		send(0xFF); // TODO figure out a good error number
		return;
	}
	assert(errorCode != 0);
	if (devices[dev].fs->fail()) {
		devices[dev].fs.reset();
		send(errorCode);
		return;
	}

	unsigned readLen = 0;
	bool eof = false;
	char buffer[256];
	if (openMode == 1) {
		// read-only mode, already buffer first 256 bytes
		readLen = readHelper1(dev, buffer);
		assert(readLen <= 256);
		eof = readLen < 256;
	}

	send(0x00); // no error
	send16(fcb);
	send16(9 + readLen + (eof ? 1 : 0)); // number of bytes to transfer

	send(openMode);
	send(0);
	send(0);
	send(0);
	send(cmdData[3]); // reg_d
	send(0);
	send(0);
	send(0);
	send(0);

	if (openMode == 1) {
		readHelper2(readLen, buffer);
	}
}

void NowindHost::deviceClose()
{
	int dev = getDeviceNum();
	if (dev == -1) return;
	devices[dev].fs.reset();
}

void NowindHost::deviceWrite()
{
	int dev = getDeviceNum();
	if (dev == -1) return;
	char data = cmdData[0]; // reg_c
	devices[dev].fs->write(&data, 1);
}

void NowindHost::deviceRead()
{
	int dev = getDeviceNum();
	if (dev == -1) return;

	char buffer[256];
	unsigned readLen = readHelper1(dev, buffer);
	bool eof = readLen < 256;
	send(0xAF);
	send(0x05);
	send(0x00); // dummy
	send16(getFCB() + 9);
	send16(readLen + (eof ? 1 : 0));
	readHelper2(readLen, buffer);
}

unsigned NowindHost::readHelper1(unsigned dev, char* buffer)
{
	assert(dev < MAX_DEVICES);
	unsigned len = 0;
	for (/**/; len < 256; ++len) {
		devices[dev].fs->read(&buffer[len], 1);
		if (devices[dev].fs->eof()) break;
	}
	return len;
}

void NowindHost::readHelper2(unsigned len, const char* buffer)
{
	for (unsigned i = 0; i < len; ++i) {
		send(buffer[i]);
	}
	if (len < 256) {
		send(0x1A); // end-of-file
	}
}


// strips a string from outer double-quotes and anything outside them
// ie: 'pre("foo")bar' will result in 'foo'
static string stripquotes(const string& str)
{
	string::size_type first = str.find_first_of('\"');
	if (first == string::npos) {
		// There are no quotes, return the whole string.
		return str;
	}
	string::size_type last  = str.find_last_of ('\"');
	if (first == last) {
		// Error, there's only a single double-quote char.
		return "";
	}
	// Return the part between the quotes.
	return str.substr(first + 1, last - first - 1);
}

void NowindHost::callImage(const string& filename)
{
	byte num = cmdData[7]; // reg_a
	if (num >= drives.size()) {
		// invalid drive number
		return;
	}
	if (drives[num]->insertDisk(stripquotes(filename))) {
		// TODO error handling
	}
}

void NowindHost::getDosVersion()
{
	sendHeader();
	send(enableMSXDOS2 ? 1:0);
}

} // namespace nowind
