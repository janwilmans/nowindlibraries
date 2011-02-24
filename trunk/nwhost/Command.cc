#include "Command.hh"
#include "NowindHostSupport.hh"

#include <cassert>
#include <string>
#include <vector>

#define DBERR nwhSupport->debugMessage

namespace nwhost {

using std::string;
using std::vector;
using std::fstream;
using std::ios;

Command::Command()
{
}

void Command::initialize(NowindHostSupport* aSupport)
{
    nwhSupport = aSupport;
}

Command::~Command()
{
}


void trim(string& str)
{
  string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}

string Command::getFilenameFromExtraData()
{
 	string whole;
	for (int i = 1; i < 13; ++i) {
		char c = extraData[i];
        whole += toupper(c);
	}
	string file = whole.substr(0, 8);
	trim(file);
    string filename = file;
    string ext = whole.substr(8, 3);
    trim(ext);
    if (!ext.empty())
    {
        filename += "." + ext;
    }
    return filename;
}


void Command::reportCpuInfo()
{
//                                   01234567    8
//	byte cmdData[9];         // reg_[cbedlhfa] + cmd
//	byte extraData[240 + 2]; // extra data for image/message/write

    word reg_bc = cmdData[0] + 256*cmdData[1];
    word reg_de = cmdData[2] + 256*cmdData[3];
    word reg_hl = cmdData[4] + 256*cmdData[5];
    word reg_af = cmdData[6] + 256*cmdData[7];

    word reg_ix = extraData[0] + 256*extraData[1];
    word reg_iy = extraData[2] + 256*extraData[3];
    word reg_sp = extraData[4] + 256*extraData[5];
    reg_sp += 6;

    byte mainSS = extraData[6];
    word fcc5 = extraData[7] + 256*extraData[8];
    word fcc7 = extraData[9] + 256*extraData[10];
    word reg_pc = extraData[11] + 256*extraData[12];
    
    DBERR("PC:%04X AF:%04X BC:%04X DE:%04X HL:%04X IX:%04X IY:%04X S:%04X\n", \
        reg_pc, reg_af, reg_bc, reg_de, reg_hl, reg_ix, reg_iy, reg_sp);
    
    /*
    // stack dump    
    for (int i=0; i<16; i++)
    {
        DBERR("  0x%04X: 0x%04x\n", reg_sp, extraData[13+(i*2)] + 256*extraData[14+(i*2)]);
        reg_sp += 2;
    }
    */
}


} // namespace nwhost
