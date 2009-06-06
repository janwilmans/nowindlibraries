//! NwhostService.h
#ifndef NWHOSTSERVICE_H
#define NWHOSTSERVICE_H

#include <string>
#include <vector>
using namespace std;

#include "NowindExports.h"

#include <libftdx.h>
using namespace ftdx;

namespace nowind {

class NwhostService {

public:
		NwhostService();
		~NwhostService();
		void setParameters(int argc, char **argv);
		int execute();
		
        static void initialize();
		void start();
		void waitForAck();
		void updateFirmware(string sImageName);
		void hostImage();
		void testMode(string);
		void testModeDev();
		void diskToRom(string imageName);
		//static void read_data_available();
		static void purge_buffers();
		static void debugout(const char *msg);

		static void statStartMeasument();
		static void statStopMeasument(unsigned int);
		static void processExit();
	
private:
		int argc;
		char **argv;
		static UsbStream * mUsbStream;

		vector<unsigned char> buffer;

		// statistics
		static unsigned long mStartTime;
		static unsigned long mTotalTime;
		static unsigned long mTransferredBytes;
		static bool mDebug;
};

} // namespace nowind

#endif // NWHOSTSERVICE_H
