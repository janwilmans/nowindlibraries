//! diskHandler->h
#ifndef DISKHANDLER_H
#define DISKHANDLER_H

#include <string>

namespace nowind {

class SectorMedium;

class DiskHandler {

public:
		virtual SectorMedium * getSectorMedium() = 0;
		virtual int insertDisk(std::string filename) = 0;
		virtual bool diskChanged() = 0;
		virtual bool isRomdisk() = 0;
		virtual ~DiskHandler() {};
};

} // namespace nowind

#endif //DISKHANDLER_H
