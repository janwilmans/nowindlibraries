// PartitionInfo.cpp

#include <stdio.h>
#include <cassert>

#include "PartitionInfo.h"

using namespace std;

PartitionInfo::PartitionInfo() {

	inserted = false;
	readonly = false;
	disabled = false;
	bootable = false;
	startLBA = 0;
	length = 0;
}

PartitionInfo::~PartitionInfo() {

}

// todo: why does the nwhost known anything about partitions? A: see nowmap functions