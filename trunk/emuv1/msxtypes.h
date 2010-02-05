//! msxtypes.h
#ifndef MSXTYPES_H
#define MSXTYPES_H

#include "SDL_types.h"

typedef unsigned char nw_byte;          // should be (at least?) an 8 bit type
typedef unsigned int nw_word;           // should be (at least?) an 16 bit type
    
typedef unsigned int emuTimeType;
typedef unsigned int msTimeType;

#endif

// todo: use SDL types for uint16/uint32 through out the emulator
//       instead of unsigned int or unsigned long
