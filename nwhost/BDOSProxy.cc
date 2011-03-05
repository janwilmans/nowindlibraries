
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

void BDOSProxy::WriteRandomBlock(const Command& command, Response& response)
{
    DBERR(" >> WriteRandomBlock\n");
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
    DBERR(" nu hebben we een fcb: %s\n", filename.c_str());
    //bdosFiles.push_back( new fstream(imageName.c_str(), ios::binary | ios::in);
    bdosfile = new fstream(filename.c_str(), ios::binary | ios::in);
	bdosfile->seekg(0, ios::end);
	size_t filesize = bdosfile->tellg();
	bdosfile->seekg(0, ios::beg);

	vector<byte> buffer(command.extraData);
	buffer.resize(36);

	buffer[0x10] = filesize & 0xff;
	buffer[0x11] = (filesize >> 8) & 0xff;
	buffer[0x12] = (filesize >> 16) & 0xff;
	buffer[0x13] = (filesize >> 24) & 0xff;
    
    if (bdosfile->fail())
    {
		blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
    }
    else
    {
		findOpenFile = BDOSCMD_EXECUTING;
		blockRead.init(command.getDE(), buffer.size(), buffer);
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

		// todo filter directories, and add masks *.*
		boost::uintmax_t filesize = 0; // fs::file_size(dirEntry);
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
		if (found) findFirstState = BDOSCMD_EXECUTING;
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
		if (found) findNextState = BDOSCMD_EXECUTING;
#endif
	return found;
}

bool BDOSProxy::ReadRandomBlock(const Command& command, Response& response)
{
	static BdosState readRandomBlockState = BDOSCMD_READY;
	// this is true when the FindNext result is being sent
	if (readRandomBlockState == BDOSCMD_EXECUTING)
	{
		//DBERR("> BDOSProxy::ReadRandomBlock ACK\n");
		blockRead.ack(command.data);
		if (blockRead.isDone())
		{
			readRandomBlockState = BDOSCMD_READY;
			return false;
		}
		return true;
	}

    DBERR("> BDOSProxy::ReadRandomBlock\n");

    word amount = command.getHL();
    std::vector<byte> buffer;
	size_t actuallyRead = 0;
    int offset = 0;
	if (amount > 0)
	{
		buffer.resize(amount);
		bdosfile->seekg(offset);
		bdosfile->read((char*)&buffer[0], amount);
		actuallyRead = bdosfile->gcount();
	}
    
    if (actuallyRead == 0)
    {
        blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
    }
    else
    {
        byte returnCode = BlockRead::BLOCKREAD_ERROR;
        if (actuallyRead == amount)
        {
			returnCode = BlockRead::BLOCKREAD_EXIT;
        }

        DBERR(" >> reg_hl: %u, actuallyRead: %u\n", amount, actuallyRead);
        word dmaAddres = command.getBC();
        blockRead.init(dmaAddres, actuallyRead, buffer, returnCode);
		readRandomBlockState = BDOSCMD_EXECUTING;
    }
	return true;
}

void BDOSProxy::ReadLogicalSector(const Command& command, Response& response)
{
    DBERR(" >> WriteRandomBlock\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteLogicalSector(const Command& command, Response& response)
{
    DBERR(" >> WriteRandomBlock\n");
    command.reportCpuInfo();
}


} // namespace nwhost
