/**
 * @file Util.cpp
 *
 * @brief Contains utility methods
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */

#define GEN_API_EXPORT
#include "Util.h"
#include <stdio.h>

using namespace general;

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/unistd.h>		// for usleep
	#include <sys/time.h>		// for timeval
#endif

/*
// for the MSVC compiler sprintf_s (and also _snprintf_s, vsnprintf, vsprintf_s) are not c99 compliant, 
// their return value is -1 if the string does not fit, in stead of the amount of characters that would
// have been written if the it had fitted.

C99 Return value:
       Upon successful return, these functions return the number of characters  printed  (not  including  the
       trailing  '\0' used to end output to strings).  The functions snprintf and vsnprintf do not write more
       than size bytes (including the trailing '\0').  If the output was truncated due to this limit then the
       return value is the number of characters (not including the trailing '\0') which would have been writ-
       ten to the final string if enough space had been available. Thus, a return value of size or more means
       that  the  output  was  truncated. (See also below under NOTES.)  If an output error is encountered, a
       negative value is returned.
*/

#ifdef _MSC_VER

// sprintf_s (and also _snprintf_s) are not c99 compliant on MSC
// we implement it by wrapping _vscprintf and _vsnprintf_s
int Util::vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    size_t count;
    count = _vscprintf(format, ap);
    _vsnprintf_s(str, size, _TRUNCATE, format, ap);
    return count;
}

#else // not MSV_VER	

int Util::vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    return ::vsnprintf(str, size, format, ap);
}
#endif // _MSC_VER

int Util::snprintf(char* str, size_t size, const char* format, ...) 
{
    size_t count;
    va_list ap;
    va_start(ap, format);
    count = Util::vsnprintf(str, size, format, ap);
    va_end(ap);
    return count;
}

#define MAXMSG 2000

void Util::debug(const char *cFormat, ...)
{
	va_list arg_list;
	char scratch[MAXMSG];
	va_start (arg_list, cFormat);  
	vsnprintf(scratch, MAXMSG, cFormat, arg_list);
	va_end (arg_list);

	fprintf(stderr, scratch);
#ifdef WIN32
    OutputDebugStringA(scratch);
#endif

}

// code borrowed from SDL

#ifdef WIN32

/* The first high-resolution ticks value of the application */
static LARGE_INTEGER hires_start_ticks;
/* The number of ticks per second of the high-resolution performance counter */
static LARGE_INTEGER hires_ticks_per_second;

void Util::_startTicks(void)
{
	QueryPerformanceFrequency(&hires_ticks_per_second);
	QueryPerformanceCounter(&hires_start_ticks);
}

unsigned long Util::getTicks(void)
{
	LARGE_INTEGER hires_now;

	QueryPerformanceCounter(&hires_now);

	hires_now.QuadPart -= hires_start_ticks.QuadPart;
	hires_now.QuadPart *= 1000;
	hires_now.QuadPart /= hires_ticks_per_second.QuadPart;
	return (DWORD)hires_now.QuadPart;
}

void Util::sleep(unsigned long aMilliseconds)
{
	Sleep(aMilliseconds);
}

#else // not WIN32

/* The clock_gettime provides monotonous time, so we should use it if
   it's available. The clock_gettime function is behind ifdef
   for __USE_POSIX199309
   Tommi Kyntola (tommi.kyntola@ray.fi) 27/09/2005
*/

/* The first ticks value of the application */
#ifdef HAVE_CLOCK_GETTIME
static struct timespec start;
#else
static struct timeval start;
#endif /* HAVE_CLOCK_GETTIME */

void Util::_startTicks(void)
{
	/* Set first ticks value */
#if HAVE_CLOCK_GETTIME
	clock_gettime(CLOCK_MONOTONIC,&start);
#else
	gettimeofday(&start, NULL);
#endif
}

unsigned long Util::getTicks(void)
{
#if HAVE_CLOCK_GETTIME
	unsigned long ticks;
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC,&now);
	ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_nsec-start.tv_nsec)/1000000;
	return(ticks);
#else
	unsigned long ticks;
	struct timeval now;
	gettimeofday(&now, NULL);
	ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
	return(ticks);
#endif
}

void Util::sleep(unsigned long aMilliseconds)
{
	// 4.3BSD, POSIX.1-2001, check HAVE_USLEEP ?
	usleep(1000*aMilliseconds);
}

#endif // WIN32
