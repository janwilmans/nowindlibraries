/**
 * @file UsbStream.h
 *
 * @brief Contains connection interface methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef CONNECTION_H
#define CONNECTION_H

#include <atomic>
#include <string>
#include <fstream>

#include "ftdx.h"
#include "FtdxExports.h"

namespace ftdx {

class FTDX_API UsbStream {
public:
    UsbStream();
    virtual ~UsbStream();
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual int readExact(unsigned char* buffer, unsigned long bytesToRead) = 0;
    virtual int readBlocking(unsigned char* buffer, unsigned long maxBytesToRead) = 0;
    virtual void write(unsigned char* buffer, unsigned long bytesToWrite, unsigned long* bytesWritten) = 0; // may block until all data is send
    virtual const char* getIdString() = 0;
    virtual void setTimeouts(unsigned int rxTimeout, unsigned int txTimeout);
    virtual void reset() = 0;
    virtual void purgeRx() = 0;
    virtual void purgeTx() = 0;

    void openBlocking();
    static void initialize();

    // readBlocking() returns 0 once *keepRunning is false, so another thread can stop a host that waits for the MSX;
    // the flag must outlive this stream
    void setKeepRunningFlag(const std::atomic<bool>* keepRunning);

protected:
    [[nodiscard]] bool stopRequested() const;

    const std::atomic<bool>* mKeepRunning = nullptr;
    unsigned int mRxTimeout;
    unsigned int mTxTimeout;
    bool mTimeoutSet;
    bool mUsbStreamOpen;
};

} // namespace ftdx
#endif
