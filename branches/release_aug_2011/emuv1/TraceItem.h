//! TraceItem.h
#ifndef TRACEITEM_H
#define TRACEITEM_H

#include <string>
#include "msxtypes.h"

class TraceItem {

public:
    TraceItem(Uint32 theSize, Uint32 theNeededSize, std::string theFile, Uint32 theLine, std::string theType);
    ~TraceItem();

    Uint32	    size;             // memory requested by caller
    Uint32	    neededSize;       // actually allocated size including guard data
    std::string file;
    Uint32	    line;      
    std::string type;
};

#endif
