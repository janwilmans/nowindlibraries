/**
 * @file ftdx.h
 *
 * @brief static methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef FTDX_H
#define FTDX_H

#include "FtdxExports.h"

namespace ftdx {
	class UsbStream;

	enum DriverType { eFTD2XX, eLibUsb, eFtdiSio };
    void FTDX_API initialize();
    UsbStream* newUsbStream(DriverType aDriverType);
}

#endif // FTDX_H
