//! msxtypes.h
#ifndef MSXTYPES_H
#define MSXTYPES_H

#include "SDL_types.h"

typedef unsigned char nw_byte;          // should be (at least?) an 8 bit type
typedef unsigned int nw_word;           // should be (at least?) an 16 bit type
    
typedef unsigned long long emuTimeType;
typedef unsigned long long msTimeType;

#endif

// todo: use SDL types for uint16/uint32 only when interfacing with SDL
//       and keep the SDL types from spreading throughout isolated classes with dedicated functions.
