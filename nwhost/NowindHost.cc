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
LIBRARY WISH LIST:
	enum to string functions for printing enum-values
*/


/*
For debugging:

commands send from the msx to the host look like:
AF 05 cc bb ee dd ll hh ff aa CC
first the 2 header bytes AF 05 are sent, then all registers are send (8 bytes) and finally the command is sent (1 byte)
Some commands need extra data, up to 240 bytes are send directly following the command.
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
	, allowOtherDiskroms(false)
	, enablePhantomDrives(false)
	, enableMSXDOS2(false)
	, nwhSupport(0)
	, driveOffset(0)
	, activeCommand(0)
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
	
	response = nwhSupport->getresponse();

    blockRead.initialize(nwhSupport);
    bdosProxy.initialize(nwhSupport);
    command.initialize(nwhSupport);
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
	return response->peek();
}

byte NowindHost::read()
{
	return response->read();  // msx <- pc
}

bool NowindHost::isDataAvailable() const
{
	return response->isDataAvailable();
}

// send:  msx -> pc
void NowindHost::write(byte data, unsigned int time)
{
	command.data = data;
	command.time = time;
	unsigned duration = time - lastTime;
	lastTime = time;
	if ((duration >= 500) && (state != STATE_SYNC1)) {
		// timeout (500ms), start looking for AF05
        DBERR("Protocol timeout occurred in state %d, purge buffers and switch back to STATE_SYNC1\n", state);
		response->purge();
		setState(STATE_SYNC1);
	}
	DBERR("received: 0x%02x (in state: %d, activeCommand: %d (0x%02x)\n", data, state, activeCommand, activeCommand);
	switch (state) {
	case STATE_SYNC1:
		timer1 = time;
		if (data == 0xAF) setState(STATE_SYNC2);
		break;
	case STATE_SYNC2:
		switch (data) {
		case 0x05: setState(STATE_RECEIVE_COMMAND); break;
		case 0xAF: setState(STATE_SYNC2); break;
		case 0xFF: setState(STATE_SYNC1); msxReset(); break;
		default: 
			setState(STATE_SYNC1); 
			break;
		}
		break;
	case STATE_RECEIVE_COMMAND:
		assert(recvCount < 9);
		command.cmdData[recvCount] = data;
		if (++recvCount == 9) {
			prepareCommand();
		}
		break;
	case STATE_RECEIVE_PARAMETERS:
		assert(recvCount < parameterLength);
		command.extraData[recvCount] = data;
		if (++recvCount == parameterLength) {
			setState(STATE_EXECUTE_COMMAND);
			executeCommand();
		}
		break;
	case STATE_EXECUTE_COMMAND:
			executeCommand();
		break;
	case STATE_DISKWRITE:
		assert(recvCount < (transferSize + 2));
		command.extraData[recvCount] = data;
		if (++recvCount == (transferSize + 2)) {
			doDiskWrite2();
		}
		break;
	case STATE_DEVOPEN:
		assert(recvCount < 11);
		command.extraData[recvCount] = data;
		if (++recvCount == 11) {
		    setState(STATE_SYNC1);
			device.open(command.cmdData, &command.extraData[0]);
		}
		break;
	case STATE_IMAGE:
		assert(recvCount < 40);
		command.extraData[recvCount] = data;
		if ((data == 0) || (data == ':') ||
		    (++recvCount == 40)) {
			char* data = reinterpret_cast<char*>(&command.extraData[0]);
			callImage(string(data, recvCount));
			setState(STATE_SYNC1);
		}
		break;
	case STATE_MESSAGE:
		assert(recvCount < (240 - 1));
		command.extraData[recvCount] = data;
		if ((data == 0) || (++recvCount == (240 - 1))) {
			dumpRegisters();
			command.extraData[recvCount] = 0;
			DBERR("DBG MSG: %s\n", &command.extraData[0]);
			setState(STATE_SYNC1);
		}
		break;
	case STATE_BLOCKREAD:
		// in STATE_BLOCKREAD we receive ack's from the send blocks and continue to send new blocks    
		blockRead.ack(data);     
		if (blockRead.isDone())
		{
            //DBERR("Blockread duration: %u\n", time-timer1);
		    setState(STATE_SYNC1);
		}
		break;
	case STATE_CPUINFO:
	{
	    unsigned int databytes = 11;
	    unsigned int stackbytes = 34;
		assert(recvCount < (databytes+(stackbytes)));
		command.extraData[recvCount] = data;
		if (++recvCount == (databytes+(stackbytes))) {
		    setState(STATE_SYNC1);
			command.reportCpuInfo();
		}
		break;	
    }	
    case STATE_RECEIVE_DATA:
        apiReceiveData(data);
        break;
/*
    case STATE_BDOS_FIND_FIRST:
		command.extraData[recvCount] = data;
		if (++recvCount == 36) {
		    state = STATE_SYNC1;
		    if (bdosProxy.FindFirst(command, *response)) state = STATE_BLOCKREAD;
		}
        break;
*/
	default:
		assert(false);
	}
}

void NowindHost::setState(State aState)
{
	state = aState;
	switch (state)
	{
	case STATE_RECEIVE_COMMAND:
	case STATE_RECEIVE_PARAMETERS:
	case STATE_RECEIVE_DATA:
	case STATE_DEVOPEN:
		recvCount = 0;
		break;
	case STATE_SYNC1:
		recvCount = 0;
		activeCommand = 0;
		break;
	default:
		// no nothing
		break;
	}

	switch (state)
	{
	case STATE_SYNC1:
		DBERR(" # STATE_SYNC1\n"); break;
	case STATE_SYNC2:
		DBERR(" # STATE_SYNC2\n"); break;
	case STATE_RECEIVE_COMMAND:
		DBERR(" # STATE_RECEIVE_COMMAND\n"); break;
	case STATE_RECEIVE_PARAMETERS:
		DBERR(" # STATE_RECEIVE_PARAMETERS\n"); break;
	case STATE_EXECUTE_COMMAND:
		DBERR(" # STATE_EXECUTE_COMMAND\n"); break;
	case STATE_DISKWRITE:
		DBERR(" # STATE_DISKWRITE\n"); break;
	case STATE_DEVOPEN:
		DBERR(" # STATE_DEVOPEN\n"); break;
	case STATE_IMAGE:
		DBERR(" # STATE_IMAGE\n"); break;
	case STATE_MESSAGE:
		DBERR(" # STATE_MESSAGE\n"); break;
	case STATE_BLOCKREAD:
		DBERR(" # STATE_BLOCKREAD\n"); break;
	case STATE_CPUINFO:
		DBERR(" # STATE_CPUINFO\n"); break;
	case STATE_RECEIVE_DATA:
		DBERR(" # STATE_RECEIVE_DATA\n"); break;
	case STATE_RECEIVE_STRING:
		DBERR(" # STATE_RECEIVE_STRING\n"); break;
	case STATE_BDOS_OPEN_FILE:
		DBERR(" # STATE_BDOS_OPEN_FILE\n"); break;
	case STATE_BDOS_FIND_FIRST:
		DBERR(" # STATE_BDOS_FIND_FIRST\n"); break;
	default:
		DBERR(" # STATE_ unknown !\n"); break;
		break;
	}
}

void NowindHost::prepareCommand()
{
	assert(activeCommand == 0);			// prepare should only be called when no command is active yet
	assert(state == STATE_RECEIVE_COMMAND);

	activeCommand = command.getCommand();
	DBERR("prepareCommand: %d (0x%02x)\n", activeCommand, activeCommand);
	switch (activeCommand) {
		case 0x0F: // bdosProxy.OpenFile
		case 0x11: // bdosProxy.FindFirst
			parameterLength = 36;
			setState(STATE_RECEIVE_PARAMETERS);
			break;
		default:
			setState(STATE_EXECUTE_COMMAND);
			executeCommand();
			break;
	}
}

void NowindHost::executeCommand()
{
	assert(activeCommand != 0);
	assert(state == STATE_EXECUTE_COMMAND);

	State nextState = STATE_SYNC1; // unless we set the state explictly, we return to STATE_SYNC1 after the command is executed.

	switch (activeCommand) {
	case 0: assert(false); break; // these is no command '0', nor should there be.
	case 0x0D: bdosProxy.DiskReset(command, *response); break;
	case 0x0F: if (bdosProxy.OpenFile(command, *response)) { nextState = STATE_EXECUTE_COMMAND; } break;
	case 0x10: bdosProxy.CloseFile(command, *response); break;
	case 0x11: if (bdosProxy.FindFirst(command, *response)) { nextState = STATE_EXECUTE_COMMAND; } break;
	case 0x12: if (bdosProxy.FindNext(command, *response)) { nextState = STATE_EXECUTE_COMMAND; } break;
	case 0x13: bdosProxy.DeleteFile(command, *response); break;
	case 0x14: bdosProxy.ReadSeq(command, *response); break;
	case 0x15: bdosProxy.WriteSeq(command, *response); break;
	case 0x16: bdosProxy.CreateFile(command, *response); break;
	case 0x17: bdosProxy.RenameFile(command, *response); break;
	case 0x21: bdosProxy.ReadRandomFile(command, *response); break;
	case 0x22: bdosProxy.WriteRandomFile(command, *response); break;
	case 0x23: bdosProxy.GetFileSize(command, *response); break;
	case 0x24: bdosProxy.SetRandomRecordField(command, *response); break;
	case 0x26: bdosProxy.WriteRandomBlock(command, *response); break;
	case 0x27: if (bdosProxy.ReadRandomBlock(command, *response)) { nextState = STATE_EXECUTE_COMMAND; } break;
	case 0x28: bdosProxy.WriteRandomFileWithZeros(command, *response); break;

	//case 0x2A: GetDate(); break; // no implementation needed
	//case 0x2B: SetDate(); break; // no implementation needed
	//case 0x2C: GetTime(); break; // no implementation needed
	//case 0x2D: SetTime(); break; // no implementation needed
	//case 0x2E: Verify(); break;  // no implementation needed

	case 0x2F: bdosProxy.ReadLogicalSector(command, *response); break;
	case 0x30: bdosProxy.WriteLogicalSector(command, *response); break;

	// http://map.grauw.nl/resources/dos2_functioncalls.php#_SETDTA
	// http://map.grauw.nl/resources/dos2_environment.php

	case 0x80: { // DSKIO
		SectorMedium* disk = getDisk();
		if (!disk) {
			// no such drive or no disk inserted
			// (no response, will cause a timeout on the MSX side)
			return;
		}
		if (command.getF() & Command::F_CARRY) { 
			if (diskWriteInit(*disk)) nextState = STATE_DISKWRITE;
		} else {
			if (diskReadInit(*disk)) nextState = STATE_BLOCKREAD;
		}
		break;
	}

	case 0x81: DSKCHG(); break;
	case 0x82: GETDPB(); break;
	//case 0x83: CHOICE();
	//case 0x84: DSKFMT();
	case 0x85: DRIVES(); break;
	case 0x86: INIENV(); break;
	case 0x87: setDateMSX(); break;

	case 0x88: nextState = STATE_DEVOPEN; break;
	case 0x89: device.close(command.cmdData); break;
	//case 0x8A: deviceRandomIO(fcb);
	case 0x8B: device.write(command.cmdData); break;
	case 0x8C: device.read(command.cmdData);  break;
	//case 0x8D: deviceEof(fcb);
	case 0x8E: auxIn(); break;
	case 0x8F: auxOut(); break;
	case 0x90: receiveExtraData(); nextState = STATE_MESSAGE; break;
	case 0x91: receiveExtraData(); nextState = STATE_IMAGE; break;
    case 0x92: getDosVersion(); break;
	case 0x93: commandRequested(); break;
	//case 0xFF: vramDump();
	case 0x94: blockReadCmd(); break;
    case 0x95: blockWriteCmd(); break;
    case 0x96: receiveExtraData(); nextState = STATE_CPUINFO; break;
    case 0x97: apiCommand();  break;
	default:
		DBERR("Unknown command! (0x%02x)\n", activeCommand);
		nextState = STATE_SYNC1;
		break;
	}
	setState(nextState);
}

void NowindHost::apiCommand()
{
    DBERR("apiCommand\n");
    command.extraData[0] = 0;
    dumpRegisters();

    byte reg_a = command.getA();
    byte reg_c = command.getC();
    byte reg_b = command.getB();
    switch (reg_c)
    {
        case API_NOWMAP:
            if (reg_a != 4)
            {   
                DBERR("API_NOWMAP received with wrong EXTBIO function (%u)\n", reg_a);
                setState(STATE_SYNC1);
            }
            else
            {
                if (reg_b == 0)
                {
                    DBERR("API_NOWMAP received without commandline!\n");
                    setState(STATE_SYNC1);
                }
                else
                {
					setState(STATE_RECEIVE_DATA);
                }
            }
            break;
        default:
            DBERR("apiCommand_API_??\n");
            setState(STATE_SYNC1);
			break;
    }
}

// todo: split into seporate 'API' class?
void NowindHost::apiReceiveData(byte data)
{
    byte reg_b = command.getB();
    
    command.extraData[recvCount] = data;
    ++recvCount;

    DBERR("apiReceiveData %u/%u: %c\n", recvCount, reg_b, data);

    if (recvCount >= reg_b)
    {
        // all data received, execute command
        byte reg_c = command.getC();
        word reg_de = command.getDE();
        DBERR("\n");

        switch (reg_c)
        {
            case API_NOWMAP:
            {
                string args = string(reinterpret_cast<char*>(&command.extraData[0]));
                string result = nowMap(args);
                vector<byte> resultData;
                resultData.assign(result.begin(), result.end());
                resultData.push_back(0);
                
                blockRead.init(reg_de, resultData.size(), resultData);
                setState(STATE_BLOCKREAD);
                break;
            }
            default:
                DBERR("apiReceiveData_API_??\n");
                setState(STATE_SYNC1);
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

bool NowindHost::diskReadInit(SectorMedium& disk)
{
	bool result = false;
	unsigned sectorAmount = getSectorAmount();
	buffer.resize(sectorAmount * 512);
	unsigned startSector = getStartSector();
    if (!disk.readSectors(&buffer[0], startSector, sectorAmount)) {
		unsigned int size = sectorAmount * 512;
		unsigned address = getStartAddress();
	    
		DBERR("NowindHost::diskRead, startSector: %u  sectorAmount: %u, address: 0x%04x\n", startSector, sectorAmount, address);
		blockRead.init(address, size, buffer);
		result = true;
	}
	return result;
}

bool NowindHost::diskWriteInit(SectorMedium& disk)
{
	bool result = false;
	DBERR("NowindHost::diskWrite, startSector: %u  sectorAmount: %u\n", getStartSector(), getSectorAmount());
	if (disk.isWriteProtected()) 
	{
		response->sendHeader();
		response->send(1);
		response->send(0); // WRITEPROTECTED
	}
	else
	{
		unsigned sectorAmount = std::min(128u, getSectorAmount());
		buffer.resize(sectorAmount * 512);
		transferred = 0;
		doDiskWrite1();
		result = true;
	}
	return result;
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
		response->sendHeader();
		response->send(255);
		setState(STATE_SYNC1);
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
    
	response->sendHeader();
	response->send(0);          // data ahead!
	response->send16(address);
	response->send16(transferSize);
	response->send(0xaa);

	// wait for data
	setState(STATE_DISKWRITE);
	recvCount = 0;
}

void NowindHost::doDiskWrite2()
{
	assert(recvCount == (transferSize + 2));
	for (unsigned i = 0; i < transferSize; ++i) {
		buffer[i + transferred] = command.extraData[i + 1];
	}

	byte seq1 = command.extraData[0];
	byte seq2 = command.extraData[transferSize + 1];
	if ((seq1 == 0xaa) && (seq2 == 0xaa)) {
		// good block received
		transferred += transferSize;

		unsigned address = getCurrentAddress();
		size_t bytesLeft = buffer.size() - transferred;
		if ((address == 0x8000) && (bytesLeft > 0)) {
			response->sendHeader();
			response->send(254); // more data for page 2/3
	        DBERR(" more data for page 2/3\n");
		}
	} else {
	    DBERR(" ERROR!!! This situation is still not handled correctly!\n");
		response->purge();
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
	byte num = command.getA();
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
	response->sendHeader();

	dumpRegisters();
	std::cin >> input;

	response->sendHeader();
	response->send(input);
    DBERR("auxIn returning 0x%02x\n", input);
}

void NowindHost::auxOut()
{
	DBERR("auxOut: %c\n", command.getA());
	dumpRegisters();
	printf("%c", command.getA());
}

void NowindHost::dumpRegisters()
{
	DBERR("AF: 0x%04X, BC: 0x%04X, DE: 0x%04X, HL: 0x%04X, CMD: 0x%02X\n", command.getAF(), command.getBC(), command.getDE(), command.getHL(), command.getCommand());
}

void NowindHost::DSKCHG()
{
	SectorMedium* disk = getDisk();
	if (!disk) {
		// no such drive or no disk inserted
		return;
	}

	response->sendHeader();
	byte num = command.getA();
	assert(num < drives.size());

	if (drives[num]->diskChanged()) {
		response->send(255); // changed
		// read first FAT sector (contains media descriptor)
		byte sectorBuffer[512];
		if (disk->readSectors(sectorBuffer, 1, 1)) {
			// TODO read error
			sectorBuffer[0] = 0;
		}
		response->send(sectorBuffer[0]); // new mediadescriptor
	} else {
		response->send(0);   // not changed
		response->send(255); // dummy
	}
}

void NowindHost::GETDPB()
{
	byte num = command.getA();

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

	response->sendHeader();

	// send destination address
	response->send(command.getE());
	response->send(command.getD());

	byte * sendBuffer = (byte *) &dpb;
	for (int i=0;i<18;i++) {
		DBERR("GETDPB offset [%d]: 0x%02X\n", i+1, sendBuffer[i]);
		response->send(sendBuffer[i]);
	}
}

// msx sends the amount of drives already installed in reg_a
void NowindHost::DRIVES()
{
	byte reg_a = command.getA();
	// at least one drive (MSXDOS1 cannot handle 0 drives)
	byte numberOfDrives = std::max<byte>(1, byte(drives.size()));

    driveOffset = reg_a;
	response->sendHeader();
	response->send(getEnablePhantomDrives() ? 0x02 : 0);
	response->send(reg_a | (getAllowOtherDiskroms() ? 0 : 0x80));
	response->send(numberOfDrives);

	for (unsigned i = 0; i < drives.size(); ++i) {
		if (drives[i]->isRomdisk()) {
			romdisk = i;
			break;
		}
	}
}

void NowindHost::INIENV()
{
	response->sendHeader();
	DBERR("INIENV (romdrv nr: %i) \n", romdisk);
	response->send(romdisk); // calculated in DRIVES()
}

void NowindHost::setDateMSX()
{
	time_t td = time(NULL);
	struct tm* tm = localtime(&td);

	response->sendHeader();
	response->send(tm->tm_mday);          // day
	response->send(tm->tm_mon + 1);       // month
	response->send16(tm->tm_year + 1900); // year
}

unsigned int NowindHost::getSectorAmount() const
{
	return command.getB();
}

unsigned int NowindHost::getStartSector() const
{
	byte reg_c = command.getC();
	unsigned startSector = command.getDE();

	if (reg_c < 0x80) {
		// FAT16 read/write sector
		startSector += reg_c << 16;
	}
	return startSector;
}

unsigned int NowindHost::getStartAddress() const
{
	return command.getHL();
}

unsigned int NowindHost::getCurrentAddress() const
{
	return getStartAddress() + transferred;
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
	byte num = command.getA();
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
	response->sendHeader();
	response->send(enableMSXDOS2 ? 1:0);
}

// the MSX asks whether the host has a command  
// waiting for it to execute
void NowindHost::commandRequested()
{
    char cmdType = command.getB();
    char cmdArg = command.getC();

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

    response->sendHeader();

    std::vector<byte> command;
    if (index >= startupRequestQueue.size())
    {
        response->send(0);   // no more commands 
        DBERR("No more startup commands.\n");
    }
    else
    {
        command = startupRequestQueue.at(index);
        index++;

        for (unsigned int i=0;i<command.size();i++)
        {
            response->send(command[i]);
        }
    }
}

// command from the requestQueue are sent only once, 
// and are them removed from the queue
void NowindHost::commandRequestedAnytime()
{
    response->sendHeader();
    if (requestQueue.empty())
    {
        response->send(0);
    }
    else
    {
        std::vector<byte> command = requestQueue.front();
        // remove command from queue        
        requestQueue.pop_front();
		if (requestQueue.empty())
		{
			response->send(0);
		}
		else
		{
			response->send(1);
		}

        for (unsigned int i=0;i<command.size();i++)
	    {
            response->send(command[i]);
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
