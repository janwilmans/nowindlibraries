//! HostApp.h
#ifndef HOSTAPP_H
#define HOSTAPP_H

#include <string>
#include <vector>
using namespace std;

#include <libnowind.h>

using namespace nowind;

class Connection;
class HostApp {

public:
		HostApp();
		~HostApp();
		void setParameters(int argc, char **argv);
		int execute();
		
		static void initialize();
		void waitForAck();
		void hostImage();
		void testMode(string);
		void testModeDev();
		void diskToRom(string imageName);
		static void debugout(const char *msg);

		static void statStartMeasument();
		static void statStopMeasument(unsigned int);
		static void processExit();
	
private:
		int argc;
		char **argv;

		vector<unsigned char> buffer;

		// statistics
		static unsigned long mStartTime;
		static unsigned long mTotalTime;
		static unsigned long mTransferredBytes;
		static bool mDebug;
		static HostApp* mHostApp;
		NwhostService* mHostService;
};

#endif // HOSTAPP_H
