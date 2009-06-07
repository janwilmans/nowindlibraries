//! NwhostService.h
#ifndef NWHOSTSERVICE_H
#define NWHOSTSERVICE_H

#include <string>
#include <vector>
using namespace std;

#include "nowind.h"
#include "NowindExports.h"

#include <libftdx.h>
#include <libnwhost.h>

#ifdef WIN32
# pragma warning( disable: 4251 )
#endif 

using namespace ftdx;
using namespace nwhost;

namespace nowind {

class NOWIND_API NwhostService {

public:
		NwhostService();
		~NwhostService();
		void setParameters(int argc, char **argv);
		int execute();
		
        static void initialize();
		void start(FtdiDriverType aDriverType);
		void waitForAck();
		void updateFirmware(string sImageName);
		void hostImage();
		void testMode(string);
		void testModeDev();
		void diskToRom(string imageName);
		
		// todo: maybe create a AttributeType and VarType class
		void setAttribute(nw_attribute aAttribute, bool aValue);
		
		void setImage(int aDriveNr, string aFilename);
		unsigned int setHarddiskImage(unsigned int aDriveNr, int aPartitionNr, bool aIgnoreBootflag, const char* aFilename);
		void setRomdisk(int aDriveNr);
				
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
