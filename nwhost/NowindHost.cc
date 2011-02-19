#include "DiskHandler.hh"
#include "SectorMedium.hh"
#include "Image.h"
#include "NowindHostSupport.hh"

#include <fstream>
#include <algorithm>
#include <cassert>
#include <ctype.h>
#include <time.h>
#include <iostream>
#include <vector>

#define ToUpper(s) std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) toupper)

#define NWHOST_API_EXPORT
#include "NowindHost.hh"

// splits a string into parts separated by delimeters (returns a vector of substrings)
std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty = true) {
    std::vector<std::string> result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    std::string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = search(substart, s.end(), delim.begin(), delim.end());
        std::string temp(substart, subend);
        if (keep_empty || !temp.empty()) {
            result.push_back(temp);
        }
        if (subend == s.end()) {
            break;
        }
        substart = subend + delim.size();
    }
    return result;
}


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

enum {
    BLOCKWRITE_EXIT_MORE_DATE_AHEAD,
    BLOCKWRITE_FASTTRANSFER,
    BLOCKWRITE_EXIT,
};

NowindHost::NowindHost(const vector<DiskHandler*>& drives_)
	: drives(drives_)
	, lastTime(0)
	, state(STATE_SYNC1)
	, romdisk(255)
	, allowOtherDiskroms(false)
	, enablePhantomDrives(false)
	, enableMSXDOS2(false)
	, nwhSupport(0)
	, driveOffset(0)
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
	unsigned duration = time - lastTime;
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
		timer1 = time;
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
            //DBERR("Blockread duration: %u\n", time-timer1);
		    state = STATE_SYNC1;
		}
		break;
	case STATE_CPUINFO:
	{
	    unsigned int databytes = 11;
	    unsigned int stackbytes = 34;
		assert(recvCount < (databytes+(stackbytes)));
		extraData[recvCount] = data;
		if (++recvCount == (databytes+(stackbytes))) {
		    state = STATE_SYNC1;
			reportCpuInfo();
		}
		break;	
    }	
    case STATE_RECEIVE_DATA:
        apiReceiveData(data);
        break;

    case STATE_BDOS_OPEN_FILE:
		extraData[recvCount] = data;
		if (++recvCount == 36) {
		    state = STATE_SYNC1;
		    BDOS_OpenFile();
		}
        break;

	default:
		assert(false);
	}
}

void NowindHost::reportCpuInfo()
{
//                                   01234567    8
//	byte cmdData[9];         // reg_[cbedlhfa] + cmd
//	byte extraData[240 + 2]; // extra data for image/message/write

    word reg_bc = cmdData[0] + 256*cmdData[1];
    word reg_de = cmdData[2] + 256*cmdData[3];
    word reg_hl = cmdData[4] + 256*cmdData[5];
    word reg_af = cmdData[6] + 256*cmdData[7];

    word reg_ix = extraData[0] + 256*extraData[1];
    word reg_iy = extraData[2] + 256*extraData[3];
    word reg_sp = extraData[4] + 256*extraData[5];
    reg_sp += 6;

    byte mainSS = extraData[6];
    word fcc5 = extraData[7] + 256*extraData[8];
    word fcc7 = extraData[9] + 256*extraData[10];
    word reg_pc = extraData[11] + 256*extraData[12];
    
    DBERR("PC:%04X AF:%04X BC:%04X DE:%04X HL:%04X IX:%04X IY:%04X S:%04X\n", \
        reg_pc, reg_af, reg_bc, reg_de, reg_hl, reg_ix, reg_iy, reg_sp);
    
    /*
    // stack dump    
    for (int i=0; i<16; i++)
    {
        DBERR("  0x%04X: 0x%04x\n", reg_sp, extraData[13+(i*2)] + 256*extraData[14+(i*2)]);
        reg_sp += 2;
    }
    */
}

void NowindHost::executeCommand()
{
    //DBERR(nowMap("0 1").c_str());
    //DBERR("\n");

	assert(recvCount == 9);
	byte cmd = cmdData[8];
	switch (cmd) {

	case 0x0F: BDOS_0FH_OpenFile(); break;
	case 0x10: BDOS_10H_CloseFile(); break;
	case 0x27: BDOS_27H_ReadRandomBlock(); break;
    /*
	case 0x0D: BDOS_0DH_DiskReset(); break;
	case 0x11: BDOS_11H_FindFirst(); break;
	case 0x12: BDOS_12H_FindNext(); break;
	case 0x13: BDOS_13H_DeleteFile(); break;
	case 0x14: BDOS_14H_ReadSeq(); break;
	case 0x15: BDOS_15H_WriteSeq(); break;
	case 0x16: BDOS_16H_CreateFile(); break;
	case 0x17: BDOS_17H_RenameFile(); break;
	case 0x21: BDOS_21H_ReadRandomFile(); break;
	case 0x22: BDOS_22H_WriteRandomFile(); break;
	case 0x23: BDOS_23H_GetFileSize(); break;
	case 0x24: BDOS_24H_SetRandomRecordField(); break;
	case 0x26: BDOS_26H_WriteRandomBlock(); break;
	case 0x28: BDOS_28H_WriteRandomFileWithZeros(); break;
	case 0x2A: BDOS_2AH_GetDate(); break;
	case 0x2B: BDOS_2BH_SetDate(); break;
	case 0x2C: BDOS_2CH_GetTime(); break;
	case 0x2D: BDOS_2DH_SetTime(); break;
	case 0x2E: BDOS_2EH_Verify(); break;
	case 0x2F: BDOS_2FH_ReadLogicalSector(); break;
	case 0x30: BDOS_30H_WriteLogicalSector(); break;
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
	case 0x90: receiveExtraData(); state = STATE_MESSAGE; break;
	case 0x91: receiveExtraData(); state = STATE_IMAGE; break;
    case 0x92: getDosVersion(); state = STATE_SYNC1; break;
	case 0x93: commandRequested(); state = STATE_SYNC1; break;
	//case 0xFF: vramDump();
	case 0x94: blockReadCmd(); break;
    case 0x95: blockWriteCmd(); break;
    case 0x96: receiveExtraData(); state = STATE_CPUINFO; break;
    case 0x97: apiCommand();  break;
	default:
		// Unknown USB command!
		state = STATE_SYNC1;
		break;
	}
}

void NowindHost::apiCommand()
{
    DBERR("apiCommand\n");
    extraData[0] = 0;
    dumpRegisters();

    byte reg_a = cmdData[7];
    byte reg_c = cmdData[0];
    byte reg_b = cmdData[1];
    switch (reg_c)
    {
        case API_NOWMAP:
            if (reg_a != 4)
            {   
                DBERR("API_NOWMAP received with wrong EXTBIO function (%u)\n", reg_a);
                state = STATE_SYNC1;
            }
            else
            {
                if (reg_b == 0)
                {
                    DBERR("API_NOWMAP received without commandline!\n");
                    state = STATE_SYNC1;
                }
                else
                {
                    recvCount = 0;
                    state = STATE_RECEIVE_DATA;
                }
            }
            break;
        default:
            DBERR("apiCommand_API_??\n");
            state = STATE_SYNC1;
    }
}

void NowindHost::apiReceiveData(byte data)
{
    byte reg_b = cmdData[1];
    
    extraData[recvCount] = data;
    ++recvCount;

    DBERR("apiReceiveData %u/%u: %c\n", recvCount, reg_b, data);

    if (recvCount >= reg_b)
    {
        // all data received, execute command
        byte reg_c = cmdData[0];
        word reg_de = cmdData[2] + 256*cmdData[3];
        DBERR("\n");

        switch (reg_c)
        {
            case API_NOWMAP:
            {
                string args = string(reinterpret_cast<char*>(extraData));
                string result = nowMap(args);
                vector<byte> resultData;
                resultData.assign(result.begin(), result.end());
                resultData.push_back(0);
                
                blockRead.init(reg_de, resultData.size(), resultData);
                state = STATE_BLOCKREAD;
                break;
            }
            default:
                DBERR("apiReceiveData_API_??\n");
                state = STATE_SYNC1;
        }
    }
}

void NowindHost::apiReceiveString(byte data)
{

}

// nowmap <drive_id> <partition> [/Nx] [/L]
// ex. nowmap 0 1
// ex. nowmap 0 1 /N1
std::string NowindHost::nowMap(std::string arguments)
{
    ToUpper(arguments);
    DBERR("arguments: %s\n", arguments.c_str());

    char temp[250];
    char letter = 'A';
    std::string response = "Nowind Map v1.2\r\n";
    for (unsigned i = 0; i < driveOffset; ++i) {
        sprintf(temp, "Drive %c: DiskRom\r\n", letter); // todo: get SLOT ?-? from drvtbl
        letter++;
        response += std::string(temp);        
    }
  
    for (unsigned i = 0; i < drives.size(); ++i) {
        Image* image = dynamic_cast<Image*>(drives[i]->getSectorMedium());
        if (image->isHarddiskPartition())
        {
            sprintf(temp, "Drive %c, driveId %d (%s), partition %d\r\n", letter, i, image->getDescription().c_str(), image->getPartitionNr());
        }
        else
        {
            sprintf(temp, "Drive %c, driveId %d (%s)\r\n", letter, i, image->getDescription().c_str());
        }
        letter++;
        response += std::string(temp);
    }
    response += "\r\n";
    
    if (arguments.find("/L") != string::npos)
    {
        // list drives (response already contains correct text)
        return response;
    }
    
    // interpret arguments
    vector<string> args = split(arguments, " ");
    if (args.size() != 2)
    {
        return "error: no 2 arguments found\nusage: nowmap <drive_id> <partition> [/Nx] [/L]\n";
    }
 
    unsigned int driveId = atoi(args[0].c_str());
    unsigned int partition = atoi(args[1].c_str());
    
    if (driveId >= drives.size())
    {
        return "error: drive_id invalid!\nusage: nowmap <drive_id> <partition> [/Nx] [/L]\n";
    }
    
    Image* image = dynamic_cast<Image*>(drives[driveId]->getSectorMedium());
    
    if (!image->isHarddiskPartition())
    {
        sprintf(temp, "error: drive is not a hdd!\nusage: nowmap <drive_id> <partition> [/Nx] [/L]\n", driveId);
        return string(temp);
    }
    
    image->SetActivePartition(partition);
    sprintf(temp, "hdd of drive id %d was set to partition %u\n", driveId, partition);
    
    DBERR("response: %s\n", temp);            
    return string(temp);
}

void NowindHost::receiveExtraData()
{
    recvCount = 0;
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
    unsigned address = getStartAddress();
    
	DBERR("NowindHost::diskRead, startSector: %u  sectorAmount: %u, address: 0x%04x\n", startSector, sectorAmount, address);
    
    blockRead.init(address, size, buffer);
    state = STATE_BLOCKREAD;
}

void NowindHost::diskWriteInit(SectorMedium& disk)
{
	DBERR("NowindHost::diskWrite, startSector: %u  sectorAmount: %u\n", getStartSector(), getSectorAmount());
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
	unsigned bytesLeft = unsigned(buffer.size()) - transferred;
	if (bytesLeft == 0) {
		// All data transferred!
		unsigned sectorAmount = unsigned(buffer.size()) / 512;
		unsigned startSector = getStartSector();
		if (SectorMedium* disk = getDisk()) {
			int result = disk->writeSectors(&buffer[0], startSector, sectorAmount);
			if (0 != result) {
			    DBERR("Error %i writing disk image!\n", result);
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
    DBERR("blockReadCmd\n");
/*
    SectorMedium* disk = drives[0]->getSectorMedium();
    
    vector<byte> data(16*1024);
	if (disk->readSectors(&data[0], 0, 32)) {
		DBERR("readSectors error reading sector 0-31\n");
	}
	
    blockRead.init(0x8000, 0x4000, data);
    state = STATE_BLOCKREAD;	
*/
}

void NowindHost::blockWriteCmd()
{
    DBERR("blockWriteCmd\n");
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

	DBERR("GETDPB driveNumber: %u\n", num);
	SectorMedium* disk = getDisk();
	if (!disk) {
		// no such drive or no disk inserted
		DBERR("GETDPB error no disk\n");
		return;
	}

	byte sectorBuffer[512];
	if (disk->readSectors(sectorBuffer, 0, 1)) {
		DBERR("GETDPB error reading sector 0\n");
		return;
	}

	word bytesPerSector = sectorBuffer[11]+256*sectorBuffer[12];
	byte sectorsPerCluster = sectorBuffer[13];
	word firstFatSector = sectorBuffer[14]+256*sectorBuffer[15];
	byte fatCopies = sectorBuffer[16];
	word rootDirEntries = sectorBuffer[17]+256*sectorBuffer[18];
	word numberOfSectors = sectorBuffer[19]+256*sectorBuffer[20];
	byte mediaType = sectorBuffer[21];
	word sectorsPerFat = sectorBuffer[22]+256*sectorBuffer[23];

    if (numberOfSectors == 0) {
	    // use sectorsBig instead
        numberOfSectors = sectorBuffer[32] + 256*sectorBuffer[33] + 256*256*sectorBuffer[34] + 256*256*256*sectorBuffer[35];
        DBERR("Using bigSectors (%u) instead of numberOfSectors!\n", numberOfSectors);
    }

	word firstDirSector = firstFatSector + (fatCopies * sectorsPerFat);
	word entriesPerSector = bytesPerSector / 16;
	word directorySizeInSectors = (rootDirEntries * entriesPerSector)/bytesPerSector;   // TODO: should be rounded up!?
	word firstRecordSector = firstDirSector + directorySizeInSectors;
    word maxClusters = ((numberOfSectors - firstRecordSector)/sectorsPerCluster) + 1;

	// the actual dpb[0] (drive number) is not send
	dpbType dpb;
	
	dpb.ID = mediaType;     	   	         // offset 1
	dpb.SECSIZ_L = bytesPerSector & 0xff;    // offset 2
	dpb.SECSIZ_H = bytesPerSector >> 8;	     // offset 3
	dpb.DIRMSK = (bytesPerSector/32)-1;	     // offset 4 (TODO: check berekening!)

	byte dirShift;
	for (dirShift=0;dpb.DIRMSK & (1<<dirShift);dirShift++) {}

	dpb.DIRSHFT = dirShift;		             // offset 5 (nr of 1-bits in DIRMSK)
	dpb.CLUSMSK = sectorsPerCluster - 1;     // offset 6

	byte cluShift;
	for (cluShift=0;dpb.CLUSMSK & (1<<cluShift);cluShift++) {}

	dpb.CLUSSHFT = cluShift+1;            	 // offset 7 (nr of bits in clusterMask+1)

	dpb.FIRFAT_L = firstFatSector & 0xFF;    // offset 8
	dpb.FIRFAT_H = firstFatSector >> 8;      // offset 9
	dpb.FATCNT = fatCopies;          	     // offset 10 (number of FATs)
	dpb.MAXENT = rootDirEntries;       	     // offset 11

	// the data of the disk starts at the firstDIRsector + size of the directory area
	// (the "directory" area contains max. 254 entries of 16 bytes, one entry of each file)

	dpb.FIRREC_L = firstRecordSector & 0xFF; // offset 12 (number of first data sector, low)
	dpb.FIRREC_H = firstRecordSector >> 8;   // offset 13 (number of first data sector, high)
	
	// maxClus is the number of clusters on disk not including reserved sector, 
	// fat sectors or directory sectors, see p260 of Msx Redbook

	dpb.MAXCLUS_L = maxClusters & 0xFF;      // offset 14 (highest cluster number, low)
	dpb.MAXCLUS_H = maxClusters >> 8;        // offset 15 (highest cluster number, high)

	dpb.FATSIZ = sectorsPerFat;              // offset 16 (number of sectors/FAT)

	dpb.FIRDIR_L = firstDirSector & 0xFF;    // offset 17
	dpb.FIRDIR_H = firstDirSector >> 8;      // offset 18

	// We dont know what sectorBuffer 0x1C-1F contains on MSX harddisk images 

	nwhSupport->sendHeader();

	// send destination address
	nwhSupport->send(cmdData[2]);	// reg_e
	nwhSupport->send(cmdData[3]);	// reg_d

	byte * sendBuffer = (byte *) &dpb;
	for (int i=0;i<18;i++) {
		DBERR("GETDPB offset [%d]: 0x%02X\n", i+1, sendBuffer[i]);
		nwhSupport->send(sendBuffer[i]);
	}
}

// msx sends the amount of drives already installed in reg_a
void NowindHost::DRIVES()
{
	byte reg_a = cmdData[7];
	// at least one drive (MSXDOS1 cannot handle 0 drives)
	byte numberOfDrives = std::max<byte>(1, byte(drives.size()));

    driveOffset = reg_a;
	nwhSupport->sendHeader();
	nwhSupport->send(getEnablePhantomDrives() ? 0x02 : 0);
	nwhSupport->send(reg_a | (getAllowOtherDiskroms() ? 0 : 0x80));
	nwhSupport->send(numberOfDrives);

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
	DBERR("INIENV (romdrv nr: %i) \n", romdisk);
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
        DBERR("MSX requests command at startup\n");
        // this reset the index for startupRequestQueue
        index = 0;
    }
    else
    {
        // The MSX is in its diskrom startup sequence at INIHDR and requests the next startup command
        DBERR("MSX requests next command at startup\n");
    }

    nwhSupport->sendHeader();

    std::vector<byte> command;
    if (index >= startupRequestQueue.size())
    {
        nwhSupport->send(0);   // no more commands 
        DBERR("No more startup commands.\n");
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

void NowindHost::BDOS_0FH_OpenFile()
{
    DBERR(" >> BDOS_0FH_OpenFile\n");

	state = STATE_BDOS_OPEN_FILE;
	recvCount = 0;   
}

void NowindHost::BDOS_OpenFile()
{
 	string result;
	for (int i = 1; i < 13; ++i) {
		char c = extraData[i];
		result += toupper(c);
	}

    DBERR(" nu hebben we een fcb: %s\n", result.c_str());
    //bdosFiles.push_back( new fstream(imageName.c_str(), ios::binary | ios::in);
    //bdosfile = new fstream(result.c_str(), ios::binary | ios::in);
    
    state = STATE_SYNC1;
}

void NowindHost::BDOS_10H_CloseFile()
{
    DBERR(" >> BDOS_10H_CloseFile\n");
    reportCpuInfo();
    
    state = STATE_SYNC1;
}

void NowindHost::BDOS_27H_ReadRandomBlock()
{
    DBERR(" >> BDOS_27H_ReadRandomBlock\n");
    reportCpuInfo();
    //std::vector<char> data;
    
    int count = 128;
    int offset = 0;
    //data.resize(count);
    //bdosfile->seekg(offset);
    //bdosfile->read(&data[0], count);
    
    state = STATE_SYNC1;
}

} // namespace nowind
