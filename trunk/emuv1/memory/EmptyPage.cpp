// EmptyPage.cpp
#include "EmptyPage.h"
#include "Debug.h" 
#include "cpu/Z80.h" 

using namespace std;

EmptyPage::EmptyPage() {

    DBERR("EmptyPage Constructor...\n");

    idString = "Empty";
    offset = 0;	
    
     /* create a dummy read-page, used by all empty pages in the slotlayout */
     nw_byte * newMem = new nw_byte[16*1024];
     memset((nw_byte *) newMem, 0xff, sizeof(nw_byte)*16*1024); // fill entire 16K block with 0's
     memory = newMem;
     dummyReadPage = newMem;
     releaseMemory = true;

     // store a pointer to this object in a static variable (in MemoryDevice)
     emptyPage = this;          
     
     DBERR("--emptyPage: 0x%08X\n",emptyPage);
}

nw_byte EmptyPage::read(nw_word address) {
	
	assert(false);                         //should not used, maybe for future use.
	return memory[address-offset];
}

void EmptyPage::write(nw_word, nw_byte) {
    
    /* ROM, so no write() */
    //DBERR("EmptyPage:write 0x%04X = 0x%02X\n", address, value);
}

void EmptyPage::install(layoutType, Uint8, Uint8, nw_word) {
	
	assert(false); // its useless to install an empty page anywhere...
}

void EmptyPage::activate(unsigned int theBlock) {

//    DBERR("EmptyPage::activate for block " << theBlock << endl);

    assert(offset != 0xFFFFFFFF);

    Z80::Instance()->readBlock[theBlock] = dummyReadPage;
    Z80::Instance()->writeFunc[theBlock].bind(this, &EmptyPage::write);
    
    // for debugging info on memoryDevice names
    slotSelector->setPage(theBlock >> 1, this);    
}


