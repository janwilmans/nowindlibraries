
#include "BDOSProxy.hh"
#include "Command.hh"
#include "NowindHostSupport.hh"

#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#define DBERR nwhSupport->debugMessage
#define ToUpper(s) std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) toupper)

namespace nwhost {

using std::string;
using std::vector;
using std::fstream;
using std::ios;

BDOSProxy::BDOSProxy()
{
}

void BDOSProxy::initialize(NowindHostSupport* aSupport)
{
    nwhSupport = aSupport;
	blockRead.initialize(aSupport);
	receiveRegisters.initialize(aSupport);
}

BDOSProxy::~BDOSProxy()
{
}

void BDOSProxy::DiskReset(const Command& command, Response& response)
{
    DBERR(" >> DiskReset\n");
    command.reportCpuInfo();
}

void BDOSProxy::CloseFile(const Command& command, Response& response)
{
    DBERR(" >> CloseFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::DeleteFile(const Command& command, Response& response)
{
    DBERR(" >> DeleteFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::ReadSeq(const Command& command, Response& response)
{
    DBERR(" >> ReadSeq\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteSeq(const Command& command, Response& response)
{
    DBERR(" >> WriteSeq\n");
    command.reportCpuInfo();
}

void BDOSProxy::CreateFile(const Command& command, Response& response)
{
    DBERR(" >> CreateFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::RenameFile(const Command& command, Response& response)
{
    DBERR(" >> RenameFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::ReadRandomFile(const Command& command, Response& response)
{
    DBERR(" >> ReadRandomFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteRandomFile(const Command& command, Response& response)
{
    DBERR(" >> WriteRandomFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::GetFileSize(const Command& command, Response& response)
{
    DBERR(" >> GetFileSize\n");
    command.reportCpuInfo();
}

void BDOSProxy::SetRandomRecordField(const Command& command, Response& response)
{
    DBERR(" >> SetRandomRecordField\n");
    command.reportCpuInfo();
}

void BDOSProxy::RandomBlockWrite(const Command& command, Response& response)
{
    DBERR(" >> RandomBlockWrite\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteRandomFileWithZeros(const Command& command, Response& response)
{
    DBERR(" >> WriteRandomFileWithZeros\n");
    command.reportCpuInfo();
}

bool BDOSProxy::OpenFile(const Command& command, Response& response)
{
	static BdosState findOpenFile = BDOSCMD_READY;

	// this is true when the response is being sent
	if (findOpenFile == BDOSCMD_EXECUTING)
	{
		blockRead.ack(command.data);
		if (blockRead.isDone())
		{
			findOpenFile = BDOSCMD_READY;
			return false;
		}
		return true;
	}

	bool found = false;
    string filename = command.getFilenameFromExtraData();
    
    command.reportFCB(&command.extraData[0]);
    
    // TODO: use findfirst here to support wildcards!
    // TODO: maybe handle DOS device names (e.g. con, lpt, aux)
    DBERR(" nu hebben we een fcb: %s\n", filename.c_str());
    //bdosFiles.push_back( new fstream(imageName.c_str(), ios::binary | ios::in);
    bdosfile = new fstream(filename.c_str(), ios::binary | ios::in);
	bdosfile->seekg(0, ios::end);
	size_t filesize = bdosfile->tellg();
	bdosfile->seekg(0, ios::beg);

	vector<byte> buffer(command.extraData);
	buffer.resize(37);

	buffer[0x0e] = 0;   // reset extent high byte
	buffer[0x0f] = 1;
	
	buffer[0x10] = filesize & 0xff;
	buffer[0x11] = (filesize >> 8) & 0xff;
	buffer[0x12] = (filesize >> 16) & 0xff;
	buffer[0x13] = (filesize >> 24) & 0xff;
	
	command.reportFCB(&buffer[0]);
    
    if (bdosfile->fail())
    {
		blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
		findOpenFile = BDOSCMD_READY;
    }
    else
    {
		blockRead.init(command.getDE(), buffer.size(), buffer);
		findOpenFile = BDOSCMD_EXECUTING;
		found = true;
    }
	return found;
}

void BDOSProxy::getVectorFromFileName(vector<byte>& buffer, string filename)
{
    buffer.resize(12);
    string file = filename;
    string ext;
    
    if (filename == "." || filename == "..")
    {
    
    }
    else
    {
        int point = filename.find('.');   
        if (point != -1)
        {
            file = filename.substr(0, point);
            ext = filename.substr(point+1);
        }
    }

    if (file.size() > 8) file.resize(8);
    if (ext.size() > 3) ext.resize(3);
    for (size_t i=1; i< buffer.size(); i++)
    {
        buffer[i] = 32;
    }
        
    buffer[0] = 0;
    for (size_t i=0; i < file.size() ;i++)
    {
        buffer[i+1] = file[i];
    }
    for (size_t i=0; i < ext.size(); i++)
    {
        buffer[i+9] = ext[i];
    }

}

#ifndef WIN_FILESYSTEM

bool BDOSProxy::FindFileResponse(const Command& command, Response& response)
{
	bool found = false;
	boost::filesystem::directory_iterator noMoreFiles; // past the end
	if (findFirstIterator != noMoreFiles)
	{
		path dirEntry = findFirstIterator->path().string();
		std::string filename = dirEntry.filename().string();
		std::string stem = dirEntry.filename().stem().string();
		std::string ext = dirEntry.filename().extension().string();

		boost::uintmax_t filesize = 0;
		if (!is_directory(dirEntry))
		{
			// todo filter directories, and add masks *.*
			filesize = file_size(dirEntry);
		}

		ToUpper(filename);
		vector<byte> buffer;
		getVectorFromFileName(buffer, filename);
		buffer.resize(36);

		buffer[0x1d] = filesize & 0xff;
		buffer[0x1e] = (filesize >> 8) & 0xff;
		buffer[0x1f] = (filesize >> 16) & 0xff;
		buffer[0x20] = (filesize >> 24) & 0xff;

		blockRead.init(command.getHL(), buffer.size(), buffer);
		found = true;
		DBERR("file: %s size: %u\n", filename.c_str(), filesize);
	}
	return found;
}

#endif 

// todo: load "thexder.bas" does not return to state_sync1!

// returns true when the command is still executing.
bool BDOSProxy::FindFirst(const Command& command, Response& response)
{
	static BdosState findFirstState = BDOSCMD_READY;

	// this is true when the FindFirst response is being sent
	if (findFirstState == BDOSCMD_EXECUTING)
	{
		blockRead.ack(command.data);
		if (blockRead.isDone())
		{
			findFirstState = BDOSCMD_READY;
			return false;
		}
		return true;
	}

	bool found = false;
    DBERR("%s\n", __FUNCTION__);
    string filename = command.getFilenameFromExtraData();
    
#ifdef WIN_FILESYSTEM
    struct _finddata_t data;
    findFirstHandle = _findfirst(filename.c_str(), &data); 
    if (findFirstHandle == -1)
    {
        blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
		DBERR("BDOSProxy::FindFirst <file not found>\n");
		findFirstState = BDOSCMD_READY;
    }
    else
    {
		size_t filesize = data.size;
        string filename(data.name);
        ToUpper(filename);
        vector<byte> buffer;
        getVectorFromFileName(buffer, filename);
		buffer.resize(36);

		buffer[0x1d] = filesize & 0xff;
		buffer[0x1e] = (filesize >> 8) & 0xff;
		buffer[0x1f] = (filesize >> 16) & 0xff;
		buffer[0x20] = (filesize >> 24) & 0xff;

        blockRead.init(command.getHL(), buffer.size(), buffer);
        findFirstState = BDOSCMD_EXECUTING;
		found = true;
		DBERR("file: %s size: %u\n", data.name, data.size);
    }
#else
		const std::string targetPath = ".";
		findFirstIterator = directory_iterator(targetPath);
		found = FindFileResponse(command, response);
		if (found) 
		{
			findFirstState = BDOSCMD_EXECUTING;
		}
		else
		{
			blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
			findFirstState = BDOSCMD_READY;
		}
#endif
	return found;
}

bool BDOSProxy::FindNext(const Command& command, Response& response)
{
	static BdosState findNextState = BDOSCMD_READY;

	// this is true when the FindNext result is being sent
	if (findNextState == BDOSCMD_EXECUTING)
	{
		blockRead.ack(command.data);
		if (blockRead.isDone())
		{
			findNextState = BDOSCMD_READY;
			return false;
		}
		return true;
	}

	bool found = false;

#ifdef WIN_FILESYSTEM
    struct _finddata_t data;
    long result = _findnext(findFirstHandle, &data); 
    if (result != 0)
    {
        blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
		DBERR("BDOSProxy::FindNext <file not fond>\n");
    }
    else
    {
		size_t filesize = data.size;
        string filename(data.name);
        ToUpper(filename);

        vector<byte> buffer;
        getVectorFromFileName(buffer, filename);
		buffer.resize(36);

		//todo: find out why this is offset +1 ? size should start at 0x0c (getVectorFromFileName returns prefixed 0 !?) 8+3 = 11, not 12
		buffer[0x1d] = filesize & 0xff;
		buffer[0x1e] = (filesize >> 8) & 0xff;
		buffer[0x1f] = (filesize >> 16) & 0xff;
		buffer[0x20] = (filesize >> 24) & 0xff;

        blockRead.init(command.getHL(), buffer.size(), buffer);
		findNextState = BDOSCMD_EXECUTING;
		found = true;
		DBERR("file: %s size: %u\n", data.name, data.size);
    }
    
#else
		boost::filesystem::directory_iterator noMoreFiles; // past the end
		++findFirstIterator;
		found = FindFileResponse(command, response);
		if (found) 
		{
			findNextState = BDOSCMD_EXECUTING;
		}
		else
		{
			blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
			findNextState = BDOSCMD_READY;
		}
#endif
	return found;
}

bool BDOSProxy::RandomBlockRead(const Command& command, Response& response)
{
    static word recordsRequested = 0;
    static word recordsSend = 0;
	static BdosState RandomBlockReadState = BDOSCMD_READY;
	static bool endOfFileReached = false;

#define ENABLE_RECEIVE_REGISTERS

#ifdef ENABLE_RECEIVE_REGISTERS
	// this is true when the RandomBlockRead response (an FCB) is being sent
	if (RandomBlockReadState == BDOSCMD_EXECUTING)
	{
		DBERR("> BDOSProxy::RandomBlockRead ACK\n");
		blockRead.ack(command.data);
		
		// the block data is done, continue to execute 'ReceiveRegisters'
		if (blockRead.isDone())
		{
			RandomBlockReadState = BDOSCMD_RECEIVE_REGISTERS;
			receiveRegisters.clear();
			receiveRegisters.setA(0);
			receiveRegisters.setF(0);
			if (endOfFileReached)
			{
			    receiveRegisters.setBC(1);
			}
			else
			{
			    receiveRegisters.setBC(0);
			}
			
			receiveRegisters.setDE(0);
			receiveRegisters.setHL(recordsSend);
			receiveRegisters.send();
			return true;
        }
		return true; // true means the command is not done, so call me again when data arrives
	}
	else if (RandomBlockReadState == BDOSCMD_RECEIVE_REGISTERS)
	{
	    receiveRegisters.ack(command.data);
	    if (receiveRegisters.isDone())
	    {
	        return false; // false means the command is done, so dont call me again.
	    }
	    return true;
	}
#else

	if (RandomBlockReadState == BDOSCMD_EXECUTING)
	{
		DBERR("> BDOSProxy::RandomBlockRead ACK\n");
		blockRead.ack(command.data);
		
		// the block data is done, continue to execute 'ReceiveRegisters'
		if (blockRead.isDone())
		{
			RandomBlockReadState = BDOSCMD_READY;
			return false;
		}
		return true; // true means the command is not done, so call me again when data arrives
	}
	
#endif
	
	command.reportFCB(&command.extraData[0]);

    DBERR("> BDOSProxy::RandomBlockRead\n");

    word recordSize = 128;  // assumption always 128 byte record size!?
    recordsRequested = command.getHL();
    std::vector<byte> buffer;
	size_t actuallyRead = 0;
    int offset = 0;
    int amount = recordsRequested*recordSize; //todo: round up
	if (amount > 0)
	{
		buffer.resize(amount); 
		bdosfile->seekg(offset);
		bdosfile->read((char*)&buffer[0], amount);
		endOfFileReached = bdosfile->eof();
		actuallyRead = bdosfile->gcount();
		recordsSend = actuallyRead / recordSize;   
	}
	DBERR(" >> reg_hl: %u, records: %u (%u bytes)\n", recordsRequested, recordsSend, actuallyRead);
    
    if (actuallyRead == 0)
    {
		DBERR(" * RandomBlockReadState = BDOSCMD_READY\n");
        blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
		RandomBlockReadState = BDOSCMD_READY;
		return false;
    }
    else
    {
        byte returnCode = BlockRead::BLOCKREAD_ERROR;
        if (actuallyRead == amount)
        {
			returnCode = BlockRead::BLOCKREAD_EXIT;
        }
		DBERR(" * RandomBlockReadState = BDOSCMD_EXECUTING\n");
        word dmaAddres = command.getBC();
        blockRead.init(dmaAddres, actuallyRead, buffer, returnCode);
		RandomBlockReadState = BDOSCMD_EXECUTING;
    }
	return true;
}

void BDOSProxy::ReadLogicalSector(const Command& command, Response& response)
{
    DBERR(" >> RandomBlockWrite\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteLogicalSector(const Command& command, Response& response)
{
    DBERR(" >> RandomBlockWrite\n");
    command.reportCpuInfo();
}


} // namespace nwhost
