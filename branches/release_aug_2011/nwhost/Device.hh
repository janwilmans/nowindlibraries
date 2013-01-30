#ifndef DEVICE_HH
#define DEVICE_HH

#include "NowindTypes.hh"
#include <string>
#include <memory>
#include <fstream>

namespace nwhost {

class NowindHostSupport;

class Device {
public:
    Device();
    ~Device();
    
	void initialize(NowindHostSupport* aSupport);

	unsigned readHelper1(unsigned dev, char* buffer);
	void readHelper2(unsigned len, const char* buffer);
	int getDeviceNum(const byte* cmdData) const;
	int getFreeDeviceNum(const byte* cmdData);
	void open(const byte* data, const byte* extraData);
	void close(const byte* cmdData);
	void write(const byte* cmdData);
	void read(const byte* cmdData);
	std::string extractName(const byte* cmdData, int begin, int end) const;
	
	word getFCB(const byte* cmdData) const;
	void reset();

private:
    NowindHostSupport* nwhSupport;
    
	static const unsigned MAX_DEVICES = 16;
	struct {
		std::auto_ptr<std::fstream> fs; // not in use when fs == NULL
		unsigned fcb;
	} devices[MAX_DEVICES];    
	
	word fcbAddress;
};

#endif // DEVICE_HH

} // namespace
