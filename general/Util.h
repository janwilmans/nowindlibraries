/**
 * @file Util.h
 *
 * @brief Contains utility methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h> // for va_list
#include <stdio.h>  // for size_t
#include "GeneralExports.h"
#include "PrintfFormat.h"

namespace general {

class GEN_API Util {
public:
    static int vsnprintf(char* str, size_t size, const char* format, va_list ap) NW_PRINTF_FORMAT(3, 0);
    static int snprintf(char* str, size_t size, const char* format, ...) NW_PRINTF_FORMAT(3, 4);

    static void initialize();
    static void debug(const char* cFormat, ...);
    static unsigned long getTicks(void);
    static void sleep(unsigned long);

    // not part of the external library interface
    static void _startTicks(void);
};

} // namespace general

#endif //_UTIL_H
