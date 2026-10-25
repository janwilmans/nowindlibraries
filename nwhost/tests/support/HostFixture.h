//! HostFixture.h
#ifndef NWTEST_HOSTFIXTURE_H
#define NWTEST_HOSTFIXTURE_H

#include "MemoryDisk.h"
#include "MsxSide.h"

#include "NowindHost.hh"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace nwtest {

// a NowindHost with in-memory drives, driven through an MsxSide
class HostFixture : public ::testing::Test {
protected:
    // the host keeps a reference to the drive list, so it is created after the drives were added
    void createHost(unsigned driveCount = 1)
    {
        for (unsigned i = 0; i < driveCount; ++i) addDrive();
        startHost();
    }

    MemoryDisk& addDrive(unsigned sectors = 1440)
    {
        disks.push_back(std::make_unique<MemoryDisk>(sectors));
        drives.push_back(disks.back().get());
        return *disks.back();
    }

    void startHost()
    {
        host = std::make_unique<nwhost::NowindHost>(drives);
        host->initialize();
        msx = std::make_unique<MsxSide>(*host);
    }

    MemoryDisk& disk(unsigned index = 0) { return *disks.at(index); }

    std::vector<std::unique_ptr<MemoryDisk>> disks;
    std::vector<nwhost::DiskHandler*> drives;
    std::unique_ptr<nwhost::NowindHost> host;
    std::unique_ptr<MsxSide> msx;
};

} // namespace nwtest

#endif // NWTEST_HOSTFIXTURE_H
