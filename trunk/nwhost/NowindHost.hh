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
#include "Device.hh"

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
		STATE_COMMAND,   // waiting for command (9 bytes)
		STATE_DISKWRITE, // waiting for AA<data>AA
		STATE_DEVOPEN,   // waiting for filename (11 bytes)
		STATE_IMAGE,     // waiting for filename
		STATE_MESSAGE,   // waiting for null-terminated message
		STATE_BLOCKREAD, // in block-transfer
		STATE_CPUINFO,   // receiving slot/stack info
		STATE_RECEIVE_DATA,    // receive data for API command
		STATE_RECEIVE_STRING,  // receive string for API command
		STATE_BDOS_OPEN_FILE,  // receive FCB
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
    void receiveExtraData();
	void msxReset();
	SectorMedium* getDisk();
	void executeCommand();
	void reportCpuInfo();
	
	void apiCommand();
	void apiReceiveData(byte data);
	void apiReceiveString(byte data);

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
    
    void blockReadCmd();
    void blockWriteCmd();

	void diskReadInit(SectorMedium& disk);
	void diskWriteInit(SectorMedium& disk);
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
	byte cmdData[9];         // reg_[cbedlhfa] + cmd
	byte extraData[240 + 2]; // extra data for image/message/write
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
    Device device;

    unsigned int timer1;
    unsigned int timer2;
        
    void BDOS_0FH_OpenFile();
    void BDOS_OpenFile();
    void BDOS_10H_CloseFile();    
    void BDOS_27H_ReadRandomBlock(); 
    
    std::vector<std::fstream* > bdosFiles;
    std::fstream* bdosfile;
    
protected:    
    NowindHostSupport* nwhSupport;          // pointer to the NowindHostSupport instance that is actually used (can be subclassed to provide implemenation specific debug-support)
    
};

} // namespace nowind

#endif // NOWINDHOST_HH
