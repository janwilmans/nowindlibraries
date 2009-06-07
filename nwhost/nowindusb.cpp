#include <libgeneral.h>

#include "NwhostInternal.h"
#define NWHOST_API_EXPORT
#include "nowindusb.h"

#define DBERR nowindusb_debug_wrap_sprintf


#include "NowindHostDebug.h"
#include "ImageHandler.h"
#include "Image.h"

#include <vector>
#include <list>
#include <stdio.h>
#include <stdarg.h>

using namespace general;
using namespace nwhost;

FP_void_const_char_p mDebug_callback;
FP_void_void_p mDataAvailable_callback;

// static linking requires: (and maybe the directx sdk)
// Sdl.lib ftd2xx.lib WINMM.LIB

/* some global instances */

nwhost::NowindHost * nowindHost;
typedef std::vector<DiskHandler*> Drives;
Drives drives;

static const unsigned MAX_MEDIA = 8;

/* this library can be used in: 
 * - the portable usbhost (cmdline version)
 * - libnowindapp
 * - usbhost wxpython GUI (todo: create this gui)
 * - nowind MSX emulator
 * - other emulators like BlueMsx
 *
 */

/* methods for passing debug messages */

void nowindusb_set_debug_callback(FP_void_const_char_p aHostdebug_callback)
{
    mDebug_callback = aHostdebug_callback;
}

// actually write a message to an output
void nowindusb_debug(const char *msg)
{
    char lTemp[MAXMSG]; lTemp[0] = 0;
    // if the host registered a debug output callback function, 
    // pass the output to it
    if (mDebug_callback) {
        Util::snprintf(lTemp, MAXMSG, "[LIBNW] %s", msg);
        mDebug_callback(lTemp);
    }
#ifdef WIN32
	OutputDebugStringA(lTemp);
#endif
}

// the DBERR macro uses this method to wrap vsprintf for convenience
void nowindusb_debug_wrap_sprintf(const char *cFormat, ...)
{
	va_list arg_list;
	char scratch[MAXMSG];		// notice: HARDCODED limit of MAXMSG bytes !
	
	va_start (arg_list, cFormat);  
    Util::vsnprintf(scratch, MAXMSG, cFormat, arg_list);
	va_end (arg_list);

	nowindusb_debug(scratch);
}

void nowindusb_startup(void)
{
	mDebug_callback = 0;
	drives.push_back(new ImageHandler());
	nowindHost = new nwhost::NowindHostDebug(drives);
	nowindusb_debug("nowindusb_startup\n");

	//nowindusb_set_image(0, "c:\\disk_dos2.dsk");
	//nowindusb_set_image(1, "c:\\disk_hd.dsk");
}

void nowindusb_cleanup(void)
{
    nowindusb_debug("nowindusb_cleanup\n");
    delete nowindHost;
    for (Drives::iterator it = drives.begin(); it != drives.end(); ++it) {
        delete *it;
    }
}

unsigned int NowindHost_InsertPartition(unsigned int driveNr, unsigned int partitionNumber, std::string aHarddiskimage) {

	if (driveNr < MAX_MEDIA) {
		// don't use openPartitionImage directly, we want to report results to the user
		return nowindusb_set_harddisk_image(driveNr, partitionNumber, false, aHarddiskimage.c_str());
	}
	DBERR("Cannot insert %s:%u, out of drives\n", aHarddiskimage.c_str(), partitionNumber);
	return 0;
}

unsigned int NowindHost_InsertHarddiskPartitions(unsigned int driveNr, std::string aHarddiskimage) {

	std::list<PartitionInfo> partitionList;
	unsigned int insertedPartitions = 0;

	Image * hdImage = new Image();
	hdImage->openDiskImage(aHarddiskimage);
	const nw_byte * partitionTable = hdImage->GetNewPartitionTable();
	delete hdImage;

	for (int i=0;i<30;i++)
	{
		PartitionInfo p;
		p.index = i;
		Image::GetPartitionInfo(partitionTable, &p, i);
		if (p.startLBA == 0 || p.length == 0) break;
		partitionList.push_back(p);
		DBERR("Partition: %u, bootable: %u, disabled: %u, readonly: %u, mediadescriptor: 0x%02X\n", i, p.bootable, p.disabled, p.readonly, p.mediaDescriptor);
	}

	for(std::list<PartitionInfo>::iterator p = partitionList.begin(); p != partitionList.end();p++)
	{
		if ((*p).bootable) {
			(*p).inserted = true;
			unsigned int result = NowindHost_InsertPartition(driveNr, (*p).index, aHarddiskimage);
			if (result > 0)
			{
				driveNr++;
				insertedPartitions++;
			}
			break;
		}
	}

	for(std::list<PartitionInfo>::iterator p = partitionList.begin(); p != partitionList.end();p++)
	{
		if ((*p).inserted) continue;
		unsigned int result = NowindHost_InsertPartition(driveNr, (*p).index, aHarddiskimage);
		if (result > 0)
		{
			driveNr++;
			insertedPartitions++;
		}
	}

	delete [] partitionTable;
	return insertedPartitions;
}

/* usb communication */

bool nowindusb_set_image(unsigned int driveNr, const char *filename)
{
	//driveNr = 0;
	//DBERR("nowindusb_set_image patched using driver 0 anyway\n");

	DBERR("nowindusb_set_image %u, %s\n", driveNr, filename);

	if (driveNr >= MAX_MEDIA) {
		DBERR("Too many images! (%s ignored)\n", filename);
		return false;
	}

	while (drives.size() <= driveNr) {
		drives.push_back(new ImageHandler());
	}

	int result = drives[driveNr]->insertDisk(filename);
	return (result == 0);
}

unsigned int nowindusb_set_harddisk_image(unsigned int driveNr, int partitionNumber, bool ignoreBootflag, const char *filename)
{
	DBERR("nowindusb_set_harddisk_image %u, %s\n", driveNr, filename);
	bool result = false;
	if (partitionNumber == 0xff)
	{
		// use autodetection
		return NowindHost_InsertHarddiskPartitions(driveNr, filename);
	}

	// make sure it exists, todo: re-design this?
	nowindusb_set_image(driveNr, filename);

	if (driveNr < drives.size()) {
		ImageHandler* imageHandler = static_cast<ImageHandler*>(drives[driveNr]);
		DBERR(" %s part: %u -> drive: %u\n", filename, partitionNumber, driveNr);
		Image * image = static_cast<Image *> (imageHandler->getSectorMedium());
		bool success = image->openPartitionImage(partitionNumber, ignoreBootflag, filename);
		if (success) {
			result = true;
		} else {
			DBERR(" %s part: %u not inserted (disabled?)\n", filename, partitionNumber);
		}
    } else {
        DBERR("Too many images! (%s ignored)\n", filename);
    }
	return result;
}

bool nowindusb_set_romdisk(unsigned int driveNr)
{
	/*
	bool result = false;
	if (driveNr < MAX_MEDIA) {
        nowindHost->media[driveNr]->setRomdisk();
		result = true;
    } else {
        DBERR("Too many images! (no romdisk)\n");
    }
	return result;
	*/
	return 0;
}

void nowindusb_write(unsigned char value)     // (msx -> pc)
{
    nowindHost->write(value, Util::getTicks());
}

unsigned char nowindusb_read(void)  // (msx <- pc) for emulators
{
    return nowindHost->read();
}

int nowindusb_attribute(nw_attribute aAttribute, bool aValue)
{
	switch (aAttribute) {
	case enable_phantom_drives:
		nowindHost->setEnablePhantomDrives(aValue);
		break;
	case allow_other_diskroms:
		nowindHost->setAllowOtherDiskroms(aValue);
		break;
	default:
		break;
	}
	return 0;
}

int nowindusb_cmd(unsigned int aCommandId, char * aCharArgument, unsigned int aIntArgument1, unsigned int aIntArgument2, unsigned int aIntArgument3, unsigned int aIntArgument4)
{
	//at_commands { cmd_none = 0, cmd_putmem_clear, cmd_putmem };
	switch(aCommandId)
	{
	case cmd_putmem_clear:
		// clear putmem list
		//nowindHost->putmemClear();
		break;
	case cmd_putmem:
		//nowindHost->putMem(aCharArgument, aIntArgument1, aIntArgument2, aIntArgument3, aIntArgument4);
		break;
	default:
		break;
	}
	return 0;
}

bool nowindusb_is_data_available()
{
	return nowindHost->isDataAvailable();
}

