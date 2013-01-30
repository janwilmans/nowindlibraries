#ifndef _NOWINDUSB_H_
#define _NOWINDUSB_H_

#include "NwhostExports.h"

typedef void (*FP_void_const_char_p)(const char *);
typedef void (*FP_void_void_p)(void);

#include "NowindTypes.hh"

// call nowindusb_set_debug_callback if you want debug info written somewhere
NWHOST_API void CCALL nowindusb_set_debug_callback(FP_void_const_char_p aHostdebug_callback);

// call nowindusb_startup
NWHOST_API void CCALL nowindusb_startup(void);

// call this at exit
NWHOST_API void CCALL nowindusb_cleanup(void);

// insert a disk image, can be an image of a single partition or a real disk image
NWHOST_API bool CCALL nowindusb_set_image(unsigned int driveNr, const char *filename);

// the filename here contains a harddisk image with a partition table
// the partition table is used to find the start of the partition within the file
NWHOST_API unsigned int CCALL nowindusb_set_harddisk_image(unsigned int driveNr, int partitionNumber, bool ignoreBootflag, const char *filename);

NWHOST_API bool nowindusb_set_romdisk(unsigned int driveNr);

// write one byte of command-data to the host       (msx -> pc)
NWHOST_API void CCALL nowindusb_write(unsigned char, unsigned int msTime);

// read one byte of response-data from the host     (msx <- pc)
NWHOST_API unsigned char CCALL nowindusb_read(void);

// used in usb-host implementation (for real usb hardware)
NWHOST_API bool nowindusb_is_data_available(void);

NWHOST_API int CCALL nowindusb_attribute(nwhost::nw_attribute aAttribute, bool aValue);

NWHOST_API void CCALL nowindusb_clear_requests();
NWHOST_API void CCALL nowindusb_clear_startup_requests();

NWHOST_API void CCALL nowindusb_add_request(const nwhost::byte* cRequest);
NWHOST_API void CCALL nowindusb_add_startup_request(const nwhost::byte* cRequest);



#endif /* _NOWINDUSB_H_ */
