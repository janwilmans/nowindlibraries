/**
 * @file UsbStream.cpp
 *
 * @brief Contains connection interface methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */

#define FTDX_API_EXPORT
#include "UsbStream.h"
#include "libgeneral.h"     // for Util::debug / Util::sleep

// should be called once before creating any UsbStream object
void UsbStream::initialize()
{
    Util::initialize();
}

UsbStream::UsbStream()
{
	mTimeoutSet = false;
}

// wait until a mUsbStream has been made
void UsbStream::openBlocking()
{
    bool retrying = false;
    while (open() == false) {
        if (!retrying) {
            Util::debug("Using driver %s, please insert nowind interface...\n", getIdString());
            retrying = true;
        }
        Util::sleep(250);
    }
    Util::debug("Found Nowind Interface using %s!\n", getIdString());

}

UsbStream::~UsbStream()
{
}

void UsbStream::setTimeouts(unsigned int aRxTimeout, unsigned int aTxTimeout)
{
	mRxTimeout = aRxTimeout;
	mTxTimeout = aTxTimeout;
	mTimeoutSet = true; 
}
