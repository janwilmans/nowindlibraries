#ifndef NO_TRACEALLOCATOR

/*
 * This file is included in every class through Debug.h if NO_TRACEALLOCATOR is not defined
 */

//! TraceAllocaInclude.h
#ifndef TRACEALLOC_INCLUDE_H
#define TRACEALLOC_INCLUDE_H

#include "TraceAllocations.h"

    /* macro trick to have the pre-processor replace new X with new(__FILE__, __LINE__)X ; 
     * unfortunately this does not seem to work for delete
     */
    
    #define DEBUG_NEW new(__FILE__, __LINE__)
    #define new DEBUG_NEW

    //#define DEBUG_DEL delete(__FILE__, __LINE__)
    //#define delete DEBUG_DEL
#endif 

#endif //NO_TRACEALLOCATOR
