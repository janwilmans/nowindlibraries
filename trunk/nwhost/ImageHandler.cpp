// ImageHandler.cpp

#include <stdio.h>
#include <assert.h>

#include "ImageHandler.h"

using namespace std;

SectorMedium * ImageHandler::getSectorMedium() {
	return &image;
}

int ImageHandler::insertDisk(std::string filename) {
	return image.openDiskImage(filename) ? 0 : -1;
}

bool ImageHandler::diskChanged() {
	return image.isDiskChanged();
}

bool ImageHandler::isRomdisk() {
	return image.isRomdisk();
}
