
#include "NowindHostSupport.hh"

namespace nwhost {

NowindHostSupport::NowindHostSupport()
{
}

NowindHostSupport::~NowindHostSupport()
{
}

void NowindHostSupport::debugMessage(const char *, ...) const
{
    // override this method in a subclass to log messages
}

} // namespace nwhost
