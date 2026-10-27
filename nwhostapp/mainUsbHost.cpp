#include "HostApp.h"
#include <cstdio>
#include <cstdlib>
#include <exception>

int main(int argc, char* argv[])
{
    HostApp::initialize();

    HostApp app;
    app.setParameters(argc, argv);
    atexit(&HostApp::processExit);

    int exitcode = 0;
    try
    {
        exitcode = app.execute();
    }
    catch (const std::exception& exception)
    {
        // NwhostService throws where it used to call exit(1)
        std::fprintf(stderr, "Error: %s\n", exception.what());
        exitcode = 1;
    }

    HostApp::processNormalExit();

    // app. normal exit
    return exitcode;
}
