#include "DiskHandler.hh"
#include "SectorMedium.hh"
#include "NowindHostSupport.hh"

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
	, nwhSupport(0)
{
    // test for requestWait
    vector<byte> requestWait;
    requestWait.push_back(1);
    requestWait.push_back(0);
    addStartupRequest(requestWait);
}

void NowindHost::initialize()
{
    if (nwhSupport == 0)
    {
        nwhSupport = new NowindHostSupport();
    }
    blockRead.initialize(nwhSupport);
    device.initialize(nwhSupport);
}

NowindHost::~NowindHost()
{
    if (nwhSupport)
    {
        delete nwhSupport;
        nwhSupport = 0;
    }
}

byte NowindHost::peek() const
{
	return nwhSupport->peek();
}

byte NowindHost::read()
{
	return nwhSupport->read();  // msx <- pc
}

bool NowindHost::isDataAvailable() const
{
	return nwhSupport->isDataAvailable();
}


// send:  msx -> pc
void NowindHost::write(byte data, unsigned int time)
{
	unsigned duration = 0; //time - lastTime;
	lastTime = time;
	if ((duration >= 500) && (state != STATE_SYNC1)) {
		// timeout (500ms), start looking for AF05
        DBERR("Protocol timeout occurred in state %d, purge buffers and switch back to STATE_SYNC1\n", state);
		nwhSupport->purge();
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
		    state = STATE_SYNC1;
			device.open(cmdData, extraData);
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
		// in STATE_BLOCKREAD we receive ack's from the send blocks and continue to send new blocks    
		blockRead.ack(data);     
		if (blockRead.isDone())
		{
		    state = STATE_SYNC1;
		}
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
	case 0x89: device.close(cmdData); state = STATE_SYNC1; break;
	//case 0x8A: deviceRandomIO(fcb);
	case 0x8B: device.write(cmdData); state = STATE_SYNC1; break;
	case 0x8C: device.read(cmdData); state = STATE_SYNC1; break;
	//case 0x8D: deviceEof(fcb);
	case 0x8E: auxIn();       state = STATE_SYNC1; break;
	case 0x8F: auxOut();      state = STATE_SYNC1; break;
	case 0x90: state = STATE_MESSAGE; recvCount = 0; break;
	case 0x91: state = STATE_IMAGE;   recvCount = 0; break;

    case 0x92: getDosVersion(); state = STATE_SYNC1; break;
	case 0x93: commandRequested(); state = STATE_SYNC1; break;
	//case 0xFF: vramDump();
	case 0x94: blockReadCmd(); break;
    case 0x95: blockWriteCmd(); break;
	default:
		// Unknown USB command!
		state = STATE_SYNC1;
		break;
	}
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

    unsigned int size = sectorAmount * 512;
    unsigned address = getCurrentAddress();
    blockRead.init(address, size, buffer);
    state = STATE_BLOCKREAD;
}

void NowindHost::diskWriteInit(SectorMedium& disk)
{
	DBERR("diskWriteInit\n");
	DBERR("startsector: %u  sectoramount %d\n", getStartSector(), getSectorAmount());
	if (disk.isWriteProtected()) {
		nwhSupport->sendHeader();
		nwhSupport->send(1);
		nwhSupport->send(0); // WRITEPROTECTED
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
	        DBERR("write to disk -> startsector: %u  sectoramount %d\n", startSector, sectorAmount);
			if (disk->writeSectors(&buffer[0], startSector, sectorAmount)) {
				// TODO write error
			}
		}
		nwhSupport->sendHeader();
		nwhSupport->send(255);
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

    DBERR(" address: 0x%04x, transferSize: 0x%04X \n", address, transferSize);
    
	nwhSupport->sendHeader();
	nwhSupport->send(0);          // data ahead!
	nwhSupport->send16(address);
	nwhSupport->send16(transferSize);
	nwhSupport->send(0xaa);

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
			nwhSupport->sendHeader();
			nwhSupport->send(254); // more data for page 2/3
	        DBERR(" more data for page 2/3\n");
		}
	} else {
	    DBERR(" ERROR!!! This situation is still not handled correctly!\n");
		nwhSupport->purge();
	}

	// continue the rest of the disk write
	doDiskWrite1();
}

// dummy command (reads first 16Kb of disk as test)
void NowindHost::blockReadCmd()
{
    SectorMedium* disk = drives[0]->getSectorMedium();
    
    vector<byte> data(16*1024);
	if (disk->readSectors(&data[0], 0, 32)) {
		DBERR("readSectors error reading sector 0-31\n");
	}
	
    blockRead.init(0x8000, 0x4000, data);
    state = STATE_BLOCKREAD;	
}

void NowindHost::blockWriteCmd()
{
}

void NowindHost::debugMessage(const char *, ...) const
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

void NowindHost::msxReset()
{
    device.reset();
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


void NowindHost::auxIn()
{
	char input;
	DBERR("auxIn\n");
	nwhSupport->sendHeader();

	dumpRegisters();
	std::cin >> input;

	nwhSupport->sendHeader();
	nwhSupport->send(input);
    DBERR("auxIn returning 0x%02x\n", input);
}

void NowindHost::auxOut()
{
	DBERR("auxOut: %c\n", cmdData[7]);
	dumpRegisters();
	printf("%c", cmdData[7]);
}

void NowindHost::dumpRegisters()
{
	//reg_[cbedlhfa] + cmd
	DBERR("AF: 0x%04X, BC: 0x%04X, DE: 0x%04X, HL: 0x%04X, CMD: 0x%02X\n", cmdData[7] * 256 + cmdData[6], cmdData[1] * 256 + cmdData[0], cmdData[3] * 256 + cmdData[2], cmdData[5] * 256 + cmdData[4], cmdData[8]);
}

void NowindHost::DSKCHG()
{
	SectorMedium* disk = getDisk();
	if (!disk) {
		// no such drive or no disk inserted
		return;
	}

	nwhSupport->sendHeader();
	byte num = cmdData[7]; // reg_a
	assert(num < drives.size());
	if (drives[num]->diskChanged()) {
		nwhSupport->send(255); // changed
		// read first FAT sector (contains media descriptor)
		byte sectorBuffer[512];
		if (disk->readSectors(sectorBuffer, 1, 1)) {
			// TODO read error
			sectorBuffer[0] = 0;
		}
		nwhSupport->send(sectorBuffer[0]); // new mediadescriptor
	} else {
		nwhSupport->send(0);   // not changed
		nwhSupport->send(255); // dummy
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
	
	// maxClus is the number of clusters on disk not including reserved sector, 
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

	nwhSupport->sendHeader();
	// send dest. address
	nwhSupport->send(cmdData[2]);	// reg_e
	nwhSupport->send(cmdData[3]);	// reg_d
	byte * refData = (byte *) &dpb_pre;
	byte * sendBuffer = (byte *) &dpb;

	for (int i=0;i<18;i++) {
		DBERR("GETDPB offset [%d]: 0x%02X, correct: 0x%02X\n", i+1, sendBuffer[i], refData[i]);
		nwhSupport->send(sendBuffer[i]);
	}
}


void NowindHost::DRIVES()
{
	// at least one drive (MSXDOS1 cannot handle 0 drives)
	byte numberOfDrives = std::max<byte>(1, byte(drives.size()));

	byte reg_a = cmdData[7];
	nwhSupport->sendHeader();
	nwhSupport->send(getEnablePhantomDrives() ? 0x02 : 0);
	nwhSupport->send(reg_a | (getAllowOtherDiskroms() ? 0 : 0x80));
	nwhSupport->send(numberOfDrives);

//	romdisk = 255; // no romdisk
	for (unsigned i = 0; i < drives.size(); ++i) {
		if (drives[i]->isRomdisk()) {
			romdisk = i;
			break;
		}
	}
}

void NowindHost::INIENV()
{
	nwhSupport->sendHeader();
	nwhSupport->send(romdisk); // calculated in DRIVES()
}

void NowindHost::setDateMSX()
{
	time_t td = time(NULL);
	struct tm* tm = localtime(&td);

	nwhSupport->sendHeader();
	nwhSupport->send(tm->tm_mday);          // day
	nwhSupport->send(tm->tm_mon + 1);       // month
	nwhSupport->send16(tm->tm_year + 1900); // year
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
	unsigned startAddress = getStartAddress();
	return startAddress + transferred;
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
	nwhSupport->sendHeader();
	nwhSupport->send(enableMSXDOS2 ? 1:0);
}



// the MSX asks whether the host has a command  
// waiting for it to execute
void NowindHost::commandRequested()
{
    char cmdType = cmdData[1]; // reg_b
    char cmdArg = cmdData[0]; // reg_c

    switch (cmdType)
    {
    case 0x00:
        // command request at startup, read from startupRequestQueue
        commandRequestedAtStartup(cmdArg);
        break;
    case 0x01:
       commandRequestedAnytime();
       break;
    default:
        DBERR("MSX sent unknown commandRequested type %d\n", cmdType);
        break;
    }
}

// the startupRequestQueue is not cleared by the msx requesting commands
// each time the msx boots, the same startup commands are send as long 
// as the user application does not remove them
void NowindHost::commandRequestedAtStartup(byte reset)
{
    static unsigned int index = 0;
    if (reset == 0x00)
    {
        // The MSX is in its diskrom startup sequence at INIHDR and requests the first startup command
        DBERR("INIHRD hook requests command at startup\n");
        // this reset the index for startupRequestQueue
        index = 0;
    }
    else
    {
        // The MSX is in its diskrom startup sequence at INIHDR and requests the next startup command
        DBERR("INIHRD hook requests next command at startup\n");
    }

    nwhSupport->sendHeader();

    std::vector<byte> command;
    if (index >= startupRequestQueue.size())
    {
        nwhSupport->send(0);   // no more commands 
    }
    else
    {
        command = startupRequestQueue.at(index);
        index++;

        for (unsigned int i=0;i<command.size();i++)
        {
            nwhSupport->send(command[i]);
        }
    }
}

// command from the requestQueue are sent only once, 
// and are them removed from the queue
void NowindHost::commandRequestedAnytime()
{
    nwhSupport->sendHeader();
    if (requestQueue.empty())
    {
        nwhSupport->send(0);
    }
    else
    {
        std::vector<byte> command = requestQueue.front();
        // remove command from queue        
        requestQueue.pop_front();
		if (requestQueue.empty())
		{
			nwhSupport->send(0);
		}
		else
		{
			nwhSupport->send(1);
		}

        for (unsigned int i=0;i<command.size();i++)
	    {
            nwhSupport->send(command[i]);
	    }
    }
}

void NowindHost::clearStartupRequests()
{
    startupRequestQueue.clear();
}

void NowindHost::addStartupRequest(std::vector<byte> command)
{
    startupRequestQueue.push_back(command);
}


void NowindHost::clearRequests()
{
    requestQueue.clear();
}

void NowindHost::addRequest(std::vector<byte> command)
{
    requestQueue.push_back(command);
}


} // namespace nowind
