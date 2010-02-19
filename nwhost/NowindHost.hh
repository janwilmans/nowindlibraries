#ifndef NOWINDHOST_HH
#define NOWINDHOST_HH

#include "NowindTypes.hh"
#include <deque>
#include <vector>
#include <string>
#include <memory>
#include <iosfwd>
#include <fstream>

#include "NwhostExports.h"

namespace nwhost {

class DiskHandler;
class SectorMedium;

// first and last 2 bytes and not send (never used anyway)
struct dpbType {
	byte ID;
	byte SECSIZ_L;
	byte SECSIZ_H;
	byte DIRMSK;
	byte DIRSHFT;
	byte CLUSMSK;
	byte CLUSSHFT;
	byte FIRFAT_L;
	byte FIRFAT_H;
	byte FATCNT;
	byte MAXENT;
	byte FIRREC_L;
	byte FIRREC_H;
	byte MAXCLUS_L;
	byte MAXCLUS_H;
	byte FATSIZ;
	byte FIRDIR_L;
	byte FIRDIR_H;
	byte FATADR_L;
	byte FATADR_H;
};

class NWHOST_API NowindHost
{
public:
	NowindHost(const std::vector<DiskHandler*>& drives);
	virtual ~NowindHost();

	// public for usb-host implementation
	bool isDataAvailable() const;

	// read one byte of response-data from the host (msx <- pc)
	byte read();

	// like read(), but without side effects (doesn't consume the data)
	byte peek() const;

	// Write one byte of command-data to the host   (msx -> pc)
	// Time parameter is in milliseconds. Emulators can pass emulation
	// time, usbhost can pass real time.
	void write(byte value, unsigned int time);

	void setAllowOtherDiskroms(bool allow);
	bool getAllowOtherDiskroms() const;

	void setEnablePhantomDrives(bool enable);
	bool getEnablePhantomDrives() const;
	void setEnableMSXDOS2(bool enable);

	// public for serialization
	enum State {
		STATE_SYNC1,     // waiting for AF
		STATE_SYNC2,     // waiting for 05
		STATE_COMMAND,   // waiting for command (9 bytes)
		STATE_DISKREAD,  // waiting for AF07
		STATE_DISKWRITE, // waiting for AA<data>AA
		STATE_DEVOPEN,   // waiting for filename (11 bytes)
		STATE_IMAGE,     // waiting for filename
		STATE_MESSAGE,   // waiting for null-terminated message
		STATE_SPEEDTEST,	// TODO: remove!
	};

	virtual void debugMessage(const char *cFormat, ...) const;

    void clearStartupRequests();
    void addStartupRequest(std::vector<byte> command);
    void clearRequests();
    void addRequest(std::vector<byte> command);
	void getDosVersion();

private:
	void msxReset();
	SectorMedium* getDisk();
	void executeCommand();

	void send(byte value);
	void send16(word value);
	void sendHeader();
	void purge();

	void DRIVES();
	void DSKCHG();
	void GETDPB();
	void CHOICE();
	void INIENV();
	void setDateMSX();

	unsigned getSectorAmount() const;
	unsigned getStartSector() const;
	unsigned getStartAddress() const;
	unsigned getCurrentAddress() const;

	void diskReadInit(SectorMedium& disk);
	void doDiskRead1();
	void doDiskRead2();
	void transferSectors(unsigned transferAddress, unsigned amount);
	void transferSectorsBackwards(unsigned transferAddress, unsigned amount);

	void diskWriteInit(SectorMedium& disk);
	void doDiskWrite1();
	void doDiskWrite2();

	unsigned getFCB() const;
	std::string extractName(int begin, int end) const;
	unsigned readHelper1(unsigned dev, char* buffer);
	void readHelper2(unsigned len, const char* buffer);
	int getDeviceNum() const;
	int getFreeDeviceNum();
	void deviceOpen();
	void deviceClose();
	void deviceWrite();
	void deviceRead();
	void auxIn();
	void auxOut();
	void dumpRegisters();

    void commandRequested();
    void commandRequestedAtStartup(byte reset);
    void commandRequestedAnytime();
	std::deque< std::vector<byte> > startupRequestQueue;   // queue for commandRequest() at startup
	std::deque< std::vector<byte> > requestQueue;   // queue for commandRequest()

	void callImage(const std::string& filename);
	const std::vector<DiskHandler*>& drives;

	// queue
	std::deque<byte> hostToMsxFifo;

    // state-machine
	unsigned lastTime;       // last time a byte was received from MSX
	State state;
	unsigned recvCount;      // how many bytes recv in this state
	byte cmdData[9];         // reg_[cbedlhfa] + cmd
	byte extraData[240 + 2]; // extra data for diskread/write
	std::vector<byte> buffer;// work buffer for diskread/write
	unsigned transferred;     // progress within diskread/write
	unsigned retryCount;     // only used for diskread
	unsigned transferSize;   // size of current chunk

	static const unsigned MAX_DEVICES = 16;
	struct {
		std::auto_ptr<std::fstream> fs; // not in use when fs == NULL
		unsigned fcb;
	} devices[MAX_DEVICES];

	byte romdisk;            // index of romdisk (255 = no romdisk)
	bool allowOtherDiskroms;
	bool enablePhantomDrives;
	bool enableMSXDOS2;

	// TODO: remove when done with testing!
	void speedTest();
};

} // namespace nowind

#endif // NOWINDHOST_HH
