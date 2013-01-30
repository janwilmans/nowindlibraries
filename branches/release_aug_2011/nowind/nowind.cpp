/**
 * @file nowind.cpp
 *
 * @brief Contains static nowind methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */

#define NOWIND_API_EXPORT
#include "nowind.h"
#include "NwhostService.h"
#include <libnwhost.h>
#include <libftdx.h>

void nowind::initialize()
{
    ftdx::initialize();
    NwhostService::initialize();    // installs debug callback
    nwhost::initialize();
}
