/**
 * @file nwhost.cpp
 *
 * @brief Contains static nwhost methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */

#define NWHOST_API_EXPORT
#include "nwhost.h"
#include <libgeneral.h>

#include "nowindusb.h"

void nwhost::initialize()
{
    general::initialize();
    nowindusb_startup();
}
