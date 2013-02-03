#include "HostApp.h"
#include <cstdlib>

int main(int argc, char *argv[])
{
    HostApp::initialize();

	HostApp app;
	app.setParameters(argc, argv);
	atexit(&HostApp::processExit); 
	int exitcode = app.execute();

	HostApp::processNormalExit();
	
	// app. normal exit
	return exitcode;
}

