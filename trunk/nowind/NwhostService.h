//! NwhostService.h
#ifndef NWHOSTSERVICE_H
#define NWHOSTSERVICE_H

#ifdef _MSC_VER
//# pragma warning( disable: 4251 )
#endif 

#include <string>
#include <vector>
using namespace std;

#include "nowind.h"
#include "NowindExports.h"

#include <libftdx.h>
#include <libnwhost.h>

namespace nowind {

class NOWIND_API NwhostService {

public:
		NwhostService();
		~NwhostService();
		void setParameters(int argc, char **argv);
		int execute();
		
        static void initialize();
        void start(ftdx::FtdiDriverType aDriverType);
		void waitForAck();
        void updateFirmware(std::string sImageName, int iMethodVersion, bool bVerify); 
		void hostImage();
        void setRunningFalse();
		void testMode(string);
		void testModeDev();
		void diskToRom(string imageName);
		
		// todo: maybe create a AttributeType and VarType class
        void setAttribute(nwhost::nw_attribute aAttribute, bool aValue);
		
		void setImage(int aDriveNr, string aFilename);
		unsigned int setHarddiskImage(unsigned int aDriveNr, int aPartitionNr, bool aIgnoreBootflag, const char* aFilename);
		void setRomdisk(int aDriveNr);

        void addStartupRequest(const nwhost::byte* cRequest);

		//static void read_data_available();
		static void purge_buffers();
		static void debugout(const char *msg);

		static void statStartMeasument();
		static void statStopMeasument(unsigned int);
		static void processExit();
		
	
private:
		int argc;
		char **argv;
        static ftdx::UsbStream* mUsbStream;

		// statistics
		static unsigned long mStartTime;
		static unsigned long mTotalTime;
		static unsigned long mTransferredBytes;
		static bool mDebug;
        bool mRunning;
};

} // namespace nowind

#endif // NWHOSTSERVICE_H
