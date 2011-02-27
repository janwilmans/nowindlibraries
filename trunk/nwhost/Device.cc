#include "Device.hh"
#include "NowindHostSupport.hh"

#include <cassert>

using namespace std;

namespace nwhost {

Device::Device()
{
}

void Device::initialize(NowindHostSupport* aSupport)
{
    nwhSupport = aSupport;
}

Device::~Device()
{
}

// TODO: Aaldert: omschrijven naar open(const Command& command, Response& response)
void Device::open(const byte* cmdData, const byte* extraData)
{
	word fcbAddress = getFCB(cmdData);
	byte openMode = cmdData[2]; // reg_e 
	byte reg_d = cmdData[3];    // reg_d

	string filename = extractName(extraData, 0, 8);
	string ext      = extractName(extraData, 8, 11);
	if (!ext.empty()) {
		filename += '.';
		filename += ext;
	}

	unsigned dev = getFreeDeviceNum(cmdData);
	devices[dev].fs.reset(new fstream()); // takes care of deleting old fs
	devices[dev].fcb = fcbAddress;

	nwhSupport->sendHeader();
	byte errorCode = 0;
	switch (openMode) {
	case 1: // read-only mode
		devices[dev].fs->open(filename.c_str(), ios::in  | ios::binary);
		errorCode = 53; // file not found
		break;
	case 2: // create new file, write-only
		devices[dev].fs->open(filename.c_str(), ios::out | ios::binary);
		errorCode = 56; // bad file name
		break;
	case 8: // append to existing file, write-only
		devices[dev].fs->open(filename.c_str(), ios::out | ios::binary | ios::app);
		errorCode = 53; // file not found
		break;
	case 4:
		nwhSupport->send(58); // sequential I/O only
		return;
	default:
		nwhSupport->send(0xFF); // TODO figure out a good error number
		return;
	}
	assert(errorCode != 0);
	if (devices[dev].fs->fail()) {
		devices[dev].fs.reset();
		nwhSupport->send(errorCode);
		return;
	}

	unsigned readLen = 0;
	bool eof = false;
	char buffer[256];
	if (openMode == 1) {
		// read-only mode, already buffer first 256 bytes
		readLen = readHelper1(dev, buffer);
		assert(readLen <= 256);
		eof = readLen < 256;
	}

	nwhSupport->send(0x00); // no error
	nwhSupport->send16(fcbAddress);
	nwhSupport->send16(9 + readLen + (eof ? 1 : 0)); // number of bytes to transfer

	nwhSupport->send(openMode);
	nwhSupport->send(0);
	nwhSupport->send(0);
	nwhSupport->send(0);
	nwhSupport->send(reg_d); // reg_d
	nwhSupport->send(0);
	nwhSupport->send(0);
	nwhSupport->send(0);
	nwhSupport->send(0);

	if (openMode == 1) {
		readHelper2(readLen, buffer);
	}
}

void Device::close(const byte* cmdData)
{
	int dev = getDeviceNum(cmdData);
	if (dev == -1) return;
	devices[dev].fs.reset();
}

void Device::write(const byte* cmdData)
{
    char data = cmdData[0]; // reg_c
	int dev = getDeviceNum(cmdData);
	if (dev == -1) return;
	devices[dev].fs->write(&data, 1);
}

void Device::read(const byte* cmdData)
{
	int dev = getDeviceNum(cmdData);
	if (dev == -1) return;

	char buffer[256];
	unsigned readLen = readHelper1(dev, buffer);
	bool eof = readLen < 256;
	nwhSupport->send(0xAF);
	nwhSupport->send(0x05);
	nwhSupport->send(0x00); // dummy
	nwhSupport->send16(getFCB(cmdData) + 9);
	nwhSupport->send16(readLen + (eof ? 1 : 0));
	readHelper2(readLen, buffer);
}

unsigned Device::getFCB(const byte* cmdData) const
{
	byte reg_l = cmdData[4];
	byte reg_h = cmdData[5];
	return reg_h * 256 + reg_l;
}

unsigned Device::readHelper1(unsigned dev, char* buffer)
{
	assert(dev < MAX_DEVICES);
	unsigned len = 0;
	for (/**/; len < 256; ++len) {
		devices[dev].fs->read(&buffer[len], 1);
		if (devices[dev].fs->eof()) break;
	}
	return len;
}

void Device::readHelper2(unsigned len, const char* buffer)
{
	for (unsigned i = 0; i < len; ++i) {
		nwhSupport->send(buffer[i]);
	}
	if (len < 256) {
		nwhSupport->send(0x1A); // end-of-file
	}
}

string Device::extractName(const byte* extraData, int begin, int end) const
{
	string result;
	for (int i = begin; i < end; ++i) {
		char c = extraData[i];
		if (c == ' ') break;
		result += toupper(c);
	}
	return result;
}

int Device::getDeviceNum(const byte* cmdData) const
{
	unsigned fcb = getFCB(cmdData);
	for (unsigned i = 0; i < MAX_DEVICES; ++i) {
		if (devices[i].fs.get() &&
		    devices[i].fcb == fcb) {
			return i;
		}
	}
	return -1;
}

int Device::getFreeDeviceNum(const byte* cmdData)
{
	int dev = getDeviceNum(cmdData);
	if (dev != -1) {
		// There already was a device open with this fcb address,
		// reuse that device.
		return dev;
	}
	// Search for free device.
	for (unsigned i = 0; i < MAX_DEVICES; ++i) {
		if (!devices[i].fs.get()) {
			return i;
		}
	}
	// All devices are in use. This can't happen when the MSX software
	// functions correctly. We'll simply reuse the first device. It would
	// be nicer if we reuse the oldest device, but that's harder to
	// implement, and actually it doesn't really matter.
	return 0;
}

void Device::reset()
{
	for (unsigned i = 0; i < MAX_DEVICES; ++i) {
		devices[i].fs.reset();
	}
}
} // namespace