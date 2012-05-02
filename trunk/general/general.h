/**
 * @file general.h
 *
 * @brief static general methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef GENERAL_H
#define GENERAL_H

#include "GeneralExports.h"

namespace general
{
    GEN_API void initialize();
    GEN_API void beep(unsigned long freq, unsigned long duration);
}

#endif // GENERAL_H
