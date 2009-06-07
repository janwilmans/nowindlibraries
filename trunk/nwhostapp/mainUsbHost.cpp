#include "HostApp.h"
#include <cstdlib>

int main(int argc, char *argv[])
{
    HostApp::initialize();

	HostApp app;
	app.setParameters(argc, argv);
	atexit(&HostApp::processExit); 
	return app.execute();
}
