#pragma once

#include "HostConfiguration.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace nowind {
class NwhostService;
}

namespace nowindgui {

enum class SessionState {
    Stopped,
    WaitingForInterface, // started, the Nowind interface is not (yet) connected
    Hosting,             // connected, serving the drives
    Failed               // stopped because of an error, see lastError()
};

// Runs NwhostService::hostImage() on a worker thread. The nowind libraries keep their state in globals, so there can
// be only one HostSession in a process. All members are for the thread that owns the session (the UI thread).
class HostSession {
public:
    HostSession();
    ~HostSession();
    HostSession(const HostSession&) = delete;
    HostSession& operator=(const HostSession&) = delete;

    void start(const HostConfiguration& configuration);
    void stop();

    [[nodiscard]] SessionState state() const;
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] unsigned long long transferredBytes() const;
    [[nodiscard]] std::string lastError() const;

    // the complete lines logged since the last call (also by the libraries, from any thread)
    [[nodiscard]] std::vector<std::string> drainLog();

private:
    void run(const HostConfiguration& configuration);

    std::unique_ptr<nowind::NwhostService> mService;
    std::thread mWorker;
    std::atomic<SessionState> mState{SessionState::Stopped};
    mutable std::mutex mErrorMutex;
    std::string mLastError;
    unsigned long long mFinalTransferredBytes = 0;
};

} // namespace nowindgui
