//! WD279X.h
#ifndef WD279X_H
#define WD279X_H

#include "msxtypes.h"
#include "MemoryDevice.h"
#include "DiskDrive.h"

// status register for type I commands
#define BUSY 0x01
#define INDEX 0x02
#define TRACK0 0x04
#define CRC_ERROR 0x08
#define SEEK_ERROR 0x10
#define HEAD_LOADED 0x20
#define WRITE_PROTECT 0x40
#define NOT_READY 0x80


// status register for type II & III commands
#define LOST_DATA 0x04
#define RECORD_NOT_FOUND 0x10
#define RECORD_TYPE 0x20
#define WRITE_PROTECT 0x40

// command register flags
#define HEAD_LOAD_FLAG 0x08
#define MULTIPLE_RECORD_FLAG 0x10


// interruptRegister
#define DATA_REQUEST 0x40
#define NOT_BUSY 0x80


class WD279X : public MemoryDevice {
private:
			DiskDrive    *disk;
			
			nw_byte trackRegister;
			nw_byte statusRegister;
			nw_byte dataRegister;
			nw_byte sectorRegister;
			nw_byte commandRegister;
            nw_byte diskSide;			

            bool dataRequest;
            bool intRequest;
			
			int currentSector;
			int currentTrack;
			
			int transferCounter;
			
			int seekDirection;
			nw_byte sectorBuffer[512];
			
            nw_byte readDataRegister(void);
            void writeDataRegister(nw_byte);
			void statusCommandTypeI(void);
            
            nw_byte getStatusRegister(void);
            void commandReadSector(void);
            void commandWriteSector(void);

public:
			WD279X(std::string filename);
			void insertDisk(std::string filename);
			
			bool isEnabled(nw_word address);
			
            nw_byte read(nw_word address);
            virtual void write(nw_word address, nw_byte value);
            virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
            virtual void activate(unsigned int block);
            virtual ~WD279X();
};

#endif


