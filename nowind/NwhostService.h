//! NwhostService.h
#ifndef NWHOSTSERVICE_H
#define NWHOSTSERVICE_H

#ifdef _MSC_VER
//# pragma warning( disable: 4251 )
#endif

#include <atomic>
#include <functional>
#include <string>
#include <vector>
using namespace std;

#include "nowind.h"
#include "NowindExports.h"

#include <libftdx.h>
#include <libnwhost.h>

namespace nowind {

enum class ConnectionState {
    WaitingForInterface, // no interface found (yet), hostImage() keeps trying
    Connected
};

class NOWIND_API NwhostService {
public:
    NwhostService();
    ~NwhostService();
    void setParameters(int argc, char** argv);
    int execute();

    static void initialize();
    // The functions below throw std::runtime_error when execution cannot continue (previously exit()), so a GUI can
    // report the error; the command line host catches it at the top level.
    void startDriver();
    void startDriver(ftdx::FtdiDriverType aDriverType);
    void waitForAck();
    void updateFirmware(std::string sImageName, int iMethodVersion, bool bVerify);
    void hostImage();

    // Thread-safe; may be called before hostImage() starts, in which case hostImage() returns immediately.
    void setRunningFalse();

    // Called on the thread running hostImage(); set it before hostImage() starts.
    void setConnectionCallback(std::function<void(ConnectionState)> callback);

    // Bytes received from and sent to the MSX since this service was created; thread-safe.
    [[nodiscard]] unsigned long long totalBytesTransferred() const;
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
    static void debugout(const char* msg);

    static void statStartMeasument();
    static void statStopMeasument(unsigned int);
    static void processExit();


private:
    // returns false if a stop was requested before the interface was connected
    bool openUnlessStopped();

    int argc = 0;
    char** argv = nullptr;
    static ftdx::UsbStream* mUsbStream;

    // statistics
    static unsigned long mStartTime;
    static unsigned long mTotalTime;
    static unsigned long mTransferredBytes;
    static bool mDebug;
    std::atomic<bool> mRunning{true}; // "not stopped", so a stop requested before hostImage() is not lost
    std::atomic<unsigned long long> mTotalBytesTransferred{0};
    std::function<void(ConnectionState)> mConnectionCallback;
};

} // namespace nowind

#endif // NWHOSTSERVICE_H
