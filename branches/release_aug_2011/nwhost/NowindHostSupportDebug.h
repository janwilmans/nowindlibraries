#ifndef NOWINDHOSTSUPPORT_DEBUG_HH
#define NOWINDHOSTSUPPORT_DEBUG_HH

/*
#ifdef _MSC_VER
# pragma warning( disable: 4251 )
#endif 
*/

#include "NowindHostSupport.hh"
#include <stdarg.h>		// for va_list

namespace nwhost {

class NowindHostSupportDebug : public NowindHostSupport
{
public:
	NowindHostSupportDebug();
	virtual ~NowindHostSupportDebug();
	virtual void debugMessage(const char *cFormat, ...) const;
};

} // namespace nowind

#endif // NOWINDHOSTSUPPORT_DEBUG_HH
