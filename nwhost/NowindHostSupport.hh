#ifndef NOWNDHOSTSUPPORT_HH
#define NOWNDHOSTSUPPORT_HH

#include "NowindTypes.hh"

namespace nwhost {

class NowindHostSupport
{
public:
	NowindHostSupport();
	virtual ~NowindHostSupport();
	virtual void debugMessage(const char *cFormat, ...) const;

};

} // namespace nwhost

#endif // NOWNDHOSTSUPPORT_HH
