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
	extraData.resize(240 + 2);
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

string Command::getFilenameFromExtraData() const
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

void Command::reportCpuInfo() const
{
    word reg_bc = getBC();
    word reg_de = getDE();
    word reg_hl = getHL();
    word reg_af = getAF();

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

void Command::reportFCB(const unsigned char* data) const
{
 	string whole;
	for (int i = 0; i < 37; ++i) {
		char c = data[i];
        whole += c;
	}

    byte drive = data[0];
    string filename = whole.substr(1,11);
    byte extent = data[12];
    byte fileattr = data[13];
    byte extentHigh = data[14];
    byte recordCountInExtent = data[15];
    word recordSize = 256 * recordCountInExtent + extentHigh;
    unsigned int filesize = data[16] + (data[17] << 8) + (data[18] << 16) + (data[19] << 24);
    byte date1 = data[20];
    byte date2 = data[21];
    byte time1 = data[22];
    byte time2 = data[23];
    byte deviceCode = data[24];
    byte dirEntryNr = data[25];
    word startCluster = data[26] + (data[27] << 8);
    word curCluster = data[28] + (data[29] << 8);
    word curRelCluster = data[30] + (data[31] << 8);
    byte recordInExtent = data[32];
    unsigned int randomAccessRecord = data[33] + (data[34] << 8) + (data[35] << 16);     
    byte randonAccessRecordSmaller64 = data[36];
            
    DBERR("+0   Drive:      %u\n", drive);
    DBERR("+1,8 Filename:   %s\n", filename.c_str());
    DBERR("+12  extent:     %u\n", extent);
    DBERR("+13  fileattr:   0x%02x\n", fileattr);
    DBERR("+14  extentHigh: %u\n", extentHigh);
    DBERR("+15  recordCountInExtent:   %u\n", recordCountInExtent);
    DBERR("     recordSize: %u\n", recordSize);
    DBERR("+16  filesize:   %u\n", filesize);
    DBERR("+20  date1:      %u\n", date1);
    DBERR("+21  date2:      %u\n", date2);
    DBERR("+22  time1:      %u\n", time1);
    DBERR("+23  time2:      %u\n", time2);
    DBERR("+24  deviceCode: %u\n", deviceCode);
    DBERR("+25  dirEntryNr: %u\n", dirEntryNr);
    DBERR("+26,2  startCluster:   %u\n", startCluster);
    DBERR("+28,2  curCluster:     %u\n", curCluster);
    DBERR("+30,2  curRelCluster:  %u\n", curRelCluster);
    DBERR("+32,2  recordInExtent: %u\n", recordInExtent);
    DBERR("+33,3  randomAccessRecord:            %u\n", randomAccessRecord);
    DBERR("+36,3  randonAccessRecordSmaller64:   %u\n", randonAccessRecordSmaller64);
}


} // namespace nwhost
