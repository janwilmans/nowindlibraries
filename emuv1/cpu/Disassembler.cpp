// Disassembler.ccp

#include "stdio.h"
#include "cpu/Z80.h"
#include "cpu/Disassembler.h"
#include "osd/Command.h"
#include "Debug.h"

using namespace std;

// TODO: INC IXh is disassembled as INC H, check all instructions with IXh/l and IYh/l

Disassembler::Disassembler() {

		DBERR("Disassembler constructor...\n");

		// constructor
		string filename("disassembler.html");

		logfile = new ofstream(filename.c_str(), ios::trunc);
		if (logfile->fail()) {
			DBERR("Error opening file %s\n", filename.c_str());
		}
	   	(*logfile) << "<html><body><basefont color=\"black\" face=\"courier\" size=\"4\">" << endl;

		startByProgramCounter = false;
		startByEmuTime = false;
		lineNumber = 0;

        filename = "MSXBIOS.MLL";
        fstream * fs = new fstream(filename.c_str(), ios::in | ios::binary);
        string str_addr = "";
        string symbol_name = "";
        nw_word symbol_address = 0;
        if (!fs->fail()) {
       	    while (!fs->eof()) {
           	    // read file
                *fs >> symbol_name; 
                *fs >> str_addr; 
                *fs >> str_addr; 
                symbol_address = Command::hex2Word(str_addr.substr(2));
           	    symbolnames[symbol_address] = symbol_name;
           	    //DBERR("symbol_address: " << symbol_address << " symbol_name:  " << symbolnames[symbol_address] << endl);
            }       	
   	    	fs->close();
        } else {
			DBERR("Could not open file: %s\n", filename.c_str());
        }    
		delete fs;
	   	z80 = Z80::Instance();
		DBERR("Disassembler constructor...finished\n");
}

Disassembler *Disassembler::Instance() {

		/* implies singleton class */

		static Disassembler deInstantie;
		return &deInstantie;
}

Disassembler::~Disassembler() {
		// destructor
		delete logfile;
		DBERR("Disassembler destroyed.\n");
}

char * Disassembler::disAsm(nw_word pc, nw_word w1, nw_word w2) {

	char *sstr;
	char buf[20], buf1[10];
	nw_byte nb1 = 0;
	nw_byte nb2 = 0;
	size = 1;

	switch (w1&0xFF) {
	case 0xCB: size++; strcpy(mstr, &mnemonicCB[w1>>8][0]); nb1=w2&0xFF; nb2=w2>>8; break;
	case 0xED: size++; strcpy(mstr, &mnemonicED[w1>>8][0]); nb1=w2&0xFF; nb2=w2>>8; break;
	case 0xDD:
	case 0xFD:
		size++;
		switch(w1>>8) {
		case 0xCB: nb1=w2&0xFF; strcpy(mstr, &mnemonicCB[w2>>8][0]); break;
		case 0xED: nb1=w2&0xFF; strcpy(mstr, &mnemonicED[w2>>8][0]); break;
		default: strcpy(mstr, &mnemonic[w1>>8][0]); nb1=w2&0xFF; nb2=w2>>8; break;
		}
		break;
	default: strcpy(mstr, &mnemonic[w1&0xFF][0]); nb1=w1>>8; nb2=w2&0xFF;
	}
	sstr = strstr(mstr, "NN");
	if (sstr!=NULL) {
		sprintf(buf, "%02X", nb1);
		memcpy(sstr, buf, 2);
		size++;
	}
	sstr = strstr(mstr, "MMMM");
	if (sstr!=NULL) {
		sprintf(buf, "%04X", (nb2<<8)+nb1);
		memcpy(sstr, buf, 4);
		size+=2;
	}
	sstr = strstr(mstr, "EEEE");
	if (sstr!=NULL) {
		sprintf(buf, "%04X", pc+(signed char)nb1+2);
		memcpy(sstr, buf, 4);
		size++;
	}
	sstr = strstr(mstr, "Y");
	if (sstr!=NULL) {
		strcpy(buf, sstr+1);
		switch (w1&0xFF) {
		case 0xDD: strcpy(sstr, "ix"); break;
		case 0xFD: strcpy(sstr, "iy"); break;
		default: strcpy(sstr, "hl");
		}
		strcat(mstr, buf);
	}
	sstr = strstr(mstr, "R");
	if (sstr!=NULL) {
		strcpy(buf, sstr+1);
		switch (w1&0xFF) {
		case 0xDD: strcpy(sstr, "ix"); break;
		case 0xFD: strcpy(sstr, "iy"); break;
		default: strcpy(sstr, "hl");break;
		}
		strcat(mstr, buf);
	}
	sstr = strstr(mstr, "X");
	if (sstr!=NULL) {
		strcpy(buf, sstr+1);
		switch (w1&0xFF) {
		case 0xDD:
			sprintf(buf1, "ix+%02X", nb1);
			strcpy(sstr, buf1);
			size++;
			break;
		case 0xFD:
			sprintf(buf1, "iy+%02X", nb1);
			strcpy(sstr, buf1);
			size++;
			break;
		default: strcpy(sstr, "hl");break;
		}
		strcat(mstr, buf);
	}
	return mstr;
}

int Disassembler::getSize() {
	return size;
}

void Disassembler::setDisassemblyPC(nw_word pc, unsigned int instructionCount) {

	startDisassbleAddress = pc;
	instructionsLeftToDisassble = instructionCount;
	startByProgramCounter = true;

}

void Disassembler::setDisassemblyEmuTime(emuTimeType emuTime, unsigned int instructionCount) {

	startDisassbleEmuTime = emuTime;
	instructionsLeftToDisassble = instructionCount;
	startByEmuTime = true;

}

void Disassembler::setDisassemblyPreview(emuTimeType emuTime) {

	/* this method should be called before the instruction is executed ! */

	reg_pc = z80->reg_pc;
	reg_sp = z80->reg_sp;
	register_af = (z80->reg_a<<8)|z80->reg_f;
	register_bc = (z80->reg_b<<8)|z80->reg_c;
	register_de = z80->reg_de;
	register_hl = z80->reg_hl;
    register_ix = z80->reg_ix;
    register_iy = z80->reg_iy;
	shadow_af = z80->shadow_af;
	shadow_bc = (z80->shadow_b<<8)|z80->shadow_c;
	shadow_de = z80->shadow_de;
	shadow_hl = z80->shadow_hl;

	previewWord1 = z80->readMem16Public(reg_pc);
	previewWord2 = z80->readMem16Public(reg_pc+2);
	lastEmuTime = z80->emuTime;

	if (startByProgramCounter && (reg_pc == startDisassbleAddress)) {
		disassembling = true;
	}

	if (startByEmuTime && (emuTime > startDisassbleEmuTime)) {
		disassembling = true;
	}

	if (disassembling) {
		++lineNumber;
		DBERR("Disassembler line: %u\n", lineNumber);
	}
}

void Disassembler::triggerDisassembler() {

	/* this method should be called after the instruction is executed ! */

	if (disassembling) {
		if (instructionsLeftToDisassble > 0) {
			instructionsLeftToDisassble--;

			char str[255];
			sprintf(str,"%05i:<br> ",int(lineNumber));
			(*logfile) << string(str) << endl;

			sprintf(str,"<b>%04X: ",reg_pc);
			(*logfile) << string(str) << string(disAsm(reg_pc, previewWord1, previewWord2));

			sprintf(str," [%02X] [%02X] [%02X] [%02X]",previewWord1 & 255, previewWord1 >> 8, previewWord2 & 255, previewWord2 >> 8);
			
			string opcodes(str);
			int size = getSize() * 5;
			(*logfile) << opcodes.substr(0,size) << endl;
			
			(*logfile) << "</b><br>" << endl;
			(*logfile) << "SP:"; logRegisterColored(reg_sp,z80->reg_sp);
			(*logfile) << " AF:"; logRegisterColored(register_af,(z80->reg_a<<8)|z80->reg_f);
			(*logfile) << " BC:"; logRegisterColored(register_bc,(z80->reg_b<<8)|z80->reg_c);
			(*logfile) << " DE:"; logRegisterColored(register_de,z80->reg_de);
			(*logfile) << " HL:"; logRegisterColored(register_hl,z80->reg_hl);
            (*logfile) << " IX:"; logRegisterColored(register_ix,z80->reg_ix);
            (*logfile) << " IY:"; logRegisterColored(register_iy,z80->reg_iy);
			(*logfile) << " F:";

			if (z80->reg_f & SFLAG) (*logfile) << ("s"); else (*logfile) << (".");
			if (z80->reg_f & ZFLAG) (*logfile) << ("z"); else (*logfile) << (".");
			if (z80->reg_f & 0x20) (*logfile) << ("1"); else (*logfile) << ("0");
			if (z80->reg_f & HFLAG) (*logfile) << ("h"); else (*logfile) << (".");
			if (z80->reg_f & 0x08) (*logfile) << ("1"); else (*logfile) << ("0");
			if (z80->reg_f & PFLAG) (*logfile) << ("p"); else (*logfile) << (".");
			if (z80->reg_f & NFLAG) (*logfile) << ("n"); else (*logfile) << (".");
			if (z80->reg_f & CFLAG) (*logfile) << ("c"); else (*logfile) << (".");
			(*logfile) << "(" <<dec << (z80->emuTime-lastEmuTime) << ")";
			(*logfile) << "<br>" << endl;
			(*logfile) << "<br>" << endl;

		} else {
				disassembling = false;
				(*logfile) << "</body></html>" << endl;
		}

	}

}

string Disassembler::recallInstruction() {

	return string(disAsm(reg_pc, previewWord1, previewWord2));
	
}

void Disassembler::logRegisterColored(nw_word firstReg, nw_word secordReg) {

	char byte1[250];
	char byte2[250];
	sprintf(byte1,"%02X",secordReg >> 8);
	sprintf(byte2,"%02X",secordReg & 255);

	if ((firstReg >> 8) == (secordReg >> 8)) {
		(*logfile) << string(byte1);
	} else {
		(*logfile) << "<font color=\"red\">" << string(byte1) << "</font>";
	}

	if ((firstReg & 255) == (secordReg & 255)) {
		(*logfile) << string(byte2);
	} else {
		(*logfile) << "<font color=\"red\">" << string(byte2) << "</font>";
	}

}

string Disassembler::disAsm(nw_word *pc_pnt, nw_word w1, nw_word w2, bool hotinfo) {

    nw_word pc = *pc_pnt;
	char *sstr;
	char buf[20], buf1[10];
	nw_byte nb1 = 0;
	nw_byte nb2 = 0;
	size = 1;
	char mstr[40];

	switch (w1&0xFF) {
	case 0xCB: size++; strcpy(mstr, &mnemonicCB[w1>>8][0]); nb1=w2&0xFF; nb2=w2>>8; break;
	case 0xED: size++; strcpy(mstr, &mnemonicED[w1>>8][0]); nb1=w2&0xFF; nb2=w2>>8; break;
	case 0xDD:
	case 0xFD:
		size++;
		switch(w1>>8) {
		case 0xCB: nb1=w2&0xFF; strcpy(mstr, &mnemonicCB[w2>>8][0]); break;
		case 0xED: nb1=w2&0xFF; strcpy(mstr, &mnemonicED[w2>>8][0]); break;
		default: strcpy(mstr, &mnemonic[w1>>8][0]); nb1=w2&0xFF; nb2=w2>>8; break;
		}
		break;
	default: strcpy(mstr, &mnemonic[w1&0xFF][0]); nb1=w1>>8; nb2=w2&0xFF;
	}
	sstr = strstr(mstr, "NN");
	if (sstr!=NULL) {
		sprintf(buf, "%02X", nb1);            // prefix $ !
		memcpy(sstr, buf, 2);
		size++;
	}
	sstr = strstr(mstr, "MMMM");
	if (sstr!=NULL) {
		sprintf(buf, "%04X", (nb2<<8)+nb1);
		memcpy(sstr, buf, 4);
		size+=2;
	}
	sstr = strstr(mstr, "EEEE");
	if (sstr!=NULL) {
		sprintf(buf, "%04X", pc+(signed char)nb1+2);
		memcpy(sstr, buf, 4);
		size++;
	}
	sstr = strstr(mstr, "Y");
	if (sstr!=NULL) {
		strcpy(buf, sstr+1);
		switch (w1&0xFF) {
		case 0xDD: strcpy(sstr, "ix"); break;
		case 0xFD: strcpy(sstr, "iy"); break;
		default: strcpy(sstr, "hl");
		}
		strcat(mstr, buf);
	}
	sstr = strstr(mstr, "R");
	if (sstr!=NULL) {
		strcpy(buf, sstr+1);
		switch (w1&0xFF) {
		case 0xDD: strcpy(sstr, "ix"); break;
		case 0xFD: strcpy(sstr, "iy"); break;
		default: strcpy(sstr, "hl");break;
		}
		strcat(mstr, buf);
	}
	sstr = strstr(mstr, "X");
	if (sstr!=NULL) {
		strcpy(buf, sstr+1);
		switch (w1&0xFF) {
		case 0xDD:
			sprintf(buf1, "ix+%02X", nb1);
			strcpy(sstr, buf1);
			size++;
			break;
		case 0xFD:
			sprintf(buf1, "iy+%02X", nb1);
			strcpy(sstr, buf1);
			size++;
			break;
		default: strcpy(sstr, "hl");break;
		}
		strcat(mstr, buf);
	}
	
	string retstr = string(mstr);
    while(retstr.length() < 10) retstr = retstr + " ";

    string curr_pc = symbolnames[pc];
    if (curr_pc != "") {
        retstr = retstr + "     ; " + curr_pc;
    }

	switch(w1&0xFF) {
        case 0xDB: // in a,(n)
        case 0xD3: // out (n),a
        case 0xED: // todo: beter doen!, nu hele ED groep :_)
        {        
            char temp[30];
            switch(w1>>8) {
                case 0x98:
                    retstr = retstr + "  ;vram";
                    break;
                case 0x99:
                    // only works in actual debugger, not in "dis $"
                    if (hotinfo) {
                        sprintf(temp,"%i",(w1>>8)&0x3f);
                        if (Emulator::Instance()->vdp->port1DataLatched) {
                            retstr = retstr + "  ;VDP register " + string(temp);
                        } else {
                            retstr = retstr + "  ;VDP data";
                        }
                    } else {
                        retstr = retstr + "  ;VDP control register";
                    }                        
                    break;
                case 0x9A:
                    if (hotinfo) {
                        if (Emulator::Instance()->vdp->port2DataLatched) {
                            retstr = retstr + "  ;palette data _G";
                        } else {
                            retstr = retstr + "  ;palette data RB";
                        }
                    } else {
                        retstr = retstr + "  ;palette data";
                    }                        
                case 0x9B:
                    if (hotinfo) {
                        sprintf(temp,"%i",Emulator::Instance()->vdp->vdpReg[17] & 0x3f);
                        retstr = retstr + "  ;VDP indirect access of R#" + string(temp);
                    } else {                        
                        retstr = retstr + "  ;VDP indirect access";
                    }
                    break;
                case 0x90:
                    retstr = retstr + "  ;printer busy/strobe";
                    break;
                case 0xA8:
                    retstr = retstr + "  ;MAINSLOT (ppi port A)";
                    break;
                case 0xAA:
                    retstr = retstr + "  ;keyboard scanline (ppi port ?)";
                    break;
                case 0xFC:
                case 0xFD:
                case 0xFE:
                case 0xFF:
                    retstr = retstr + "  ;mapper bank";
                    break;
                default:
                    break;
            }
            break;
        }
        case 0xC3: //jp 
        case 0xC4: //call nz,
        case 0xCC: //call z,
        case 0xCD: //call 
        case 0xD4: //call nc,
        case 0xDC: //call c,
        case 0xE4: //call po,
        case 0xEC: //call pe,
            {
                nw_word adr = 256*(w2&0xff) + (w1 >> 8);
                string curr_symbol = symbolnames[adr];
                if (curr_symbol != "") {
                    retstr = retstr + "     -> [" + symbolnames[adr] + "]";
                }
            } 
            break;
        default:
            break;
    }

	(*pc_pnt) = pc + size;
	
	return retstr;
}
