#ifndef COMMAND_HH
#define COMMAND_HH

#include "NowindTypes.hh"
#include <vector>
#include <deque>

namespace nwhost {

class NowindHostSupport;

class Command
{
public:
	Command();
	virtual ~Command();
	void initialize(NowindHostSupport* aSupport);

	byte data;
	unsigned int time;
	byte cmdData[9];         // reg_[cbedlhfa] + cmd
	byte extraData[240 + 2]; // extra data for image/message/write
		
	std::string getFilenameFromExtraData();
	void reportCpuInfo();
private:
    NowindHostSupport* nwhSupport;

};

} // namespace nwhost

#endif // COMMAND_HH
