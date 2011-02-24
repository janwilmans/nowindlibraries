#ifndef BDOSPROXY_HH
#define BDOSPROXY_HH

#include "NowindTypes.hh"
#include "BlockRead.hh"
#include <vector>
#include <deque>

namespace nwhost {

class NowindHostSupport;
class Command;

class BDOSProxy
{
public:
	BDOSProxy();
	virtual ~BDOSProxy();
	void initialize(NowindHostSupport* aSupport);

	void DiskReset(Command& command);
	void OpenFile(Command& command);
    void CloseFile(Command& command);
    bool FindFirst(Command& command);
    bool FindNext(Command& command);
	void DeleteFile(Command& command);
	void ReadSeq(Command& command);
	void WriteSeq(Command& command);
	void CreateFile(Command& command);
	void RenameFile(Command& command);
	void ReadRandomFile(Command& command);
	void WriteRandomFile(Command& command);
	void GetFileSize(Command& command);
	void SetRandomRecordField(Command& command);
	void WriteRandomBlock(Command& command);
	bool ReadRandomBlock(Command& command);
	void WriteRandomFileWithZeros(Command& command);
	void ReadLogicalSector(Command& command);
	void WriteLogicalSector(Command& command);

	enum BdosState {
		BDOSCMD_EXECUTING,
		BDOSCMD_READY,
	};

private:
    NowindHostSupport* nwhSupport;
	BlockRead blockRead;

	BdosState findFirstState;
	BdosState findNextState;
	BdosState readRandomBlockState;

    long findFirstHandle;
    void getVectorFromFileName(std::vector<byte>& buffer, std::string filename);
    std::vector<std::fstream* > bdosFiles;
    std::fstream* bdosfile;

};

} // namespace nwhost

#endif // BDOSPROXY_HH
