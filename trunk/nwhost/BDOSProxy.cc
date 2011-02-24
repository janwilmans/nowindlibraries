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

#ifdef WIN32
#include <io.h>         // not portable to linux nor MacOS, TODO: fix!
#endif

namespace nwhost {

using std::string;
using std::vector;
using std::fstream;
using std::ios;

BDOSProxy::BDOSProxy()
{
	findFirstState = BDOSCMD_READY;
	findNextState = BDOSCMD_READY;
}

void BDOSProxy::initialize(NowindHostSupport* aSupport)
{
    nwhSupport = aSupport;
	blockRead.initialize(aSupport);
}

BDOSProxy::~BDOSProxy()
{
}

void BDOSProxy::DiskReset(Command& command)
{
    DBERR(" >> DiskReset\n");
    command.reportCpuInfo();
}

void BDOSProxy::CloseFile(Command& command)
{
    DBERR(" >> CloseFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::DeleteFile(Command& command)
{
    DBERR(" >> DeleteFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::ReadSeq(Command& command)
{
    DBERR(" >> ReadSeq\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteSeq(Command& command)
{
    DBERR(" >> WriteSeq\n");
    command.reportCpuInfo();
}

void BDOSProxy::CreateFile(Command& command)
{
    DBERR(" >> CreateFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::RenameFile(Command& command)
{
    DBERR(" >> RenameFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::ReadRandomFile(Command& command)
{
    DBERR(" >> ReadRandomFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteRandomFile(Command& command)
{
    DBERR(" >> WriteRandomFile\n");
    command.reportCpuInfo();
}

void BDOSProxy::GetFileSize(Command& command)
{
    DBERR(" >> GetFileSize\n");
    command.reportCpuInfo();
}

void BDOSProxy::SetRandomRecordField(Command& command)
{
    DBERR(" >> SetRandomRecordField\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteRandomBlock(Command& command)
{
    DBERR(" >> WriteRandomBlock\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteRandomFileWithZeros(Command& command)
{
    DBERR(" >> WriteRandomFileWithZeros\n");
    command.reportCpuInfo();
}

void BDOSProxy::OpenFile(Command& command)
{
    string filename = command.getFilenameFromExtraData();
    DBERR(" nu hebben we een fcb: %s\n", filename.c_str());
    //bdosFiles.push_back( new fstream(imageName.c_str(), ios::binary | ios::in);
    bdosfile = new fstream(filename.c_str(), ios::binary | ios::in);
    nwhSupport->sendHeader();
    
    if (bdosfile->fail())
    {
        nwhSupport->send(0xff);
    }
    else
    {
        nwhSupport->send(0);
    }
}

// returns true when the command is still executing.
bool BDOSProxy::FindFirst(Command& command)
{
	// this is true when the FindFirst result is being sent
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
    word reg_hl = command.cmdData[4] + 256*command.cmdData[5];
    string filename = command.getFilenameFromExtraData();
    
#ifdef WIN32
    struct _finddata_t data;
    findFirstHandle = _findfirst(filename.c_str(), &data); 
    if (findFirstHandle == -1)
    {
        blockRead.cancelWithCode(128);  // file not found
		DBERR("BDOSProxy::FindFirst <file not found>\n");
		findFirstState = BDOSCMD_READY;
    }
    else
    {
        string filename(data.name);
        ToUpper(filename);
        vector<byte> buffer;
        getVectorFromFileName(buffer, filename);
        
        blockRead.init(reg_hl, buffer.size(), buffer);
        findFirstState = BDOSCMD_EXECUTING;
		found = true;
		DBERR("file: %s size: %u\n", data.name, data.size);
    }
    
    
#else
    // linux not implemented
    assert(false);
#endif
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

bool BDOSProxy::FindNext(Command& command)
{
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
    word reg_hl = command.cmdData[4] + 256*command.cmdData[5];
    
#ifdef WIN32
    struct _finddata_t data;
    long result = _findnext(findFirstHandle, &data); 
    if (result != 0)
    {
        blockRead.cancelWithCode(128);
		DBERR("BDOSProxy::FindNext <file not fond>\n");
    }
    else
    {
        string filename(data.name);
        ToUpper(filename);

        vector<byte> buffer;
        getVectorFromFileName(buffer, filename);
        blockRead.init(reg_hl, buffer.size(), buffer);
		findNextState = BDOSCMD_EXECUTING;
		found = true;
		DBERR("file: %s size: %u\n", data.name, data.size);
    }
    
#else
    // linux not implemented
    assert(false);
#endif
	return found;
}

bool BDOSProxy::ReadRandomBlock(Command& command)
{
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

    word reg_bc = command.cmdData[0] + 256*command.cmdData[1];
    word reg_hl = command.cmdData[4] + 256*command.cmdData[5];

    std::vector<byte> buffer;
    int size = reg_hl;
    int offset = 0;
    buffer.resize(size);
    bdosfile->seekg(offset);
    bdosfile->read((char*)&buffer[0], size);
    size_t actuallyRead = bdosfile->gcount();
    
    if (actuallyRead == 0)
    {
        blockRead.cancelWithCode(BlockRead::BLOCKREAD_ERROR);
    }
    else
    {
        byte returnCode = BlockRead::BLOCKREAD_ERROR;
        if (actuallyRead == reg_hl)
        {
			returnCode = BlockRead::BLOCKREAD_EXIT;
        }

        DBERR(" >> reg_hl: %u, actuallyRead: %u\n", reg_hl, actuallyRead);
        word dmaAddres = reg_bc;
        blockRead.init(dmaAddres, actuallyRead, buffer, returnCode);
		readRandomBlockState = BDOSCMD_EXECUTING;
    }
	return true;
}

void BDOSProxy::ReadLogicalSector(Command& command)
{
    DBERR(" >> WriteRandomBlock\n");
    command.reportCpuInfo();
}

void BDOSProxy::WriteLogicalSector(Command& command)
{
    DBERR(" >> WriteRandomBlock\n");
    command.reportCpuInfo();
}


} // namespace nwhost