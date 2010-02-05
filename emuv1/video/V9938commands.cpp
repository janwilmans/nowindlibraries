// V9938commands.cpp

#include "stdio.h"
#include "Debug.h"
#include "V9938commands.h"
#include "V9938.h"
#include "Emulator.h"

using namespace std;

// TODO: routine die checkt of NX==0 en afhankelijk van de screen er 256
// of 512 van maakt. Bovendien moet ny geclipt worden. Nu kan een vdpcommando
// ervoor zorgen dat er buiten het vram geschreven wordt.


V9938commands::V9938commands(V9938 *thevdp) {
	
	vram = thevdp->videoRam;
	statusRegister = thevdp->statusReg;
	modeRegister0 = thevdp->vdpReg;
}

V9938commands::~V9938commands() {

   DBERR("V9938commands destroyed.\n");
}

void V9938commands::reset() {

	DBERR("RESET V9938commands\n");
	sxl = sxh = sx = 0;
	syl = syh = sy = 0;
	dxl = dxh = dx = 0;
	dyl = dyh = dy = 0;
	nxl = nxh = nx = 0;
	nyl = nyh = ny = 0;
	color = argument = command = 0;
	borderX = 0;
}    

void V9938commands::writeCommandRegister(nw_byte commandRegister, nw_byte value) {

    switch (commandRegister) {
    case 32: sxl = value; sx = sxl + (sxh<<8); break;
    case 33: sxh = value & 1; sx = sxl + (sxh<<8); break;
    case 34: syl = value; sy = syl + (syh<<8); break;
    case 35: syh = value & 3; sy = syl + (syh<<8); break;
    case 36: dxl = value; dx = dxl + (dxh<<8); break;
    case 37: dxh = value & 1; dx = dxl + (dxh<<8); break;
    case 38: dyl = value; dy = dyl + (dyh<<8); break;
    case 39: dyh = value & 3; dy = dyl + (dyh<<8); break;
    case 40: nxl = value; nx = nxl + (nxh<<8); break;
    case 41: nxh = value & 1; nx = nxl + (nxh<<8); break;
    case 42: nyl = value; ny = nyl + (nyh<<8); break;
    case 43: nyh = value & 3; ny = nyl + (nyh<<8); break;
    case 44:
        color = value;
        switch(command>>4) {
        case 0x0b: transferLmmc(); break;
        case 0x0f: transferHmmc(); break;
        default: break;
        }
        break;
    case 45: argument = value; break;
    case 46: 
        command = value;
    	screenMode = *modeRegister0 & 0x0E;
        executeCommand();
     	break;

    default:
        break;
    }
} 

int V9938commands::getBorderCoordinate() {
	return borderX;
}

void V9938commands::executeCommand() {

	// TODO: nieuwe commando negeren als er nog een bezig is???
	// TODO: check screen 5 tot 8

/*
	bool debuggenkreng = true;

    switch (command>>4) {
	
    case 0x00: DBERR("vdpCommand: STOP "); break;
	case 0x04: DBERR("vdpCommand: POIN "); break;
	case 0x05: DBERR("vdpCommand: PSET "); break;
	case 0x06: DBERR("vdpCommand: SRCH "); break;
	case 0x07: DBERR("vdpCommand: LINE "); break;
	case 0x08: DBERR("vdpCommand: LMMV "); break;
	case 0x09: DBERR("vdpCommand: LMMM "); break;
	case 0x0a: DBERR("vdpCommand: LMCM "); break;
	case 0x0b: DBERR("vdpCommand: LMMC "); break;
	case 0x0c: DBERR("vdpCommand: HMMV "); break;
	case 0x0d: DBERR("vdpCommand: HMMM "); break;
	case 0x0e: DBERR("vdpCommand: YMMM "); break;
	case 0x0f: DBERR("vdpCommand: HMMC "); break;

	default: debuggenkreng = false;
        break;
    }
	if (debuggenkreng) {
	    DBERR("-> dx:%3i dy:%3i sx:%3i sy:%3i nx:%3i ny:%3i color:%3i ARG:%i vdp0&0x0e: %i\n",dx,dy,sx,sy,nx,ny,color,argument, (int)screenMode);
    }
*/

	statusRegister[2]|=0x01;    //set CE
	switch (command >> 4) {
	case 0x00: commandDone(); break;
	case 0x04: commandPoint(); break;
	case 0x05: commandPset(); break;
	case 0x06: commandSearch(); break;
	case 0x07: commandLine(); break;
	case 0x08: commandLmmv(); break;
	case 0x09: commandLmmm(); break;
	case 0x0a: commandLmcm(); break;
	case 0x0b: commandLmmc(); break;
	case 0x0c: commandHmmv(); break;
	case 0x0d: commandHmmm(); break;
	case 0x0e: commandYmmm(); break;
	case 0x0f: commandHmmc(); break;

	default:
		DBERR("Invalid vdp command!\n");
		commandDone();
		break;
	}
	
//	Emulator::Instance()->setPause(true);

}

inline nw_byte V9938commands::pointG4(int x, int y) {
	return (x&0x01) ? vram[(y<<7)+(x>>1)]&0x0F : vram[(y<<7)+(x>>1)]>>4;
}

inline nw_byte V9938commands::pointG5(int x, int y) {
    nw_byte vb = vram[(y<<7) + (x>>2)];
    switch (x & 0x03) {
	case 0: return vb >> 6; break;
	case 1: return (vb>>4) & 0x03; break;
	case 2: return (vb>>2) & 0x03; break;
	default: return vb & 0x03; break;
	}
}

inline nw_byte V9938commands::pointG6(int x, int y) {
	unsigned int addr = (y<<8) + (x>>1);
	addr = (addr>>1)|((addr&1)<<16);
    return (x&0x01) ? vram[addr]&0x0F : vram[addr]>>4;
}

inline nw_byte V9938commands::pointG7(int x, int y) {
	unsigned int addr = (y<<8) + x;
	addr = (addr>>1)|((addr&1)<<16);
 	return vram[addr];
}

void V9938commands::commandPoint() {

	switch (screenMode) {
	case GRAPHICMODE4: color = statusRegister[7] = pointG4(sxl, sy); break;
	case GRAPHICMODE5: color = statusRegister[7] = pointG5(sx, sy); break;
	case GRAPHICMODE6: color = statusRegister[7] = pointG6(sx, sy); break;
	case GRAPHICMODE7: color = statusRegister[7] = pointG7(sxl, sy); break;
	default:
		break;
	}
	commandDone();
}

inline void V9938commands::psetG4(int x, int y, nw_byte cl) {
	nw_byte s = ((~x)&1)<<2;
	logicalOperation((y<<7)+(x>>1), (cl&0x0F)<<s, ~(0x0F<<s));
}

inline void V9938commands::psetG5(int x, int y, nw_byte cl) {
	nw_byte s = ((~x)&3)<<1;
	logicalOperation((y<<7)+(x>>2), (cl&0x03)<<s, ~(0x03<<s));
}

inline void V9938commands::psetG6(int x, int y, nw_byte cl) {
	unsigned int addr = (y<<8)+(x>>1);
	addr = (addr>>1)|((addr&1)<<16);
    nw_byte s = ((~x)&1)<<2;
	logicalOperation(addr, (cl&0x0F)<<s, ~(0x0F<<s));
}

inline void V9938commands::psetG7(int x, int y, nw_byte cl) {
	unsigned int addr = (y<<8) + x;
	addr = (addr>>1)|((addr&1)<<16);
 	logicalOperation(addr, cl, 0);
}

void V9938commands::commandPset() {

	switch (screenMode) {
	case GRAPHICMODE4: psetG4(dx, dy, color); break;
	case GRAPHICMODE5: psetG5(dx, dy, color); break;
	case GRAPHICMODE6: psetG6(dx, dy, color); break;
	case GRAPHICMODE7: psetG7(dx, dy, color); break;
	default:
		break;
	}
	commandDone();
}

void V9938commands::logicalOperation(int addr, nw_byte cl, nw_byte mask) {

    // DBERR("logOp addr: %i color: %i mask: %i\n", addr, cl, mask);
	addr &= 0x1ffff; // TODO: soms gaat addr over de kop, misschien in de commands zelf checken???

    switch (command&0x0F) {
	case IMP: vram[addr] = (vram[addr]&mask)|cl; break;
	case AND: vram[addr] &= (cl|mask); break;
	case OR:  vram[addr] |= cl; break;
	case EOR: vram[addr] ^= cl; break;
	case NOT: vram[addr] = (vram[addr]&mask)|~(cl|mask); break;

	case TIMP: if (cl) vram[addr] = (vram[addr]&mask)|cl; break;
	case TAND: if (cl) vram[addr] &= (cl|mask); break;
	case TOR:  if (cl) vram[addr] |= cl; break;
	case TEOR: if (cl) vram[addr] ^= cl; break;
	case TNOT: if (cl) vram[addr] = (vram[addr]&mask)|~(cl|mask); break;

	default:
		break;    // invalid logical operation (do nothing)
	}
}

void V9938commands::commandSearch() {

	bool eq = argument & EQ; 	// warning C4800: 'unsigned int' : forcing value to bool 'true' or 'false' (perf

	int step = (argument & DIX) ? -1:1;
	unsigned int i;
	nw_byte cl = color; 
   	statusRegister[2] &= 0xEF;
	
	switch (screenMode) {
 	case GRAPHICMODE4:
 	    cl &= 0x0F;
 	    for (i=sx;(i>=0)&&(i<256);i+=step) {
 	        if ((eq && (cl != pointG4(i,sy))) || (!eq && (cl == pointG4(i,sy)))) {
 	        	statusRegister[2] |= 0x10;
 	        	borderX = i;
                break;
	        }    
        }
        break;
 	case GRAPHICMODE5:
 	    cl &= 0x03;
 	    for (i=sx;(i>=0)&&(i<512);i+=step) {
 	        if ((eq && (cl != pointG5(i,sy))) || (!eq && (cl == pointG5(i,sy)))) {
 	            statusRegister[2] |= 0x10;
 	            borderX = i;
   	            break;
	        }    
        }
        break;
 	case GRAPHICMODE6:
 	    cl &= 0x0F;
 	    for (i=sx;(i>=0)&&(i<512);i+=step) {
 	        if ((eq && (cl != pointG6(i,sy))) || (!eq && (cl == pointG6(i,sy)))) {
 	            statusRegister[2] |= 0x10;
 	            borderX = i;
 	            break;
	        }    
        }
        break;
 	case GRAPHICMODE7:
 	    for (i=sx;(i>=0)&&(i<256);i+=step) {
 	        if ((eq && (cl != pointG7(i,sy))) || (!eq && (cl == pointG7(i,sy)))) { 
 	            statusRegister[2] |= 0x10;
 	            borderX = i;
 	            break;
	        }    
        }
        break;
    }
 	commandDone();
}

void V9938commands::commandLine() {

	//TODO: check for lus met maj+1, mag er wel altijd 1 bij opgeteld worden?
 	float nc;
	float stepMin = ((nx==0)||(ny==0)) ? 0 : (float)ny/(float)nx;	// nx=maj, ny=min
	int stepMaj;
	
	if (argument & MAJ) {
		// majorside parallel to y-axis
  		nc = (float)dx;
  		if (argument & DIX) stepMin *= -1;
    	if (argument & DIY) stepMaj = -1; else stepMaj = 1;
     	for (unsigned int i=0;i<nx+1;i++, dy+=stepMaj) {
	     	dy &= 1023;
        	DBERR("x: %u y: %u clr: %u\n", dx, dy, color);
        	commandPset(); //TODO: toch uitsplitsen met screen misschien (laydock)
			nc += stepMin;
			dx = (nw_word)nc;
		}
	} else {
	    // majorside parallel to x-axis
        nc = (float)dy;
        if (argument & DIY) stepMin *= -1;
        if (argument & DIX) stepMaj = -1; else stepMaj = 1;
        for (unsigned int i=0;i<nx+1;i++, dx+=stepMaj) {
	     	dy &= 1023;
           	commandPset();
			nc += stepMin;
			dy = (nw_word)nc;
		}
	}
	dy += ny; // TODO: moet dit wel?
	commandDone();
}


// LOGICAL MOVES

void V9938commands::commandLmmv() {

	assert((argument & DIX)==0);
	assert((argument & DIY)==0);

	switch (screenMode) {
 	case GRAPHICMODE4:
	    if (nx==0) nx=256;
 	    for (unsigned int y=dy;y<(dy+ny);y++) {
 	        for (unsigned int x=dx;x<(dx+nx);x++) {
 	            psetG4(x, y, color);
            }
        }
        break;
    case GRAPHICMODE5:
	    if (nx==0) nx=512;
 	    for (unsigned int y=dy;y<(dy+ny);y++) {
 	        for (unsigned int x=dx;x<(dx+nx);x++) {
 	            psetG5(x, y, color);
            }
        }
        break;
	case GRAPHICMODE6:
	    if (nx==0) nx=512;
 	    for (unsigned int y=dy;y<(dy+ny);y++) {
 	        for (unsigned int x=dx;x<(dx+nx);x++) {
 	            psetG6(x, y, color);
            }
        }
     	break;
	case GRAPHICMODE7:
	    if (nx==0) nx=256;
 	    for (unsigned int y=dy;y<(dy+ny);y++) {
 	        for (unsigned int x=dx;x<(dx+nx);x++) {
 	            psetG7(x, y, color);
            }
        }
        break;
	}
	dy += ny;
	// TODO: ny???
	commandDone();
}

void V9938commands::commandLmmm() {

	if (ny == 0) ny = 1024;
	int dix = (argument & DIX) ? -1:1;
	int diy = (argument & DIY) ? -1:1;
    
	switch (screenMode) {
	case GRAPHICMODE4:
		if (nx==0) nx=256;
		for (unsigned int y=0;y<ny;y++) {
			for (unsigned int x=0;x<nx;x++) psetG4(dx+(dix*x), dy+(diy*y), pointG4(sx+(dix*x), sy+(diy*y)));
		}
		break;
	case GRAPHICMODE5:
        assert((argument & DIX)==0); // TODO: dix en diy
        assert((argument & DIY)==0);
		if (nx==0) nx=512;
  		for (unsigned int y=0;y<ny;y++) {
			for (unsigned int x=0;x<nx;x++) psetG5(dx+x, dy+y, pointG5(sx+x, sy+y));
		}
		break;
	case GRAPHICMODE6:
        assert((argument & DIX)==0); // TODO: dix en diy
        assert((argument & DIY)==0);
		if (nx==0) nx=512;
  		for (unsigned int y=0;y<ny;y++) {
			for (unsigned int x=0;x<nx;x++) psetG6(dx+x, dy+y, pointG6(sx+x, sy+y));
		}
		break;
	case GRAPHICMODE7:
	    if (nx==0) nx=256;
 		for (unsigned int y=0;y<ny;y++) {
 		    for (unsigned int x=0;x<nx;x++) {
//                DBERR("lmmm: x: %i y: %i\n", dx+(dix*x), dy+(diy*y));
                psetG7(dx+(dix*x), dy+(diy*y), pointG7(sx+(dix*x), sy+(diy*y)));
            }
        }        
   		break;
	default:
		break;
	}
	sy += ny;
	dy += ny;
	//TODO: ny???
	commandDone();
}

void V9938commands::commandLmcm() {

	assert((argument & DIX)==0);
	assert((argument & DIY)==0);

	tx = sx;
	ty = sy;
	// het werkt, maar moet TR niet eerst geset worden?
} 

nw_byte V9938commands::transferLmcm() {

	if ((command>>4)!=10) {
		// commandDone(); // TODO: moet dit wel???
		return color;
	}

	switch (screenMode) {
	case GRAPHICMODE4: color = pointG4(tx, ty); break;
	case GRAPHICMODE5: color = pointG5(tx, ty); break;
	case GRAPHICMODE6: color = pointG6(tx, ty); break;
	case GRAPHICMODE7: color = pointG7(tx, ty); break;
	default:
		DBERR("lmcm: undefined state\n");
	}

	if (++tx==(sx+nx)) {
		tx=sx;
		if (++ty==(sy+ny)) {
			sy += ny;
			// TODO: ny???
			commandDone();
		}
	} else {
		statusRegister[2]|=0x80;    // set TR;
	}	
	return color;
}

void V9938commands::commandLmmc() {

	assert((argument & DIX)==0);
	assert((argument & DIY)==0);

	tx = dx;
	ty = dy;
	transferLmmc();
}

void V9938commands::transferLmmc() {

	switch (screenMode) {
 	case GRAPHICMODE4: psetG4(tx, ty, color); break;
	case GRAPHICMODE5: psetG5(tx, ty, color); break;
	case GRAPHICMODE6: psetG6(tx, ty, color); break;
	case GRAPHICMODE7: psetG7(tx, ty, color); break;
	default:
		break;
	}
	assert (nx!=0); // TODO: kan nog wel voorkomen
	if (++tx==(dx+nx)) {
		tx=dx;
		if (++ty==(dy+ny)) {
			dy += ny;
			// TODO: ny???
			commandDone();
			return;
		}
	}
	statusRegister[2]|=0x80;    // set TR;
}


// HIGH SPEED MOVES

void V9938commands::commandHmmv() {

	assert((argument & DIX)==0);
	assert((argument & DIY)==0);

	if (ny == 0) ny = 1024;
	
	unsigned int addr;
	switch (screenMode) {
	case GRAPHICMODE4:
	    if (nx==0) nx = 256;
	    for (unsigned int y=0;y<ny;y++) {
       	    addr = ((dy+y)<<7) + (dxl>>1);
         	for (unsigned int x=0;x<(nx>>1);x++, addr++) {
          	    vram[addr] = color;
          	}    
	    }    
        break;
	case GRAPHICMODE5:
 	    if (nx==0) nx = 512;
     	for (unsigned int y=0;y<ny;y++) {
     	    addr = ((dy+y)<<7) + (dx>>2);
         	for (unsigned int x=0;x<(nx>>2);x++, addr++) {
            	vram[addr] = color;
        	}   	
	    }    
        break;
	case GRAPHICMODE6:
 	    if (nx==0) nx = 512;
     	for (unsigned int y=0;y<ny;y++) {
     	    addr = ((dy+y)<<8) + (dx>>1);
         	for (unsigned int x=0;x<(nx>>1);x++, addr++) {
            	vram[(addr>>1)|((addr&1)<<16)] = color;
        	}   	
	    }    
        break;
	case GRAPHICMODE7:
//DBERR("-> dx:%3i dy:%3i sx:%3i sy:%3i nx:%3i ny:%3i color:%3i ARG:%i vdp0&0x0e: %i\n",dx,dy,sx,sy,nx,ny,color,argument, (int)screenMode);
//DBERR("-> dxl:%3i\n",dxl);
        if (nx==0) nx = 256;
     	for (unsigned int y=0;y<ny;y++) {
     	    addr = ((dy+y)<<8) + dxl;
	        for (unsigned int x=0;x<nx;x++, addr++) {
                vram[(addr>>1)|((addr&1)<<16)] = color;
	        }    
	    }    
        break;
	default:
		break;
	}
	dy += ny;
	//TODO: ny???
	commandDone();
}

inline void V9938commands::psetG4highspeed(int x, int y, nw_byte cl) {
	unsigned int addr = (y<<7)+(x>>1);
	nw_byte s = ((~x)&1)<<2;
	vram[addr] = (vram[addr] & ~(0x0F<<s)) | ((cl&0x0F)<<s);
}

inline void V9938commands::psetG5highspeed(int x, int y, nw_byte cl) {
	unsigned int addr = (y<<7)+(x>>2);
 	nw_byte s = ((~x)&3)<<1;
	vram[addr] = (vram[addr] & ~(0x03<<s)) | ((cl&0x03)<<s);
}

inline void V9938commands::psetG6highspeed(int x, int y, nw_byte cl) {
	unsigned int addr = (y<<8)+(x>>1);
	addr = (addr>>1)|((addr&1)<<16);
    nw_byte s = ((~x)&1)<<2;
	vram[addr] = (vram[addr] & ~(0x0F<<s)) | ((cl&0x0F)<<s);
}

inline void V9938commands::psetG7highspeed(int x, int y, nw_byte cl) {
	unsigned int addr = (y<<8) + x;
	addr = (addr>>1)|((addr&1)<<16);
 	vram[addr] = cl;
}

void V9938commands::commandHmmm() {

	unsigned int srcAddr, dstAddr;

	if (ny == 0) ny = 1024;
	int dix = (argument & DIX) ? -1:1;
	int diy = (argument & DIY) ? -1:1;
	
	switch (screenMode) {
	case GRAPHICMODE4:
		if (nx==0) nx = 256;
	    for (unsigned int y=0;y<ny;y++) {
      		srcAddr = ((sy+y*diy)<<7) + (sxl>>1);
      		dstAddr = ((dy+y*diy)<<7) + (dxl>>1);	        
         	for (unsigned int x=0;x<(nx>>1);x++) {
         		vram[dstAddr] = vram[srcAddr];
         		srcAddr+=dix;
         		dstAddr+=dix;
	       }  		
        }        
		break;
		
	case GRAPHICMODE5:
		if (nx==0) nx = 512;
	    for (unsigned int y=0;y<ny;y++) {
      		srcAddr = ((sy+y*diy)<<7) + (sx>>2);
      		dstAddr = ((dy+y*diy)<<7) + (dx>>2);	        
         	for (unsigned int x=0;x<(nx>>2);x++) {
         		vram[dstAddr] = vram[srcAddr];
         		srcAddr+=dix;
         		dstAddr+=dix;
	       }  		
        }        
		break;

	case GRAPHICMODE6:
		if (nx==0) nx = 512;
	    for (unsigned int y=0;y<ny;y++) {
      		srcAddr = ((sy+y*diy)<<8) + (sx>>1);
      		dstAddr = ((dy+y*diy)<<8) + (dx>>1);	        
         	for (unsigned int x=0;x<(nx>>1);x++) {
         		vram[(dstAddr>>1)|((dstAddr&1)<<16)] = vram[(srcAddr>>1)|((srcAddr&1)<<16)];
         		srcAddr+=dix;
         		dstAddr+=dix;
	       }  		
        }        
		break;
		
  	case GRAPHICMODE7:
		if (nx==0) nx = 256;
	    for (unsigned int y=0;y<ny;y++) {
      		srcAddr = ((sy+y*diy)<<8) + sxl;
      		dstAddr = ((dy+y*diy)<<8) + dxl;	        
         	for (unsigned int x=0;x<nx;x++) {
         		vram[(dstAddr>>1)|((dstAddr&1)<<16)] = vram[(srcAddr>>1)|((srcAddr&1)<<16)];
         		srcAddr+=dix;
         		dstAddr+=dix;
	       }  		
        }        
		break;
	default:
		break;
	}
	sy += ny;
	dy += ny;
	// TODO: ny???
	commandDone();
}

void V9938commands::commandYmmm() {

	unsigned int srcAddr, dstAddr;
	
 	if (ny == 0) ny = 1024;
	int dix = (argument & DIX) ? -1:1;
	int diy = (argument & DIY) ? -1:1;

	switch (screenMode) {
	case GRAPHICMODE4:
	    if (nx==0) nx = 256;
		for (unsigned int y=0;y<ny;y++) {
			for (unsigned int x=dxl>>1;(x>=0)&&(x<128);x+=dix) {
			    vram[((dy+y*diy)<<7)+x] = vram[((sy+y*diy)<<7)+x];
			}    
		}    
		break;
	case GRAPHICMODE5: assert(false); break;
	case GRAPHICMODE6:
	    if (nx==0) nx = 512;
		for (unsigned int y=0;y<ny;y++) {
			for (unsigned int x=dx>>1;(x>=0)&&(x<256);x+=dix) {
				srcAddr = ((sy+y*diy)<<8) + x;
				dstAddr = ((dy+y*diy)<<8) + x;
			    vram[(dstAddr>>1)|((dstAddr&1)<<16)] = vram[(srcAddr>>1)|((srcAddr&1)<<16)];
			}    
		}    
		break;	    
	case GRAPHICMODE7:
		// TODO: check!
		if (nx==0) nx = 256;
		for (unsigned int y=0;y<ny;y++) {
			for (unsigned int x=dxl;(x>=0)&&(x<256);x+=dix) {
				srcAddr = ((sy+y*diy)<<8) + x;
				dstAddr = ((dy+y*diy)<<8) + x;
				vram[(dstAddr>>1)|((dstAddr&1)<<16)] = vram[(srcAddr>>1)|((srcAddr&1)<<16)];
			}    
		}   
		break;
	default:
		break;
	}
	sy += ny;
	dy += ny;
	// TODO: change ny
	commandDone();
}

void V9938commands::commandHmmc() {

//	assert((argument & DIX)==0);
//	assert((argument & DIY)==0);

	switch (screenMode) {
	case GRAPHICMODE4:
	    if (nx==0) nx = 256;
     	adrp=(dy<<7)+(dxl>>1);
      	break;
	case GRAPHICMODE5:
	    if (nx==0) nx = 512;
     	adrp=(dy<<7)+(dx>>2);
      	break;
	case GRAPHICMODE6:
        if (nx==0) nx = 512;
        adrp=(dy<<8)+(dx>>1);
        break;
	case GRAPHICMODE7:
     	if (nx==0) nx = 256;
       break;
	default:
		break;
	}
	_nx=0;
	_ny=0;
	transferHmmc();
}

void V9938commands::transferHmmc() {

	int dix = (argument & DIX) ? -1:1;
	int diy = (argument & DIY) ? -1:1;

	unsigned int addr;

	switch (screenMode) {
	case GRAPHICMODE4:
		//DBERR("hmmc: ("<<dec<<adrp+_nx<<"),"<<(int)color<<endl);
		vram[adrp+(_nx*dix)] = color;
		if (++_nx==(nx>>1)) {
			_nx=0;
			adrp+=128;
			if (++_ny==ny) {
				// TODO: ny???
				dy += ny*diy;
				commandDone();
//                DBERR("HMMC AFGEROND!!!\n");
				return;
			}
		}
		break;
	case GRAPHICMODE5:
	    assert(false);
	    break;
	case GRAPHICMODE6:
//        assert(false);
        assert(dix == 1); // -1 nog testen
        assert(diy == 1);
	    addr = adrp+(_nx*dix);
	    addr = (addr>>1)|((addr&1)<<16);
		vram[addr] = color;
		if (++_nx==(nx>>1)) {
		    _nx=0;
		    adrp+=256;
		    if (++_ny==ny) {
		        dy += ny*diy;
     		    commandDone();
     		    return;
		    }
	    }
  	    break;    
 	case GRAPHICMODE7:
		assert (dix == 1);
		assert (diy == 1);
 	    psetG7highspeed(dx++, dy, color);
 	    if (++_nx==nx) {
 	        _nx=0;
 	        dy++;
 	        dx = dxl;
 	        if (++_ny==ny) {
 	            commandDone();
 	            return;
            }
        }        
      	break;
	default:
		break;
	}
	statusRegister[2]|=0x80;    // set TR;
}

void V9938commands::commandDone() {
	statusRegister[2]&=0x7E;    // reset TR and CE;
	command &= 0x0F;
}
