/* 	
 * Media.cpp: Insert media (disks/roms etc... into Nowind 
 *
 * Copyright (C) 2004 Jan Wilmans <jw@dds.nl>
 *                    Aaldert Dekker <a.dekker@student.tue.nl>
 */

#define UCase(s) std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) toupper)
#define LCase(s) std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) tolower)
 
#include "Media.h"
#include "Debug.h"
#include "DiskInterface.h"
#include "cpu/Z80.h"
#include <algorithm>

using namespace std;

Media * Media::Instance() {

	/* implies singleton class */
	static Media deInstantie;
	return &deInstantie;
}


Media::Media() {
    /* use DEBUGERROR in the Media class because it needs to be created before the 
       emulator is properly initialized */
}

Media::~Media() {

}

/* 
 * Loads a file, this method try's to "guess" what to do with this file :)
 *  *.dsk => load as disk-image
 *  *.rom => load as rom
 *  A full pathname should be specified!
 * 
 *  The priority gives a clue about where to load the media (
 *  (for disks priority 0 = A:, 1 = B:)
 *  (for roms priority 0 = start adress 0, 1 = $4000, etc...
 */
void Media::insertMedia(string filename, unsigned int priority) {

    if (priority != 0) priority = 1;

    string extention = filename.substr(filename.size()-3);
    UCase(extention);
    
    if (extention == "DSK") { 
        // disk
        DiskInterface::Instance()->insertDisk(priority,filename);	
    } else if (extention == "ROM") {
        unsigned long fileSize = 0;
        fstream * fs = new fstream(filename.c_str(), ios::in | ios::binary);
        if (!fs->fail()) {
       		fs->seekg(0, ios::end);
       		fileSize = fs->tellg();
   	    	fs->close();
        } else {
			DBERR("Could not open rom: %s\n", filename.c_str());
        }    

    	delete fs;
        
/*          	
   		unsigned int thePages = fileSize/(16*1024);
   		if (thePages > 3)
   		   Z80::Instance()->loadRom(filename,1+priority,0,0,4);
        else {
            if (thePages == 0) {
                DBERR("Warning: rom to small, might be incomplete..\n");
                thePages = 1;
       	    }    	    
       	    Z80::Instance()->loadRom(filename,1+priority,0,1,thePages);
        }
        */
    } else {
		DBERR("Extention unknown: %s\n", extention.c_str());
    }     
}


void Media::insertNowindImage(string filename, Uint32 size) {

    string extention = filename.substr(filename.size()-3);
    UCase(extention);

    char buf[512];
    for (Uint32 i=0;i<512;i++) buf[i] = 0;
    
    fstream * fs = new fstream(filename.c_str(), ios::out | ios::binary); //ios::trunc
    if (!fs->fail()) {
   		fs->seekp(size-512, ios::beg);
   		fs->write(buf, 512);

		/* boot sector */
   		fs->seekp(0, ios::beg);
	    for (Uint32 i=0;i<512;i++) buf[i] = 0;
	    buf[0] = 0xEB;
	    buf[1] = 0xFE;
	    buf[2] = 0x90;
	    buf[0x15] = 0xF9;
	    buf[0x1E] = 0xC9;
		fs->write(buf, 512);

		/* fat directory */
   		fs->seekp(0x200, ios::beg);
	    for (Uint32 i=0;i<512;i++) buf[i] = 0;
	    buf[0] = 0xF9;
	    buf[1] = 0xFF;
	    buf[2] = 0xFF;
		fs->write(buf, 512);
		
    	fs->close();
	} else {
		DBERR("insertNowindImage fout!\n");	
	}
    //DiskInterface::Instance()->insertDisk(0, filename);	
}

/* 
 */
void Media::loadRom(Uint32 diskNumber, string filename) {
		
		assert(false);		
}

/* 
 * Opens a file (readonly) in a path relative to the nowind-executable
 */
fstream * Media::openfile(string filename, NW_OPENMODE mode) {

        string fullpath = Debug::Instance()->getPath() + filename;
        fstream *fs = new fstream(fullpath.c_str(), mode);
        if (fs->fail()) {
            DEBUGERROR(fullpath << " not found!\n");
			assert(false);
        }    
		return fs;
}
