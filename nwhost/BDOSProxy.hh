#ifndef BDOSPROXY_HH
#define BDOSPROXY_HH

#include "NowindTypes.hh"
#include "BlockRead.hh"
#include "ReceiveRegisters.hh"
#include <vector>
#include <deque>
#include <string>

#ifdef WIN32
#define WIN_FILESYSTEM
#endif

#ifdef WIN_FILESYSTEM
// not portable to linux nor MacOS, TODO: fix!
// Boost.FileSystem could do this, it is a seporately compiled library (not header only)
#include <io.h>  

#else

#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp" 
#include <iostream>
using namespace boost::filesystem;

#endif


namespace nwhost {

class NowindHostSupport;
class Command;
class Response;

class BDOSProxy
{
public:
	BDOSProxy();
	virtual ~BDOSProxy();
	void initialize(NowindHostSupport* aSupport);

	void DiskReset(const Command& command, Response& response);
	bool OpenFile(const Command& command, Response& response);
    void CloseFile(const Command& command, Response& response);
    bool FindFirst(const Command& command, Response& response);
    bool FindNext(const Command& command, Response& response);
	void DeleteFile(const Command& command, Response& response);
	void ReadSeq(const Command& command, Response& response);
	void WriteSeq(const Command& command, Response& response);
	void CreateFile(const Command& command, Response& response);
	void RenameFile(const Command& command, Response& response);
	void ReadRandomFile(const Command& command, Response& response);
	void WriteRandomFile(const Command& command, Response& response);
	void GetFileSize(const Command& command, Response& response);
	void SetRandomRecordField(const Command& command, Response& response);
	void RandomBlockWrite(const Command& command, Response& response);
	bool RandomBlockRead(const Command& command, Response& response);
	void WriteRandomFileWithZeros(const Command& command, Response& response);
	void ReadLogicalSector(const Command& command, Response& response);
	void WriteLogicalSector(const Command& command, Response& response);

	enum BdosState {
		BDOSCMD_EXECUTING,
		BDOSCMD_RECEIVE_REGISTERS,
		BDOSCMD_READY,
	};

private:
    NowindHostSupport* nwhSupport;
	BlockRead blockRead;
	ReceiveRegisters receiveRegisters;

    long findFirstHandle;
    void getVectorFromFileName(std::vector<byte>& buffer, std::string filename);
    std::vector<std::fstream*> bdosFiles;
    std::fstream* bdosfile;

#ifndef WIN_FILESYSTEM
	bool FindFileResponse(const Command& command, Response& response);
	boost::filesystem::directory_iterator findFirstIterator;
#endif

};

} // namespace nwhost

#endif // BDOSPROXY_HH
