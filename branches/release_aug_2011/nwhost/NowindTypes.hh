//! NowindTypes.h
#ifndef NOWIND_TYPES_H
#define NOWIND_TYPES_H

namespace nwhost {

typedef unsigned char byte;
typedef unsigned int word;

// attributes 
enum nw_attribute { attr_none = 0, enable_phantom_drives, allow_other_diskroms, enable_dos2 };

} // namespace nowind

#endif // NOWIND_TYPES_H
