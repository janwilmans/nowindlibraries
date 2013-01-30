#include <windows.h>

using namespace std;

#include <msxtypes.h>

#include "devices/FT245.h"
#include "devices/FT245UsbHost.h"
#include "DiskDrive.h"
#include "Debug.h"
#include "FTD2XX.H"

#define main no_main_please
    #include "SDL/SDL.h"
#undef main

// globals 
FT_STATUS ftStatus;

static const unsigned int RXTX_BUFFER_SIZE = 720*1024;
unsigned char xBuffer[RXTX_BUFFER_SIZE];
unsigned char yBuffer[RXTX_BUFFER_SIZE];
char * cBuffer = (char *) &xBuffer[0];

static const unsigned int CMD_VERIFY = 0xA2;
static const unsigned int CMD_WRITE = 0xA3;
static const unsigned int CMD_ERASE = 0xA4;

void waitForAck(FT_HANDLE ftHandle)
{
            bool firstByte = true;
            bool ackReceived = false;
            FT_STATUS ftStatus;
            DWORD BytesReceived;
            
            while (!ackReceived) {
                ftStatus = FT_Read(ftHandle, yBuffer, 1, &BytesReceived);
                if (BytesReceived != 1) {
                    cerr << "Communication failed!\n";
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
            ftStatus = FT_Read(ftHandle, yBuffer, 1, &BytesReceived);            
//            cerr << " ACK received..." << ((unsigned int) yBuffer[0]) << "\n";
}

void waitForDebugAck(FT_HANDLE ftHandle)
{
            bool firstByte = true;
            bool ackReceived = false;
            FT_STATUS ftStatus;
            DWORD BytesReceived;
            
            while (!ackReceived) {
                ftStatus = FT_Read(ftHandle, yBuffer, 1, &BytesReceived);
                if (BytesReceived != 1) {
                    cerr << "Communication failed!\n";
                    exit(1);
                }
                if (firstByte) {
chkAA:                    
                    if (yBuffer[0] == 0xaa) {
                        firstByte = false;
                        continue;
                    } 
                } else {
                    if (yBuffer[0] != 0x51) {
                        firstByte = true;
                        goto chkAA;
                    } else {
                        ackReceived = true;
                        continue;
                    } 
                }
            }
            ftStatus = FT_Read(ftHandle, yBuffer, 3, &BytesReceived);            
            cerr << " DACK received...\n";
            cerr << "D0: " << ((unsigned int) yBuffer[0]) << "\n";
            cerr << "D1: " << ((unsigned int) yBuffer[1]) << "\n";
            cerr << "D2: " << ((unsigned int) yBuffer[2]) << "\n";

}

FT_HANDLE connect()
{
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;
    DBERR("Connecting to Nowind Interface...\n");
/*
    DWORD numDevs;
    ftStatus = FT_ListDevices(&numDevs,NULL,FT_LIST_NUMBER_ONLY);
    if (ftStatus == FT_OK) {
        // FT_ListDevices OK, number of devices connected is in numDevs
    }
*/
 
    /* Just try to connect to the first available device */  
    bool uiRetry = false;
    while (FT_Open(0, &ftHandle) != FT_OK) {
        if (uiRetry) {
            DBERR("Waiting for response...\n");
            uiRetry = true;
        }
        Sleep(250);
    }
    if (uiRetry) {
        DBERR("Nowind Interface found!\n");
    }    

    /* clear all buffers */
    FT_Purge(ftHandle, FT_PURGE_RX  | FT_PURGE_TX);
    
    /* set the buffer-flush-timeout */
    ftStatus = FT_SetLatencyTimer(ftHandle, 2);       // set to 10ms ?
    if (ftStatus != FT_OK) {
        DBERR("FT_SetLatencyTimer failed!\n");
    }
//    ftStatus = FT_SetUSBParameters(ftHandle, 640, 640);
//    if (ftStatus != FT_OK) {
//        DBERR("FT_SetUSBParameters failed!\n");
//    }
    
    
    return ftHandle;
}        

void speedTest(void) {

    FT_HANDLE ftHandle = connect();
    
    memset(xBuffer, 0, 128);

    DWORD dwBytesWritten = 0;
    
    while (1)
    {
        DWORD dwTimeNow = SDL_GetTicks();
        DWORD dwPartWritten = 0;
        dwBytesWritten = 0;

        for (unsigned int i=0;i<10;i++)
        {    
            ftStatus = FT_Write(ftHandle, xBuffer, 4100, &dwPartWritten);
            dwBytesWritten += dwPartWritten;
        }
        // transferred x bytes in y ms
        DWORD dwTotalTime = SDL_GetTicks()-dwTimeNow;
        
        if (dwTotalTime != 0) {
            DBERR("%u bytes in %u ms = %u KBPS\n", dwBytesWritten, dwTotalTime, (dwBytesWritten*1000/dwTotalTime)/1024);
        }    
    }
}

void updateFirmware(string sImageName) {

    unsigned int uiSectorSize = 128;
    unsigned int uiHeader = 7;
    unsigned int uiAmount = uiSectorSize+uiHeader;
    unsigned int uiAddress = 0;
    unsigned int uiWriteAdress = 0;
    unsigned int uiBank = 0;
    DWORD uiBytesWritten = 0;
    unsigned int uiSector;
    
    char cString[250];

    FT_HANDLE ftHandle = connect();

    fstream *fs = new fstream(sImageName.c_str(), ios::binary | ios::in | ios::out);
    if (fs->fail()) {
        cerr << sImageName << " not found!\n";
        exit(1);
    } else {
        cerr << "Flashing " << sImageName << " to USB interface...\n";
    }

	fs->seekg(0, ios::end);
	unsigned int uiFileSize = fs->tellg();

    cerr << "Send erase command to flash.... ";

    xBuffer[0] = 0xEE;
    xBuffer[1] = 0xBB;
    xBuffer[2] = 0x55;
    xBuffer[3] = CMD_ERASE;
    ftStatus = FT_Write(ftHandle, xBuffer, 4, &uiBytesWritten);

    cerr << "waiting for ack after erase.... ";
    waitForAck(ftHandle);

    cerr << "Writing to flash....\n";
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
            xBuffer[5] = uiWriteAdress >> 8;
            xBuffer[6] = uiBank;
            
            uiProgress = (uiSector*100)/uiSectorCount;
            if (uiLastProgress != uiProgress)
            {
                uiLastProgress = uiProgress;          
                printf("Writing to bank %u at address 0x%04X, progress %2u%%\r", uiBank, uiWriteAdress, uiProgress);
            }
            
            fs->seekg(uiSector*uiSectorSize);
            fs->read(cBuffer+uiHeader, uiSectorSize);
 
            ftStatus = FT_Write(ftHandle, xBuffer, uiAmount, &uiBytesWritten);
            uiAddress = uiAddress + uiSectorSize;

            if (uiBytesWritten != uiAmount) {
                cerr << "error: sector " << uiSector << " not completely send..." << endl;
                cerr << "       BytesWritten: " << uiBytesWritten << " amount: " << uiAmount << endl;
            }
            waitForAck(ftHandle);            
    }

    //cerr << "Done, but not verified!\n";
    //exit(1);
    
    cerr << "\nVerifing " << dec << uiSector << " sectors.\n";

    FT_Purge(ftHandle, FT_PURGE_RX  | FT_PURGE_TX);     // todo: why is this needed?

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
            xBuffer[5] = uiWriteAdress >> 8;
            xBuffer[6] = uiBank;

            ftStatus = FT_Write(ftHandle, xBuffer, uiAmount, &uiBytesWritten);
            
            fs->seekg(uiSector*uiSectorSize);
            fs->read(cBuffer+uiHeader, uiSectorSize);
 
            ftStatus = FT_Read(ftHandle, yBuffer, uiSectorSize, &uiBytesWritten);
            for (unsigned int i=0; i<uiSectorSize; i++) {
                if (xBuffer[i+uiHeader] != yBuffer[i]) {
                    sprintf(cString, "\nVerify of address 0x%04X failed: 0x%04X != 0x%04X\n", (uiWriteAdress+i)+(uiBank*0x4000), xBuffer[i+uiHeader], yBuffer[i]);
                    uiErrors++;
                    if (uiErrors > 10) exit(1);
                    cerr << cString;
                }
            }
            uiAddress += 128;

            waitForAck(ftHandle);
            
            uiProgress = (uiSector*100)/uiSectorCount;
            if (uiLastProgress != uiProgress)
            {
                uiLastProgress = uiProgress;          
                printf("Verify progress %2u%%\r", uiProgress);
            }
    }

    cerr << "\nDone... " << uiSector << " sectors written and verified.\n";
    FT_Close(ftHandle);    
}

void hostImage(string sImageName) {

    DWORD dwRxBytes;
    DWORD dwBytesReceived;
    DWORD dwBytesWritten;

    FT_HANDLE ftHandle = connect();
    DiskDrive * disk = new DiskDrive();    
    FT245UsbHost * ft245UsbHost = new FT245UsbHost(disk);
    FT245 * ft245 = new FT245(ft245UsbHost);
    
    /* assosiate ft245UsbHost <-> ft245 */
    ft245UsbHost->setFt245(ft245);
    assert(disk->loadDiskImage(sImageName));

    while (1) {

        ftStatus = FT_Read(ftHandle, xBuffer, 1, &dwBytesReceived);
        /* A return value of FT_IO_ERROR suggests an error in the parameters of the function, or a fatal error like USB disconnect has occurred. */
        
        if (ftStatus != FT_OK) {
            DBERR("Read error!, check cable!\n");
        	FT_Close(ftHandle);
        	bool closed = true;
        
        	while (closed) {
        		Sleep(250);
        		ftStatus = FT_Open(0, &ftHandle);
        	    if (ftStatus == FT_OK) {
        	        // device open
        	        DBERR("Found Nowind Interface!\n");
        			closed = false;
        	    }
        	}
        	continue;
        }
        assert(dwBytesReceived > 0);
    
        // copy the first byte
//      DBERR("pass first byte 0x%02X to fifo\n", xBuffer[0]);
        ft245->sendWrite(xBuffer[0]); 
        
        /* how much is in the receive queue? */
        FT_GetQueueStatus(ftHandle, &dwRxBytes);
        
        /* read that much from the device */
        ftStatus = FT_Read(ftHandle, xBuffer, dwRxBytes, &dwBytesReceived);
        assert(ftStatus == FT_OK);
//      DBERR("received %u of %u bytes from msx\n", dwBytesReceived, dwRxBytes);
        
        // copy the buffer into the host-class
        for (unsigned int i=0;i<dwBytesReceived;i++) {
//          if (dwBytesReceived < 40) DBERR("  pass 0x%02X to fifo\n", xBuffer[i]);
            ft245->sendWrite(xBuffer[i]);   
       }
       
        unsigned int dwBytesToSend = 0;
    
        /* prefixing the response with a dummy-byte is essential
         * to prevent loss of data, this is a requirement of our
         * hardware design.
         *
         * When it's receive buffer is empty, a read from the 
         * msx is done and during that read data becomes available
         * the byte is "read" from the receivebuffer, but the msx
         * has already sampled the data-bus; so the byte is lost.
         */ 
        xBuffer[dwBytesToSend++] = 0xf1;
        while (!ft245->receiveQueueEmpty()) { 
            xBuffer[dwBytesToSend] = ft245->receiveRead();
           // DBERR("send byte 0x%02X to msx\n", xBuffer[dwBytesToSend]);
            dwBytesToSend++;
            assert(dwBytesToSend < RXTX_BUFFER_SIZE);
        }
        DWORD dwTimeNow = SDL_GetTicks();
        
        ftStatus = FT_Write(ftHandle, xBuffer, dwBytesToSend, &dwBytesWritten);
        //DBERR("sending response of %u bytes.\n", dwBytesToSend);
        if (ftStatus != FT_OK) {
    		  assert(false);
        }
        assert(dwBytesToSend == dwBytesWritten);
        
        // transferred x bytes in y ms
        DWORD dwTotalTime = SDL_GetTicks()-dwTimeNow;
        
        if (dwTotalTime != 0) {
            DBERR("%u bytes in %u ms = %u KBPS\n", dwBytesWritten, dwTotalTime, (dwBytesWritten*1000/dwTotalTime)/1024);
        }
    }
}

void diskImage2rom() {

    fstream *fsRom = new fstream("roms\\disk.rom", ios::binary | ios::in);
    if (fsRom->fail()) {
        cerr << "File not found!\n";
        exit(1);
    }
    fstream *fsDisk = new fstream("image.dsk", ios::binary | ios::in);
    if (fsDisk->fail()) {
        cerr << "File not found!\n";
        exit(1);
    }
    fstream *fsOut = new fstream("romwithdisk.bin", ios::binary | ios::out);
    if (fsOut->fail()) {
        cerr << "Unable to create file!\n";
        exit(1);
    }
    char buf[128 * 1024];
    fsRom->seekg(0);
    fsRom->read(buf, 0x4000);
    fsRom->close();
    
    fsOut->write(buf, 0x4000);
    for (int j=0;j<0x20000;j++) buf[j] = 0xff;
    fsOut->write(buf, 0x20000-0x4000);
    
    fsOut->seekg(0x20000);
    fsDisk->seekg(0);
    // TODO: reads too much...
    char lastBank[0x4000];
    memset(lastBank, 0x44, 0x4000);
    
    for (int i=0;i<23;i++) {
        fsDisk->read(lastBank + i*512, 512);    // TODO: ROM header
        fsOut->write(lastBank + i*512, 512);
        fsDisk->read(buf, 31 * 512);
        fsOut->write(buf, 31 * 512);
    }
    // fill last bank with the skipped sectors
    for (int j=0;j<512;j++) buf[j] = 0x22;
    fsOut->write(buf, 512);                     // TODO: ROM header
    fsOut->write(lastBank, 0x4000 - 512);
    fsDisk->close();
    fsOut->close();
}

int main(int argc, char *argv[])
{
    Debug::Instance()->initialize();
    cerr << "Nowind Interface USB HOST v2.0\n";

    string sImageName = "";
    bool bFirmwareUpdate = false;

    string sArgument = "";
	if (argc > 1) 
    {
        sArgument = string(argv[1]);
        switch (sArgument[1])
        {
        case 'f':   
        case 'F': 
            bFirmwareUpdate = true;  
            sImageName = string(argv[2]);
            break;
        case 'h':   
        case 'H': 
            sImageName = string(argv[2]);
            break;
        case 't':   
        case 'T': 
            speedTest();
            exit(0);
            break;
        case 'r':
        case 'R': 
            diskImage2rom();
            exit(0);
            break;

        default:
            break;
        }
        
    }
    else
    {
        printf("Syntax: usbhost </h> </f> </i> <filename>\n");
        printf(" /h host a disk image\n");
//      printf(" /i info\n");
        printf(" /f send firmware update to interface\n");
        printf("Examples: usbhost /h image.dsk\n");
        printf("          usbhost /h harddiskimage.dsk\n");
        printf("          usbhost /f firmware.bin\n");
        printf("          usbhost /t speed test\n");
//      printf("          usbhost /i\n");
        exit(1);
    }
    assert(sArgument != ""); 
    
    if (bFirmwareUpdate)
    {
        printf("Firmware update for Nowind interface: %s\n", sImageName.c_str());
        updateFirmware(sImageName);
    }
    else
    {
        printf("Hosting image for Nowind interface: %s\n", sImageName.c_str());
        hostImage(sImageName);
    }
    return 0;
}    
