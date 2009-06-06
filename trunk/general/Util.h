/**
 * @file Util.h
 *
 * @brief Contains utility methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>		// for va_list
#include <stdio.h>		// for size_t
#include "GeneralExports.h"

class GEN_API Util
{
public:

	static int vsnprintf(char *str, size_t size, const char *format, va_list ap);
	static int snprintf(char* str, size_t size, const char* format, ...);

	static void initialize();
    static void debug(const char *cFormat, ...);
	static unsigned long getTicks(void);
	static void sleep(unsigned long);
private:
   	static void startTicks(void);

};

#endif  //_UTIL_H
