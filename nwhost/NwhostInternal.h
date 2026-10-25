#ifndef NWHOST_INTERNAL_H
#define NWHOST_INTERNAL_H

#include "PrintfFormat.h"

#define MAXMSG 400
#define DBERR nowindusb_debug_wrap_sprintf

extern void nowindusb_debug_wrap_sprintf(const char* cFormat, ...) NW_PRINTF_FORMAT(1, 2);
extern void nowindusb_debug(const char* msg);

#endif // NWHOST_INTERNAL_H
