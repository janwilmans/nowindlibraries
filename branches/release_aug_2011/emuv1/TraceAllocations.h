//! TraceAllocations.h
#ifndef TRACEALLOC_H
#define TRACEALLOC_H

    void * operator new  (unsigned int size, const char *file, int line);
    void * operator new[](unsigned int size, const char *file, int line);

//    void operator delete (void *p, const char *file, int line);
    
    void operator delete  (void *p);
    void operator delete[](void *p);

    extern void TraceCheck(void *);
    extern void DumpReport();
	extern void CheckSanity();
#endif 
