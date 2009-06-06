//! NowindTypes.h
#ifndef NOWIND_TYPES_H
#define NOWIND_TYPES_H

namespace nwhost {

typedef unsigned char byte;
typedef unsigned int word;

// attributes 
//enum at_attributes { attr_none = 0, attr_ctrl, attr_shift, attr_dos2 };

// commands 
enum at_commands { cmd_none = 0, cmd_putmem_clear, cmd_putmem };

// attributes 
enum nw_attribute { attr_none = 0, enable_phantom_drives, allow_other_diskroms, enable_dos2 };

} // namespace nowind

#endif // NOWIND_TYPES_H
