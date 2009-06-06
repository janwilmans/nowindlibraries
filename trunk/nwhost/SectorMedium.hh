//! SectorMedium.h
#ifndef SECTORMEDIUM_H
#define SECTORMEDIUM_H

typedef unsigned char nw_byte;

namespace nwhost {

class SectorMedium {

public:
		virtual ~SectorMedium() {};

		virtual int readSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount) = 0;
		virtual int writeSectors(nw_byte * buffer, unsigned int startSector, unsigned int sectorCount) = 0;
		virtual bool isWriteProtected() = 0;
};

} // namespace nowind

#endif //SECTORMEDIUM_H
