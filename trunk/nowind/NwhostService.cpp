/**
 * @file NwhostService.cpp
 *
 * @brief Contains methods to service a nowind interface 
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */

#include <fstream>
#include <assert.h>

/*
 * If an "error while loading shared libraries: libX11.so.6" occurs at runtime
 * SDL is not compiled right for your current installation.
 * (sdl.so is depending on libX11.so.6 even if an X11 function is never actually used)
 * re-compiling SDL with ./configure --disable-video-x11 or installing x11 will solve this
 */
#define NOWIND_API_EXPORT
#include "NwhostService.h"

#include <libftdx.h>
#include <libnwhost.h>

#include <libgeneral.h>
#include <algorithm>

using namespace nowind;
using namespace ftdx;
using namespace nwhost;
using namespace general;
using namespace std;

#ifndef WIN32
# include <fcntl.h>  // for open() and O_RDRW
#endif

#define LCase(s) std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) tolower)

/* initialize static member variables */

UsbStream * NwhostService::mUsbStream = 0;
unsigned long NwhostService::mStartTime = 0;
unsigned long NwhostService::mTotalTime = 0;
unsigned long NwhostService::mTransferredBytes = 0;
bool NwhostService::mDebug = false;


void NwhostService::initialize()
{
	// set callbacks (before nowindusb_startup)
    nowindusb_set_debug_callback(&debugout);
}

NwhostService::NwhostService() {
	mUsbStream = 0;
}

NwhostService::~NwhostService() {
    Util::debug("~NwhostService\n");
	delete mUsbStream;
    mUsbStream = 0;
}

void NwhostService::setParameters(int argc, char *argv[])
{
	this->argc = argc;
	this->argv = argv;
}

static const unsigned int RXTX_BUFFER_SIZE = 720*1024;
unsigned char xBuffer[RXTX_BUFFER_SIZE];
unsigned char yBuffer[RXTX_BUFFER_SIZE];
char * cBuffer = (char *) &xBuffer[0];

static const unsigned int CMD_PRINT = 0xA1;
static const unsigned int CMD_VERIFY = 0xA2;
static const unsigned int CMD_WRITE = 0xA3;
static const unsigned int CMD_ERASE = 0xA4;

void NwhostService::debugout(const char *msg)
{
    Util::debug("[hwhost] %s", msg);
}

void NwhostService::waitForAck()
{
    bool firstByte = true;
    bool ackReceived = false;
    int lBytesReceived;
    
    while (!ackReceived) {
        lBytesReceived = mUsbStream->readExact(yBuffer, 1);
        if (lBytesReceived != 1) {
            Util::debug("Communication failed!\n");
            exit(1);
        }
        if (firstByte) {
chkAA:                    
            if (yBuffer[0] == 0xaa) {
                firstByte = false;
                continue;
            } 
        } else {
            if (yBuffer[0] != 0x55) {
                firstByte = true;
                goto chkAA;
            } else {
                ackReceived = true;
                continue;
            } 
        }
    }
    mUsbStream->readExact(yBuffer, 1);		// ack is followed by a byte        
    //Util::debug(" ACK received... %u\n", yBuffer[0]);
}

void NwhostService::start(FtdiDriverType aDriverType)
{
	delete mUsbStream;   //does nothing if mUsbStream == 0
	mUsbStream = ftdx::newUsbStream(aDriverType);
}

void NwhostService::updateFirmware(string sImageName) {

    unsigned int uiSectorSize = 128;
    unsigned int uiHeader = 7;
    unsigned int uiAmount = uiSectorSize+uiHeader;
    unsigned int uiAddress = 0;
    unsigned int uiWriteAdress = 0;
    unsigned int uiBank = 0;
    unsigned long uiBytesWritten = 0;
    unsigned int uiSector;
    
    char cString[250];

    start(eLibUsb);

	// this should not be needed, but without it, FTD2xx::read returns to fast!
	mUsbStream->setTimeouts(500000, 500000);

	mUsbStream->openBlocking();

    fstream *fs = new fstream(sImageName.c_str(), ios::binary | ios::in | ios::out);
    if (fs->fail()) {
        Util::debug("%s not found!\n", sImageName.c_str());
        exit(1);
    }
    // check for rom headers in all banks
    for (int i=0;i<32;i++) {
        fs->seekg(i*0x4000);
        fs->read(cBuffer, 2);

        if ((xBuffer[0] != 'A') || (xBuffer[1] != 'B')) {
            Util::debug("No rom header found in bank %u!\n", i);
            exit(1);
        }
    }
    
    Util::debug("Flashing %s to USB interface...\n", sImageName.c_str());
    Util::sleep(500);  // wait for buffer-flush to occur 
	
    fs->seekg(0, ios::end);
	unsigned int uiFileSize = fs->tellg();

    Util::debug("Send erase command to flash....\n");

    xBuffer[0] = 0xEE;
    xBuffer[1] = 0xBB;
    xBuffer[2] = 0x55;
    xBuffer[3] = CMD_ERASE;

    mUsbStream->write(xBuffer, 4, &uiBytesWritten);
    Util::debug("Waiting for erase to complete...\n");
    waitForAck();
    Util::debug("Erase complete!\n");

    Util::debug("Writing to flash....\n");
    unsigned int uiSectorCount = uiFileSize/uiSectorSize;
    unsigned int uiLastProgress = 1;
    unsigned int uiProgress = 0;
    
    for (uiSector=0; uiSector<uiSectorCount; uiSector++) {

            uiWriteAdress = uiAddress & 0x3fff;
            uiBank = uiSector/uiSectorSize;                  // there are 128 x 128 bytes in a bank
            xBuffer[0] = 0xEE;
            xBuffer[1] = 0xBB;
            xBuffer[2] = 0x55;
            xBuffer[3] = CMD_WRITE;
            xBuffer[4] = uiWriteAdress & 0xFF;
            xBuffer[5] = (uiWriteAdress >> 8) & 0xFF;
            xBuffer[6] = uiBank & 0xFF;
            
            uiProgress = (uiSector*100)/uiSectorCount;
            if (uiLastProgress != uiProgress)
            {
                uiLastProgress = uiProgress;          
                printf("Writing bank %u (%u%%)\r", uiBank, uiProgress);
				fflush(stdout);
            }
            
            fs->seekg(uiSector*uiSectorSize);
            fs->read(cBuffer+uiHeader, uiSectorSize);
 
            mUsbStream->write(xBuffer, uiAmount, &uiBytesWritten);
            uiAddress = uiAddress + uiSectorSize;

            if (uiBytesWritten != uiAmount) {
                Util::debug("error: sector %u not completely send...\n", uiSector);
                Util::debug("       BytesWritten: %u amount: %u\n", uiBytesWritten, uiAmount);
            }
            waitForAck();            
    }
    Util::debug("Write complete!\n");

    uiAddress = 0;
    unsigned int uiErrors = 0;
    uiAmount = uiHeader;
    for (uiSector=0; uiSector<uiSectorCount; uiSector++) {

            uiWriteAdress = (uiAddress & 0x3fff) + 0x4000;
            uiBank = uiSector/uiSectorSize;
            xBuffer[0] = 0xEE;
            xBuffer[1] = 0xBB;
            xBuffer[2] = 0x55;
            xBuffer[3] = CMD_VERIFY;
            xBuffer[4] = uiWriteAdress & 0xFF;
            xBuffer[5] = (uiWriteAdress >> 8) & 0xff;
            xBuffer[6] = uiBank & 0xFF;

            mUsbStream->write(xBuffer, uiAmount, &uiBytesWritten);
            
            fs->seekg(uiSector*uiSectorSize);
            fs->read(cBuffer+uiHeader, uiSectorSize);
 
			uiBytesWritten = mUsbStream->readExact(yBuffer, uiSectorSize);
            for (unsigned int i=0; i<uiSectorSize; i++) {
                if (xBuffer[i+uiHeader] != yBuffer[i]) {
                    Util::snprintf(cString, sizeof(cString), "\nVerify of address 0x%04X failed! Found: 0x%02X Expected: 0x%02X\n", (uiWriteAdress+i)+(uiBank*0x4000), yBuffer[i], xBuffer[i+uiHeader]);
                    uiErrors++;
                    if (uiErrors > 25) {
                        Util::debug("Found more than 25 error...!\n");
                        exit(1);
                    }
                    Util::debug(cString);
                }
            }
            uiAddress += 128;

            waitForAck();
            
            uiProgress = (uiSector*100)/uiSectorCount;
            if (uiLastProgress != uiProgress)
            {
                uiLastProgress = uiProgress;          
                printf("Verify progress %u%%\r", uiProgress);
				fflush(stdout);
            }
    }

    Util::debug("\nDone... %u sectors written and verified.\n", uiSector);
	mUsbStream->close(); 
}

/*
// called when data is available from the NowindHost to be send back to the msx
void NwhostService::read_data_available()
{
	unsigned int lBytesToSend = 0;
	unsigned long lBytesWritten = 0;
	lBytesToSend = nowindusb_readbuf(xBuffer);

	assert(lBytesToSend < RXTX_BUFFER_SIZE);
	if (lBytesToSend > 0) {
		mUsbStream->write(xBuffer, lBytesToSend, &lBytesWritten);
		if (lBytesToSend != lBytesWritten)
		{
			Util::debug("Failed to send all data, %i of %i bytes sent (mUsbStream lost?)\n", lBytesWritten, lBytesToSend);
			// the NowindHost will timeout and reset its state
			return;
		}
	}
	statStopMeasument(lBytesWritten);
}
*/

void NwhostService::purge_buffers()
{
	Util::debug("Internal buffers purged!\n");
	mUsbStream->purgeTx();
	mUsbStream->purgeRx();
}

#ifndef WIN32

#include <unistd.h>

void NwhostService::invokeHostImage()
{
    pid_t lPid = fork();
    if (lPid == 0)
    {
        //i'm a child 
        hostImage();
        return;
    }
    if (lPid == -1)
    {
        Util::debug("Could not create child process\n");
    }
}

#else
void NwhostService::invokeHostImage() {

    Util::debug("fork() is not supported on windows!\n");
}
#endif

void NwhostService::hostImage() {

    int lBytesReceived;

	// create the mUsbStream instance 
    start(eLibUsb);	

    //nowindusb_set_dataavailable_callback(&read_data_available);
   // nowindusb_set_purge_callback(&purge_buffers);

	mUsbStream->setTimeouts(100, 100);
	bool lConnected = false;
    for (;;) {
		
		if (!lConnected)
		{
            mUsbStream->openBlocking();
			purge_buffers();
			lConnected = true;
		}

        lBytesReceived = mUsbStream->readBlocking(xBuffer, 8192);
		if (lBytesReceived == 0)
		{
			// connection problem
			mUsbStream->close();
			lConnected = false;
			continue;
		}

		statStartMeasument();

        // copy the buffer into the nowindhost
        for (int i=0;i<lBytesReceived;i++) {
			nowindusb_write(xBuffer[i]);
        }

		buffer.clear();
		while (nowindusb_is_data_available())
		{
			buffer.push_back(nowindusb_read());
		}
		unsigned long lBytesWritten = 0;
		if (buffer.size() > 0)
		{
			mUsbStream->write(&buffer[0], buffer.size(), &lBytesWritten);
		}
		if (lBytesWritten != buffer.size())
		{
			//TODO: handle error..
			Util::debug("Error: lBytesWritten != buffer.size()!\n");
		}

    }
}

void NwhostService::statStartMeasument()
{
		if (mTransferredBytes > 1024*1024) { 
			mTransferredBytes = 0;
			mTotalTime = 0;
		}
		mStartTime = Util::getTicks();
}

void NwhostService::statStopMeasument(unsigned int aBytes)
{
    unsigned long lNow = Util::getTicks();
	unsigned long lElapsed = lNow-mStartTime;
	
    mTotalTime += lElapsed;
	if (mTotalTime == 0) mTotalTime = 1; // prevent division by 0
    
	mTransferredBytes += aBytes;
	unsigned long lKbps = ((mTransferredBytes * 1000) / mTotalTime)/1024;		//*1000 ms->seconds /1024 bytes->kbytes
	Util::debug("sending response of %u bytes. (%u Kbps)\n", aBytes, lKbps);
}

void NwhostService::diskToRom(string imageName) {

    fstream *fsDisk = new fstream(imageName.c_str(), ios::binary | ios::in);
    if (fsDisk->fail()) {
        Util::debug("File %s not found!\n", imageName.c_str());
        exit(1);
    }

    // TODO: check 360 kB
        
    fstream *fsOut = new fstream("romdisk.bin", ios::binary | ios::out);
    if (fsDisk->fail()) {
        Util::debug("Unable to create romdisk.bin!\n");
        exit(1);
    }
    char *buffer = new char[0x4000];
    char *bank31 = new char[0x4000];

    memset(buffer, 0xff, 0x4000);
    memset(bank31, 0xff, 0x4000);
       
    fsDisk->seekg(0);
    fsOut->seekg(0);
    
    for (int i=0;i<23;i++) {
        fsDisk->read(bank31 + (i*512) + 0x100, 512);   // first sector
        fsDisk->read(buffer + 0x100, 31*512);
        fsOut->write(buffer, 0x4000);
    }
    Util::snprintf(bank31 + 0x103, sizeof(bank31), "ROMDRIVE");
    fsOut->write(bank31, 0x4000);
    fsOut->close();
    fsDisk->close();
    Util::debug("Image converted to romdisk.bin!\n");
}

void NwhostService::testModeDev()
{
#ifndef WIN32
	system("stty -F /dev/ttyUSB0 115200 cs8 -cstopb -parity -icanon min 1 time 1");
	int lHandle = open("/dev/ttyUSB0", O_RDWR);
	
	//write(lHandle, command, 2);

	char lBuffer[2048];
	for (;;) {
		int lRead = read(lHandle, lBuffer, 1);
		lBuffer[1] = 0;
		Util::debug("read: %s (%i byte)\n", lBuffer, lRead);
	}
	close(lHandle);
#endif
}

void NwhostService::testMode(string aArgument)
{
	unsigned char testString[200];
    Util::snprintf((char *) testString, sizeof(testString), "HELLO MSX 123456");

	if (aArgument.compare("dev") == 0)
	{
		testModeDev();
		return;
	}

	testString[12] = 0xff;
	testString[13] = 0x00;
	testString[14] = 0xAA;
	testString[15] = 0x55;

	/*
	char * testString = "HELLO MSX 123456";
	testString[12] = 0xff;
	testString[13] = 0x00;
	testString[14] = 0xAA;
	testString[15] = 0x55;
	*/

    start(eLibUsb);
	mUsbStream->openBlocking();
	mUsbStream->reset();

	if (aArgument.compare("raw") == 0)
	{
		Util::debug("Running in TESTMODE, waiting for MSX to send me data and outputing in raw mode\n");

		unsigned char lBuffer[2048];
		for (;;) {
			int received = mUsbStream->readBlocking(lBuffer, 2048);
			lBuffer[received] = 0;
			Util::debug("%s", lBuffer);
		}
	} else if (aArgument.compare("read") == 0)
	{
		Util::debug("Running in TESTMODE, waiting for MSX to send me data\n");

		unsigned char lBuffer[2048];
		for (;;) {
			int received = mUsbStream->readBlocking(lBuffer, 2048);
			lBuffer[received] = 0;
			Util::debug("received: %s\n", lBuffer);
		}
	}
	else
	{
		Util::debug("Running in TESTMODE, sending 'HELLO MSX 12' FF 00 AA 55 repeatedly\n", testString);
		unsigned long uiBytesWritten;
		for (;;) {
			mUsbStream->write((unsigned char *) testString, 16, &uiBytesWritten);
			Util::debug("SEND: %s\n", testString);
		}
	}

}

void NwhostService::processExit()
{
	if (mDebug) Util::debug("process closing down...");
	if (mUsbStream != 0) mUsbStream->close();

	//delete the object here, because the NwhostService destructor will not run if ctrl-c is pressed
	delete mUsbStream;		
	nowindusb_cleanup();
	if (mDebug) Util::debug("done.\n");
}

// these method will need to be reworking to cope with multiple connections
void NwhostService::setImage(int aDriveNr, string aFilename)
{
	nowindusb_set_image(aDriveNr, aFilename.c_str());
}

unsigned int NwhostService::setHarddiskImage(unsigned int aDriveNr, int aPartitionNr, bool aIgnoreBootflag, const char* aFilename)
{
	return nowindusb_set_harddisk_image(aDriveNr, aPartitionNr, aIgnoreBootflag, aFilename);
}

void NwhostService::setRomdisk(int aDriveNr)
{
	nowindusb_set_romdisk(aDriveNr);
}

void NwhostService::setAttribute(nw_attribute aAttribute, bool aValue)
{
	nowindusb_attribute(aAttribute, aValue);
}
