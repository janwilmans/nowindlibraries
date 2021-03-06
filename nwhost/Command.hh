#ifndef COMMAND_HH
#define COMMAND_HH

#include "NowindTypes.hh"
#include <vector>
#include <deque>
#include <string>

namespace nwhost {

class NowindHostSupport;

class Command
{
public:
	static const int F_CARRY = 1;

	Command();
	virtual ~Command();
	void initialize(NowindHostSupport* aSupport);

	byte data;                      // last received command byte
	unsigned int time;              // time in ms since start of application the last command byte was received 
	byte cmdData[9];                // reg_[cbedlhfa] + cmd
		
	byte getC() const { return cmdData[0]; }		// cmdData getters
	byte getB() const { return cmdData[1]; }
	byte getE() const { return cmdData[2]; }
	byte getD() const { return cmdData[3]; }
	byte getL() const { return cmdData[4]; }
	byte getH() const { return cmdData[5]; }
	byte getF() const { return cmdData[6]; }
	byte getA() const { return cmdData[7]; }
	byte getCommand() const { return cmdData[8]; }

	word getBC() const { return getC() + 256*getB(); }
	word getDE() const { return getE() + 256*getD(); }
	word getHL() const { return getL() + 256*getH(); }
	word getAF() const { return getF() + 256*getA(); }
	
	word getFCBrecordSize() const { return extraData[14] + (extraData[15] << 8); }
	int getRandomAccessRecord() const  { return extraData[33] + (extraData[34] << 8) + (extraData[35] << 16); }

	std::vector<byte> extraData;		// [240 + 2]; // extra data for image/message/write
	std::string getFilenameFromExtraData() const;
	void reportCpuInfo() const;
	void reportFCB(const unsigned char* data) const;
private:
    NowindHostSupport* nwhSupport;

};

} // namespace nwhost

#endif // COMMAND_HH
