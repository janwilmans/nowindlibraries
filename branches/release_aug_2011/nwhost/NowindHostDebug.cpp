/**
 * @file NowindHostDebug.cpp
 *
 * @brief implement the NowindHost debugMessage method to log debug messages
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
 
#define NWHOST_API_EXPORT
#include "NowindHostDebug.h"
#include <libgeneral.h>

#include "NwhostInternal.h"
#include <stdarg.h>		// for va_list

using namespace general;
using namespace nwhost;

NowindHostDebug::NowindHostDebug(const std::vector<DiskHandler*>& drives) : 
    NowindHost(drives)
{
    
}

void NowindHostDebug::initialize()
{
    nwhSupport = new NowindHostSupportDebug();
    NowindHost::initialize();
}

NowindHostDebug::~NowindHostDebug()
{

}

void NowindHostDebug::debugMessage(const char *cFormat, ...) const
{
	va_list arg_list;
	char scratch[2000];
	va_start (arg_list, cFormat);  
    Util::vsnprintf(scratch, MAXMSG, cFormat, arg_list);
	va_end (arg_list);
    nowindusb_debug(scratch);

}
