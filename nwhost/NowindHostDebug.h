#ifndef NOWINDHOST_DEBUG_HH
#define NOWINDHOST_DEBUG_HH

#ifdef _MSC_VER
# pragma warning( disable: 4251 )
#endif 

#include "NowindHost.hh"
#include <stdarg.h>		// for va_list

#include "NwhostExports.h"

namespace nwhost {

class NWHOST_API NowindHostDebug : public NowindHost
{
public:
	NowindHostDebug(const std::vector<DiskHandler*>& drives) : NowindHost(drives) {}
	virtual ~NowindHostDebug() {}
	virtual void debugMessage(const char *cFormat, ...);
};

} // namespace nowind

#endif // NOWINDHOST_DEBUG_HH
