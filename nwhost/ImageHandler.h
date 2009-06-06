//! ImageHandler.h
#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <string>
#include <fstream>

#include "Image.h"
#include "PartitionInfo.h"
#include "DiskHandler.hh"
#include "SectorMedium.hh"

using namespace nowind;

class ImageHandler : public nowind::DiskHandler {

private:
	Image image;
public:
	virtual ~ImageHandler() {};

	virtual SectorMedium * getSectorMedium();
	virtual int insertDisk(std::string filename);
	virtual bool diskChanged();
	virtual bool isRomdisk();
};

#endif // IMAGEHANDLER_H

