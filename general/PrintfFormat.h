//! PrintfFormat.h
#pragma once

// lets GCC and Clang check the arguments of a printf-style function against its format string (-Wformat);
// the indexes count from 1, in a non-static member function the implicit 'this' is argument 1
#if defined(__GNUC__) || defined(__clang__)
#define NW_PRINTF_FORMAT(formatIndex, argumentsIndex) __attribute__((format(printf, formatIndex, argumentsIndex)))
#else
#define NW_PRINTF_FORMAT(formatIndex, argumentsIndex)
#endif
