#ifndef NO_TRACEALLOCATOR

// Do NOT add the memory traceallocator to the traceallocator functions!!! :)
#define NO_TRACEALLOCATOR

#include "Debug.h"
#include "TraceItem.h" 
#include "TraceAllocations.h"

// it is required to re-define new _before_ including stl-containers 
// to avoid conflicts is MSC-stl memory checks
#include <map>
#include <list>

#undef NO_TRACEALLOCATOR

/*
 *     Type definitions
 */
typedef std::map<void *, TraceItem *> traceMapType;
typedef std::list<TraceItem *> traceListType;

/*
 *     Global variable definitions
 */
traceMapType traceMap;
traceListType unusedList;

static const Uint32 prefix = 4;        // bytes to reserve before the actual data
static const Uint32 postfix = 4;        // bytes to reserve after the actual data
static const Uint32 markerSize = prefix+postfix;

#define ADDPTR(p,v) ((void *) (((unsigned int) p)+v))
#define SUBPTR(p,v) ((void *) (((unsigned int) p)-v))

void CheckUnused(void * ptr, TraceItem *i)
{
	//DBERR("CheckUnused: checking whether the memory at 0x%08X was used at all\n", ptr);
	unsigned char * start = (unsigned char *) ADDPTR(ptr, prefix);
	unsigned char * end = (unsigned char *) ADDPTR(ptr, prefix+i->size);

	if (end <= start) {
		DBERR("trace-error: end before or equal to start?!\n");
		return;
	}

	bool bUnused = true;
	for (unsigned char * p = start; p < end; p++)
	{
		//DBERR("check: memory at 0x%08X (0x%02X)\n", p, *p);
		if (*p != 0xa5) 
		{
			bUnused = false;
		}
	}
	if (bUnused)
	{
		//DBERR("CheckUnused: memory at 0x%08X was NOT used at all\n", ptr);
		unusedList.push_back(new TraceItem(*i));
	}
}

void TraceCheckInternal(void * ptr, TraceItem *i)
{
	bool errorDetected = false;
	//DBERR("TraceCheckInternal: checking magic markers of 0x%08X allocated at %s:%u\n", ptr, i->file.c_str(), i->line);

	// Check the begin-marker is in-tact
	unsigned int *markBegin = (unsigned int *) ptr;
	if ((*markBegin) != 0xa1a2a3a4)
	{
		DBERR("TraceCheckInternal: markBegin check FAILED for %s at 0x%08X allocated at %s:%u\n", i->type.c_str(), ptr, i->file.c_str(), i->line);
		errorDetected = true;
	}

	// Check for in-tact no-mans land poison 
	unsigned char * nomansLand = (unsigned char *) ADDPTR(ptr, prefix+i->size);
	unsigned char * nomansEnd = (unsigned char *) ADDPTR(nomansLand, i->neededSize-markerSize-i->size);
	
	if (nomansEnd < nomansLand) {
		DBERR("trace-error: negative amount of nomansland\n");
		return;
	}
	
	for (unsigned char * p = nomansLand; p < nomansEnd; p++)
	{
		if (*p != 0xa6)
		{
			DBERR("TraceCheckInternal: Nomans land area overwritten (0x%02X != 0x%02X) for %s at 0x%08X allocated at %s:%u\n", *p, 0xa6, i->type.c_str(), p, i->file.c_str(), i->line);
			errorDetected = true;
		}
	}

	// Check the end-marker is in-tact
    unsigned int *markEnd = (unsigned int *) ADDPTR(ptr, i->neededSize-postfix);
	if ((*markEnd) != 0xf1f2f3f4)
	{
		DBERR("TraceCheckInternal: markEnd check FAILED for %s at 0x%08X allocated at %s:%u\n", i->type.c_str(), ptr, i->file.c_str(), i->line);
		errorDetected = true;
	}
	
	// comment this line to just LOG errors, and not stop when they occur
	//assert(!errorDetected);
}

void TraceCheck(void * ptr)
{
	traceMapType::iterator iter = traceMap.find(ptr);
	if (iter == traceMap.end())
	{
		DBERR("TraceCheck: 0x%08X is not a valid address\n", ptr);
	}
	else
	{
		// enable to be much more verbose
		//DBERR("TraceCheck: 0x%08X\n", ptr);

		// restore the ptr to it's actual value
		ptr = SUBPTR(ptr, prefix);	    
		TraceCheckInternal(ptr, iter->second);
	}
}

/*
 * Allocated memory will be surrounded by 8 guard-bytes that will be
 * checked at deallocation. (a leading and trailing unsigned int)
 */
inline void * allocate(unsigned int size, const char *file, int line, const char *type)
{
	// before allocation new memory, check for corruption in existing markers
	CheckSanity();

	Uint32 extraNomansLand = 0;
	size = size + extraNomansLand;

	Uint32 neededSize = (size + 3) & 0xffffffc;  // actual allocated memory for data (assumption: 32 bit architecture)
    neededSize += markerSize;                    // add area of markers

    void * ptr = (void *) malloc(neededSize);
    memset(ptr, 0xa6, neededSize);		// no-mans land poison

	unsigned int *markBegin = (unsigned int *) ptr;
	*markBegin = 0xa1a2a3a4;

    unsigned int *markEnd = (unsigned int *) ADDPTR(ptr, neededSize-postfix);
 	*markEnd = 0xf1f2f3f4;

	// increse ptr behound the magic marker
	ptr = ADDPTR(ptr, prefix);

	memset(ptr, 0xa5, size);		// un-initialized poison
	
	traceMap[ptr] = new TraceItem(size, neededSize, file, line, type);
    DBERR("MEMORY allocation at 0x%08X (%s) of %u bytes from %s:%u\n", ptr, type, size, file, line);
    return ptr;
}

static int unrelatedDeallocations = 0;

/*
 * deallocate memory, checking the surrounding by 8 guard-bytes
 */
inline void deallocate(void *ptr, const char *type)
{
	// printing debug info of unrelated deallocations will cause a new unrelated deallocation! (from Debug.cpp)
	
    traceMapType::iterator iter = traceMap.find(ptr);

    if (iter != traceMap.end())
    {
    	//there was indeed memory allocated at that location
    	TraceItem *i = iter->second;
	    // compare type to i->type to catch mis-matched alloc/frees!
	    
		// restore the ptr to it's actual value
		ptr = SUBPTR(ptr, prefix);	    
		CheckUnused(ptr, i);	    
	    TraceCheckInternal(ptr, i);

	    // todo: add support for 2x free'd allocations?
	    // this could be done by keeping the entry and marking it "deleted")
	    // note: the same entry could be re-allocated, and the entry should be removed from the
	    // deleted-list when that happens.

	    //DBERR("MEMORY free at 0x%08X (%s) of %u bytes allocated from %s:%u\n", ptr, i->type.c_str(), i->size, i->file.c_str(), i->line);
	    memset(ptr, 0xa7, i->size);		// free poison

	    traceMap.erase(iter);
    }
    else
    {
	    unrelatedDeallocations++;
	}
	// if the entry was not in the map, it was from the Debug.cpp or stdlib so still free it!
	free(ptr);
}

void * operator new(unsigned int size, const char *file, int line)
{
    return allocate(size, file, line, "block");
}

void operator delete(void *p)
{
    deallocate(p, "block");
}

void * operator new[](unsigned int size, const char *file, int line)
{
    return allocate(size, file, line, "array");
}

void operator delete[](void *p)
{
	deallocate(p, "array");
}


void CheckSanity()
{
	//DBERR("CheckSanity()\n");
	try {
  		for(traceMapType::iterator iter = traceMap.begin(); iter != traceMap.end(); iter++)
  		{
  			void *ptr = (void *) iter->first;
  			TraceCheck(ptr);
  		}
	}
	catch (...)
	{
		DBERR("CheckSanity exception!\n");
	}
}

void DumpReport()
{
	DBERR("\n============================\n");
 	DBERR("== TRACE ALLOCATOR REPORT ==\n");
	DBERR("============================\n");
  
  	DBERR("Caught %u unrelated deallocations (which is normal behaviour)\n", unrelatedDeallocations);

  	char pcFilename[260];
  	Uint32 totalSize = 0;
  	for(traceMapType::iterator iter = traceMap.begin(); iter != traceMap.end(); iter++)
  	{
  		unsigned int address = (unsigned int) iter->first;
  		TraceItem *i= iter->second;
    	sprintf(pcFilename, "%s:%u", i->file.c_str(), i->line);
    	DBERR("%-35s at 0x%08X: %s of %d bytes unfreed\n", pcFilename, address, i->type.c_str(), i->size);
    	totalSize += i->size;
  	}
  	DBERR("-----------------------------------------------------------\n");
	DBERR("A total of %d bytes was allocated but not freed\n", totalSize);

  	DBERR("\nInformation about memory usage:\n");

  	for(traceListType::iterator iter = unusedList.begin(); iter != unusedList.end(); iter++) {
		TraceItem *i = *iter;
    	sprintf(pcFilename, "%s:%u", i->file.c_str(), i->line);
    	DBERR("%-35s: %s of %d bytes unused\n", pcFilename, i->type.c_str(), i->size);
	}

	DBERR("== End of TraceAllocator information ==\n");
  
  	// delete the map and list
  	while (!traceMap.empty()) {
		delete traceMap.begin()->second;
		traceMap.erase(traceMap.begin());
	}

	while(!unusedList.empty()) {
		delete unusedList.back();
		unusedList.pop_back();
	}  
}

#endif //don't compile this class at all if traceAllocations are off
