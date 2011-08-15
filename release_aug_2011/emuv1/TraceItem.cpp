// TraceItem.cpp

#include "TraceItem.h"
#include <string>

using namespace std;

TraceItem::TraceItem(Uint32	theSize, Uint32 theNeededSize, string theFile, Uint32 theLine, string theType) {
    size = theSize;
    neededSize = theNeededSize;
    file = theFile;
    line = theLine;
    type = theType;
}

TraceItem::~TraceItem() {

}
