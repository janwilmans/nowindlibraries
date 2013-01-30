//! NowindInterface.h
#ifndef NOWINDINTERFACE_H
#define NOWINDINTERFACE_H

#include "msxtypes.h"
#include "MemoryDevice.h"
#include "DiskDrive.h"

class NowindInterface : public MemoryDevice {
private:
			unsigned int switchedBlock;
			unsigned int switchedBlockMask;
			
			DiskDrive    *disk;
            int numberOfLatencyReads;

public:
			NowindInterface(std::string filename);
			void insertDisk(int driveNr, std::string filename);
			void insertHarddisk(int, std::string filename, int);
			
			bool isEnabled(nw_word address);
			
            nw_byte read(nw_word address);
            virtual void write(nw_word address, nw_byte value);
            virtual void install(layoutType layout, Uint8 mainSlot, Uint8 subSlot, nw_word address);
            virtual void activate(unsigned int block);
            virtual ~NowindInterface();
            
            static void debugout(const char *msg);
};

#endif 


