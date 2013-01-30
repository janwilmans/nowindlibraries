//! Media.h
#ifndef MEDIA_H
#define MEDIA_H

#include <iostream>
#include <string>

#include "compiler.h"
#include "msxtypes.h"

/*!
 * This class handles the different Media that can be inserted, like Disks and Roms 
 */
class Media {
private:
			    Media();
public:
   
static Media    * Instance();
            	~Media();

void            insertMedia(std::string filename, Uint32 priority);
void 			insertNowindImage(std::string filename, Uint32 size);
void            loadRom(Uint32 diskNumber, std::string filename);

static std::fstream * openfile(std::string filename, NW_OPENMODE mode);
   
};

#endif
