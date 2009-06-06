/**
 * @file NowindHostDebug.cpp
 *
 * @brief implement the NowindHost debugMessage method to log debug messages
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#include "NowindHostDebug.h"
#include <libgeneral.h>

#include "nwhost_internal.h"
#include <stdarg.h>		// for va_list

using namespace nowind;

void NowindHostDebug::debugMessage(const char *cFormat, ...)
{
	va_list arg_list;
	char scratch[2000];
	va_start (arg_list, cFormat);  
    Util::vsnprintf(scratch, MAXMSG, cFormat, arg_list);
	va_end (arg_list);
    nowindusb_debug(scratch);
}
