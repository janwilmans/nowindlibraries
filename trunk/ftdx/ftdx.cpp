/**
 * @file ftdx.cpp
 *
 * @brief Contains static ftdx methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */

#define FTDX_API_EXPORT
#include "ftdx.h"
#include "libgeneral.h"

#include "ConFTD2XX.h"
#include "ConFtdiSio.h"
#include "ConLibFtdi.h"

using namespace general;

void ftdx::initialize()
{
    general::initialize();
}

ftdx::UsbStream* ftdx::newUsbStream(FtdiDriverType aDriverType)
{
	UsbStream* lUsbStream = 0;
	switch (aDriverType)
	{
	case eLibUsb:
		lUsbStream = new ConLibFtdi();
		break;
	case eFTD2XX:
		lUsbStream = new ConFTD2XX();
		break;
#ifndef WIN32
	case eFtdiSio:
		lUsbStream = new ConFtdiSio();
		break;
#endif
	default:
		Util::debug("unknown driver type %u specified, null returned\n", aDriverType);
		break;	
	}
	return lUsbStream;	
}
