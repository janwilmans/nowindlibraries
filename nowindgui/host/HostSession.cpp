#include "HostSession.h"

#include <NwhostService.h>
#include <libgeneral.h>
#include <libnwhost.h>

#include <exception>
#include <utility>

namespace nowindgui {

namespace {

// Everything the libraries log goes through Util::debug(), possibly from the worker thread. The sink is a plain
// function pointer, so the buffer is process-wide (one session per process anyway).
class LogBuffer {
public:
    void append(const char* message)
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        for (const char* character = message; *character != '\0'; ++character)
        {
            if (*character == '\r' || *character == '\n')
            {
                // progress output rewrites the line with '\r', show each state as a line of its own
                if (!mPending.empty())
                {
                    mLines.push_back(std::exchange(mPending, std::string()));
                }
                continue;
            }
            mPending.push_back(*character);
        }
    }

    [[nodiscard]] std::vector<std::string> drain()
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        return std::exchange(mLines, std::vector<std::string>());
    }

private:
    std::mutex mMutex;
    std::vector<std::string> mLines;
    std::string mPending;
};

LogBuffer& logBuffer()
{
    static LogBuffer buffer; // Meyers singleton, see above
    return buffer;
}

void logSink(const char* message)
{
    logBuffer().append(message);
}

// LL = 1, CB = 1 (command of 1 byte: 1), same as nwhostapp: the MSX waits until the host is ready
constexpr nwhost::byte requestWait[2] = {1, 1};

void configure(nowind::NwhostService& service, const HostConfiguration& configuration)
{
    service.setAttribute(nwhost::enable_phantom_drives, configuration.phantomDrives);
    service.setAttribute(nwhost::allow_other_diskroms, configuration.allowOtherDiskRoms);
    service.setAttribute(nwhost::enable_dos2, configuration.preferDos2);
    service.addStartupRequest(requestWait);

    unsigned int driveNumber = 0;
    for (const DriveEntry& drive : configuration.drives)
    {
        switch (drive.kind)
        {
        case DriveKind::FloppyImage:
            service.setImage(static_cast<int>(driveNumber), drive.path);
            ++driveNumber;
            break;
        case DriveKind::HarddiskImage:
            // all partitions, the bootable one first, like 'nwhostapp -m file'
            driveNumber += service.setHarddiskImage(driveNumber, 0xff, false, drive.path.c_str());
            break;
        case DriveKind::RomDisk:
            service.setRomdisk(static_cast<int>(driveNumber));
            ++driveNumber;
            break;
        }
    }
}

} // namespace

HostSession::HostSession()
{
    general::Util::setDebugSink(&logSink);
}

HostSession::~HostSession()
{
    stop();
    general::Util::setDebugSink(nullptr);
}

void HostSession::start(const HostConfiguration& configuration)
{
    if (isActive())
    {
        return;
    }
    stop(); // joins the worker of a session that failed

    {
        const std::lock_guard<std::mutex> lock(mErrorMutex);
        mLastError.clear();
    }

    // a fresh protocol engine, the previous session's drives must not be served again
    nowindusb_cleanup();
    nowindusb_startup();

    mService = std::make_unique<nowind::NwhostService>();
    mService->setConnectionCallback([this](nowind::ConnectionState connectionState) {
        mState = (connectionState == nowind::ConnectionState::Connected) ? SessionState::Hosting : SessionState::WaitingForInterface;
    });

    mState = SessionState::WaitingForInterface;
    mWorker = std::thread([this, configuration] { run(configuration); });
}

void HostSession::run(const HostConfiguration& configuration)
{
    try
    {
        configure(*mService, configuration);
        mService->startDriver();
        mService->hostImage();
        mState = SessionState::Stopped;
    }
    catch (const std::exception& exception)
    {
        // the libraries throw where they used to call exit(), see NwhostService.h
        {
            const std::lock_guard<std::mutex> lock(mErrorMutex);
            mLastError = exception.what();
        }
        mState = SessionState::Failed;
    }
}

void HostSession::stop()
{
    if (mService)
    {
        mService->setRunningFalse();
    }
    if (mWorker.joinable())
    {
        mWorker.join();
    }
    if (mService)
    {
        mFinalTransferredBytes = mService->totalBytesTransferred();
        mService.reset();
    }
    if (mState != SessionState::Failed)
    {
        mState = SessionState::Stopped;
    }
}

SessionState HostSession::state() const
{
    return mState;
}

bool HostSession::isActive() const
{
    const SessionState current = mState;
    return mWorker.joinable() && (current == SessionState::WaitingForInterface || current == SessionState::Hosting);
}

unsigned long long HostSession::transferredBytes() const
{
    return mService ? mService->totalBytesTransferred() : mFinalTransferredBytes;
}

std::string HostSession::lastError() const
{
    const std::lock_guard<std::mutex> lock(mErrorMutex);
    return mLastError;
}

std::vector<std::string> HostSession::drainLog()
{
    return logBuffer().drain();
}

} // namespace nowindgui
