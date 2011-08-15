#ifndef NWHOST_INTERNAL_H
#define NWHOST_INTERNAL_H

#define MAXMSG 400
#define DBERR nowindusb_debug_wrap_sprintf

extern void nowindusb_debug_wrap_sprintf(const char *cFormat, ...);
extern void nowindusb_debug(const char *msg);

#endif // NWHOST_INTERNAL_H


