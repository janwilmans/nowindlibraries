#ifndef NOWINDHOST_DEBUG_HH
#define NOWINDHOST_DEBUG_HH

#ifdef _MSC_VER
# pragma warning( disable: 4251 )
#endif 

#include "NowindHost.hh"
#include "NowindHostSupportDebug.h"
#include <stdarg.h>		// for va_list

namespace nwhost {

class NowindHostDebug : public NowindHost
{
public:
	NowindHostDebug(const std::vector<DiskHandler*>& drives);
	virtual ~NowindHostDebug();
	
	virtual void Initialize();
	virtual void debugMessage(const char *cFormat, ...) const;

};

} // namespace nowind

#endif // NOWINDHOST_DEBUG_HH
