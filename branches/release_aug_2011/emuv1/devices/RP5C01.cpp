// RP5C01.cc
// Real Time Clock RP5C01

#include <iostream>
#include <time.h>
#include "math.h"
#include "stdio.h"

#include "RP5C01.h"
#include "Debug.h"

using namespace std;

RP5C01::RP5C01() {

		DBERR("RP5C01 constructor...\n");

		// load RTC state
		rtcFilename = Debug::Instance()->getPath() + "RTC.state";
		
		ifstream ifs(rtcFilename.c_str(),ios::in|ios::binary);
		if (ifs.fail()) {
			DBERR("Error loading file RTC.state!\n");
		} else {
		    // create new buffer to ensure that the native datatype is converted to chars
		    char * buffer = new char[13*3];       
			ifs.read(buffer,13*3);
			ifs.close();
			for (int i=0;i<13;i++) {
                for (int j=0;j<3;j++) dataRegister[i][j] = buffer[i+(13*j)] & 0x0f;
            }
            delete [] buffer;  
			
		}
		modeRegister = 0xFF;

		DBERR("RP5C01 constructor...finished\n");
}

RP5C01::~RP5C01() {

		// save RTC state
		ofstream ofs(rtcFilename.c_str(),ios::binary|ios::trunc);
		if (ofs.fail()) {
			DBERR("Error writing file RTC.state!\n");
		} else {
		    // create new buffer to ensure that the native datatype is converted to chars		    
            char * buffer = new char[13*3];
			for (int i=0;i<13;i++) {
			     for (int j=0;j<3;j++) buffer[i+(13*j)] = dataRegister[i][j] & 0x0f;
            }    
			ofs.write(buffer,13*3);
			ofs.close();
            delete [] buffer;			
		}
		DBERR("RP5C01 destroyed.\n");
}

void RP5C01::writePortB4(nw_byte value) {
		indexRegister = value & 0x0F;
}

void RP5C01::writePortB5(nw_byte value) {

		value &= 0x0F;
		switch(indexRegister) {
		case 0x0D: modeRegister = value&0x03; break; //Timer EN and Alarm EN not supported
		case 0x0E: break;
		case 0x0F:
			// reset alarm registers
			if (value&1) for (int i=0; i<0x0D; i++) dataRegister[i][ALARMBLOCK] = 0;
			break;
		default:
			//TODO kan leapyear (reg B alarmblock) wel geschreven worden?
			if (modeRegister) dataRegister[indexRegister][modeRegister-1] = value;
			break;
		}
}

nw_byte RP5C01::readPortB5() {

//	   return 0xff;
       time_t td = time(NULL);
	   struct tm *tm = localtime(&td);

	   nw_byte r = 0xff;
	   if (indexRegister < 0x0D) {
			if (modeRegister) {
				if ((modeRegister==1)&&(indexRegister==0xB)) {
					r = tm->tm_year%4;
				} else {
					r = dataRegister[indexRegister][modeRegister-1];
				}
			}
			else {
				nw_byte hour = tm->tm_hour;
				// 12/24 hour mode
				if (!(dataRegister[0x0A][0]&1)) {
					if (hour>11) hour = (hour - 12) + 20;
				}

				switch(indexRegister) {
				case 0: r = tm->tm_sec%10; break;
				case 1: r = tm->tm_sec/10; break;
				case 2: r = tm->tm_min%10; break;
				case 3: r = tm->tm_min/10; break;
				case 4: r = hour%10; break;
				case 5: r = hour/10; break;
				case 6: r = tm->tm_wday; break;
				case 7: r = (tm->tm_mday)%10; break;
				case 8: r = (tm->tm_mday)/10; break;
				case 9: r = (tm->tm_mon+1)%10; break;
				case 10: r = (tm->tm_mon+1)/10; break;
				case 11: r = (tm->tm_year-80)%10; break;
				case 12: r = (tm->tm_year-80)/10; break;
				default:
					break;
				}
			}
		} else {
			// register E and F are write only
			if (indexRegister==0x0D) r = modeRegister;
			else r = 0xFF;
		}
        return r;
}
