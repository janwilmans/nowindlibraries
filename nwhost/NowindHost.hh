#ifndef NOWINDHOST_HH
#define NOWINDHOST_HH

#include "NowindTypes.hh"
#include <deque>
#include <vector>
#include <string>
#include <iosfwd>

#include "NwhostExports.h"
#include "DataBlock.hh"
#include "BlockRead.hh"
#include "Command.hh"
#include "BDOSProxy.hh"
#include "Device.hh"
#include "Response.hh"

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

// although this enum is used in only one place
// it allows easy searching/matching between host and rom code.
enum {
    C_DSKIO       = 0x80,
    C_DSKCHG      = 0x81,
    C_GETDPB      = 0x82,
    C_CHOICE      = 0x83,
    C_DSKFMT      = 0x84,
    C_DRIVES      = 0x85,
    C_INIENV      = 0x86,
    C_GETDATE     = 0x87,
    C_DEVICEOPEN  = 0x88,
    C_DEVICECLOSE = 0x89,
    C_DEVICERNDIO = 0x8a,
    C_DEVICEWRITE = 0x8b,
    C_DEVICEREAD  = 0x8c,
    C_DEVICEEOF   = 0x8d,
    C_AUXIN       = 0x8e,
    C_AUXOUT      = 0x8f,
    C_MESSAGE     = 0x90,
    C_CHANGEIMAGE  = 0x91,
    C_GETDOSVERSION = 0x92,
    C_CMDREQUEST  = 0x93,
    C_BLOCKREAD   = 0x94,
    C_BLOCKWRITE  = 0x95,
    C_CPUINFO     = 0x96,
    C_COMMAND     = 0x97,
    C_STDOUT      = 0x98
};

class NWHOST_API NowindHost
{
public:
	NowindHost(const std::vector<DiskHandler*>& drives);
	virtual ~NowindHost();
    virtual void initialize();

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
		STATE_RECEIVE_COMMAND,   // waiting for command (9 bytes)
		STATE_RECEIVE_PARAMETERS,   // waiting for parameters (n bytes)
		STATE_EXECUTING_COMMAND,
		STATE_DISKWRITE, // waiting for AA<data>AA
		STATE_DEVOPEN,   // waiting for filename (11 bytes)
		STATE_IMAGE,     // waiting for filename
		STATE_MESSAGE,   // waiting for null-terminated message
		STATE_BLOCKREAD, // in block-transfer
		STATE_CPUINFO,   // receiving slot/stack info
		STATE_RECEIVE_DATA,    // receive data for API command
		STATE_RECEIVE_STRING,  // receive string for API command
		STATE_BDOS_OPEN_FILE,  // receive FCB
		STATE_BDOS_FIND_FIRST, // receive FCB
	};
	
	enum ApiCommands {
		API_NOWMAP = 0
	};	
	

	virtual void debugMessage(const char *cFormat, ...) const;

    void clearStartupRequests();
    void addStartupRequest(std::vector<byte> command);
    void clearRequests();
    void addRequest(std::vector<byte> command);
	void getDosVersion();
	std::string nowMap(std::string arguments);

private:
	void setState(State aState);
    void receiveExtraData(); // remove
	void msxReset();
	SectorMedium* getDisk();
	void executeCommand();
	void prepareCommand();
	
	void apiCommand();
	void apiReceiveData(byte data);
	void apiReceiveString(byte data);

	void DRIVES();
	void DSKCHG();
	void GETDPB();
	void CHOICE();
	void INIENV();
	void getDate();
	
	void stdOutCatch();

	unsigned int getSectorAmount() const;
	unsigned int  getStartSector() const;
	unsigned int  getStartAddress() const;
	unsigned int  getCurrentAddress() const;

	bool diskReadInit(SectorMedium& disk);
	bool diskWriteInit(SectorMedium& disk);
	void doDiskWrite1();
	void doDiskWrite2();

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

    // state-machine
	unsigned lastTime;       // last time a byte was received from MSX
	State state;
	unsigned recvCount;      // how many bytes recv in this state
	std::vector<byte> buffer;// work buffer for sector read/write
	unsigned transferred;    // progress within diskwrite
	unsigned transferSize;   // size of current diskwrite chunk

	byte romdisk;            // index of romdisk (255 = no romdisk)
	byte driveOffset;
	bool allowOtherDiskroms;
	bool enablePhantomDrives;
	bool enableMSXDOS2;
    bool transferingToPage01;   // used to known in which state the MSX is during block-tranfers
    
    BlockRead blockRead;
	BDOSProxy bdosProxy;
	Command command;
	Response* response;
    Device device;

	unsigned int parameterLength;
	unsigned int activeCommand;

    unsigned int timer1;
    unsigned int timer2;
    
protected:
    NowindHostSupport* nwhSupport;          // pointer to the NowindHostSupport instance that is actually used (can be subclassed to provide implemenation specific debug-support)
    
};

} // namespace nowind

#endif // NOWINDHOST_HH
