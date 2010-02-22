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

	FTDX_API enum FtdiDriverType { eDRIVER_FTD2XX, eDRIVER_LibUsb, eDRIVER_FtdiSio };
    FTDX_API void initialize();
    FTDX_API UsbStream* newUsbStream(FtdiDriverType aDriverType);
}

#endif // FTDX_H
