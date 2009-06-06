//! PartitionInfo.h
#ifndef PARTITIONINFO_H
#define PARTITIONINFO_H

#include <string>
#include <fstream>

class PartitionInfo {

public:
    PartitionInfo();
	virtual ~PartitionInfo();

	unsigned char mediaDescriptor;
	unsigned int index;
    unsigned int startLBA;
    unsigned int length;
	bool inserted;
	bool readonly;	// bit 0
	bool disabled;	// bit 1
	bool bootable;	// bit 7
  ;
};

#endif //PARTITIONINFO_H

