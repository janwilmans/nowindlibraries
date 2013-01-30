// V9938renderer.cpp

#include "V9938renderer.h"
#include "V9938sprites.h"
#include "V9938.h"
#include "Debug.h"
#include "GUI.h"

using namespace std;

V9938renderer::V9938renderer() {

	for (int i=0;i<256;i++) RGBgraphic7[i] = 
 		(convertTo8Bit[(i & 0x1C) >> 2]) << 16 |
		(convertTo8Bit[(i & 0xE0) >> 5]) << 8 |
 		(convertTo8Bit[(i & 0x03) << 1]);
	
	// YJK to RGB conversion table
 	signed int r, g, b, j1, k1;
  	for (int y=0;y<32;y++) {
	    for (int j=0;j<64;j++) {
	        for (int k=0;k<64;k++) {
	            
	            j1 = j; if (j1 > 31) j1 -= 64;
	            k1 = k; if (k1 > 31) k1 -= 64;
	            
	            r = (y + j1);
	            g = (y + k1);
	            b = (5 * y)/4 - (j1 / 2) - (k1 / 4);
	            
	            if (r<0) r = 0; else if (r>31) r = 31;
	            if (g<0) g = 0; else if (g>31) g = 31;
	            if (b<0) b = 0; else if (b>31) b = 31;
	            
	            YJK2RGB[y | (k<<5) | (j<<11)] = (r<<19) | (g<<11) | (b<<3);
	        }
        }
    }            
}

V9938renderer* V9938renderer::Instance() {

		/* implies singleton class */
		static V9938renderer deInstantie;
		return &deInstantie;
}

void V9938renderer::setVdp(V9938 *theVdp) {

	vdp = theVdp;
	vram = vdp->videoRam;
	MSXscreen = vdp->MSXscreen;
	spriteOverlay = vdp->spriteOverlay;
}

V9938renderer::~V9938renderer() {

   DBERR("V9938renderer destroyed.\n");
}

void V9938renderer::reset() {
    
    DBERR("RESET V9938renderer\n");
    
   	for (int i=0;i<16;i++) {
    	RGBpalette[i] =
   		(convertTo8Bit[defaultPalette[i][1]] << 16) |
		(convertTo8Bit[defaultPalette[i][0]] << 8) |
        (convertTo8Bit[defaultPalette[i][2]]);
    }  
    colorTable = 0;
    patternGeneratorTable = 0;
    colorRegister = 0;
    backDropColor = 0;
    textColor = 0;
    blinkBackDrop = 0;
    blinkText = 0;
    blinkTimer = 0;
    blinking = false;
    
    spritesEnabled = false;
    
    paletteColor0 = 0;
	transparent = false;
	inTextMode = false;

    displayOffset = 0;
    screenMode = 0;
    scr = vdp->MSXscreen;
}

void V9938renderer::setModeRegister1(nw_byte value) {

	inTextMode = ((value & 0x10) != 0);
    checkTransparency();
}    

void V9938renderer::setModeRegister2(nw_byte value) {

	transparent = ((value & 0x20) == 0);
	spritesEnabled = ((value & 2) != 0);
	checkTransparency();
}

void V9938renderer::writePalette(unsigned int color, nw_byte colorRB, nw_byte colorG) {
	
	unsigned int cl = (convertTo8Bit[(colorRB >> 4) & 7] << 16) |
					  (convertTo8Bit[colorG & 7] << 8) |
			 	 	  (convertTo8Bit[colorRB & 7]);
 	if (color == 0) {
  		paletteColor0 = cl;
	} else {  		
	    RGBpalette[color] = cl;
    }
	checkTransparency();
}

void V9938renderer::checkTransparency() {

	if (inTextMode) {
		
  		/* In textmode color0 is always solid,
		   when textColor = 0 it assumes the backDropColor
     	*/
     	
		RGBpalette[0] = paletteColor0;
		backDropColor = RGBpalette[colorRegister & 15];
    	unsigned int n = colorRegister >> 4;
		textColor = n ? RGBpalette[n]:backDropColor;
		
		blinkBackDrop = RGBpalette[blinkColor & 15];
		blinkText = RGBpalette[blinkColor >> 4];
		
	} else {
	    
	    /* In graphic modes when TP=0 color0 is solid, otherwise it is
	       transparent which means it assumes the colorRegister. When transparent
	       and colorRegister=0, it becomes solid again.
	    */
		if (transparent && ((colorRegister & 15) != 0)) {
			RGBpalette[0] = RGBpalette[colorRegister & 15];
		} else {
		    RGBpalette[0] = paletteColor0;
  		}
  		backDropColor = RGBpalette[colorRegister & 15];
 	}	
  	// TODO: quick & very dirty (bovendien klopt het niet voor screen6)
  	GUI::Instance()->setBorderColor(backDropColor);
}
    
void V9938renderer::setNameTable(nw_byte value) {
    
	nameTable = value;
}    

void V9938renderer::setColorTableLow(nw_byte value) {

	colorTable = (colorTable & 0x1C000) | (value << 6);
} 

void V9938renderer::setColorTableHigh(nw_byte value) {

	colorTable = (colorTable & 0x03FC0) | (value << 14);
}

void V9938renderer::setPatternGeneratorTable(nw_byte value) {

	patternGeneratorTable = value << 11;
}

void V9938renderer::setBorderColor(nw_byte value) {

	colorRegister = value;
  	checkTransparency();
} 

void V9938renderer::setBlinkColor(nw_byte value) {
	
	blinkColor = value;
	checkTransparency();
}
    
void V9938renderer::setBlinkPeriod(nw_byte value) {

	blinkPeriod = value;
	blinkTimer = 0;
	blinking = false;
}
    
void V9938renderer::setDisplayOffset(nw_byte value) {

	displayOffset = value;
}

void V9938renderer::dumpText1(unsigned int firstChunk, unsigned int count) {

//	DBERR("TEXT1\n");
	
	unsigned int nt = (nameTable & 0x7F) << 10;
	unsigned int firstChunkInLine = firstChunk & (DISPLAYCHUNKS-1);
	unsigned int vp, pixel, lines, i, n;
	
	if ((firstChunkInLine + count) <= DISPLAYCHUNKS) { //TODO: <= of = ???
     	while (count--) {
	        *scr++ = 0xff0000;
	        *scr++ = 0xff0000;
	    }    
	} else {
	    
		n = (firstChunk >> 7);
        if (firstChunkInLine != 0) {
            // regel uitvullen tot rechter border
            i = DISPLAYCHUNKS - firstChunkInLine;
       		count -= i;
         	while (i--) {
    	        *scr++ = 0xff0000;
    	        *scr++ = 0xff0000;
    	    }
            n++;    
        }
        
       	lines = count >> 7;
   		while (lines--) {
   			RENDERBORDER(8, backDropColor);
   			vp = nt + ((n/8) * 40);
   			for (i=0;i<40;i++) {
		    	pixel = vram[patternGeneratorTable + (n&7) + (vram[vp++] * 8)];
		    	*scr++ = (pixel & 0x80)? textColor:backDropColor;
		    	*scr++ = (pixel & 0x40)? textColor:backDropColor;
		    	*scr++ = (pixel & 0x20)? textColor:backDropColor;
		    	*scr++ = (pixel & 0x10)? textColor:backDropColor;
		    	*scr++ = (pixel & 0x08)? textColor:backDropColor;
		    	*scr++ = (pixel & 0x04)? textColor:backDropColor;

	    	}
	    	RENDERBORDER(8, backDropColor);
	    	n++;
     	}
     	i = count & (DISPLAYCHUNKS-1);
     	if (i) {
     	    RENDERBORDER(8+8, backDropColor);
     	    while (i--) {
     	        *scr++ = 0x00ff00;
     	        *scr++ = 0x00ff00;
     	    }    
     	}        
	}
}
 
std::string V9938renderer::copyFromText2(unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
    
    int tx = 0;
    if (x < 32) tx = 0;
    else tx = (x-32)/6;

    int ty = 0;
    if (y < 32) ty = 0;
    else ty = (y-32)/16;
   
    int tw = (w+6)/6 + tx;
    int th = (h+16)/16 + ty;

//    DBERR("x: " << x << " tx: " << tx << endl);
//    DBERR("y: " << y << " ty: " << ty << endl);
//    DBERR("w: " << w << " tw: " << tw << endl);
//    DBERR("h: " << h << " th: " << th << endl);
    
    string copytest = "";
    char line[100];
    unsigned int nt = (nameTable & 0x7C) << 10;
    for (int i=ty;i<th;i++) {
        int j=tx;
        int p=0;
        for (;j<tw;j++) {
            line[p++] = vram[nt+(i*80)+j];
//            vram[nt+(i*80)+j] = 'W';
        }      
        line[p++] = '\n';
        line[p++] = '\0';
        copytest = copytest + string(line); 
    }        
    return copytest;
}
    
void V9938renderer::dumpText2(unsigned int firstChunk, unsigned int count) {

//	DBERR("TEXT2\n");
	
 	if (count > (212*128)) {
        DBERR("count----------------------%u\n", count);
        return;
    }        

// TODO: nameTable and colorTable mirroring
	unsigned int nt = (nameTable & 0x7C) << 10;
	unsigned int firstChunkInLine = firstChunk & (DISPLAYCHUNKS-1);
	unsigned int vp, ct, c1, c2;
	unsigned int pixel, lines, i, n;

	if (firstChunkInLine == 0) {
	    // anders! met emuTime, dit punt is ook niet ideaal, mag alleen aan het begin van het scherm
     	if (blinkTimer-- == 0) {
     		if (blinking) {
   			    blinkTimer = 10 * (blinkPeriod & 15);
           	    if (blinkTimer > 0) blinking = false;
   			} else {
   		        blinkTimer = 10 * (blinkPeriod >> 4);
   		        if (blinkTimer > 0) blinking = true;
       		}        
      	}
 	}
	
	if ((firstChunkInLine + count) <= DISPLAYCHUNKS) {
     	while (count--) {
	        *scr++ = 0xff0000;
	        *scr++ = 0xff0000;
	        *scr++ = 0xff0000;
	        *scr++ = 0xff0000;
	    }    
	} else {
	    
	    n = firstChunk >> 7;
        if (firstChunkInLine != 0) {
    		// regel uitvullen tot rechter border
    		i = DISPLAYCHUNKS - firstChunkInLine;
       		count -= i;
         	while (i--) {
    	        *scr++ = 0xff0000;
    	        *scr++ = 0xff0000;
    	        *scr++ = 0xff0000;
    	        *scr++ = 0xff0000;
    	    }
            n++;    
        }    
       	lines = count >> 7;
       	
   		while (lines--) {
   			RENDERBORDER(16, backDropColor);
   			vp = nt + ((n >> 3) * 80);
   			ct = (colorTable & 0x1fc00) + ((n >> 3) * 10);
   			for (i=0;i<80;i++) {

   			    if ((blinking) && (vram[ct + (i >> 3)] & (0x80 >> (i & 7)))) {
   			        c1 = blinkText;
   			        c2 = blinkBackDrop;
   			    } else {
   			        c1 = textColor;
   			        c2 = backDropColor;
          		}
		    	pixel = vram[patternGeneratorTable + (n&7) + (vram[vp++] << 3)];
		    	*scr++ = (pixel & 0x80)? c1:c2;
		    	*scr++ = (pixel & 0x40)? c1:c2;
		    	*scr++ = (pixel & 0x20)? c1:c2;
		    	*scr++ = (pixel & 0x10)? c1:c2;
		    	*scr++ = (pixel & 0x08)? c1:c2;
		    	*scr++ = (pixel & 0x04)? c1:c2;
	    	}
	    	RENDERBORDER(16, backDropColor);
	    	n++;
     	}
     	i = count & (DISPLAYCHUNKS-1);
     	if (i) {
     	    RENDERBORDER((8+8) << 1, backDropColor);
     	    while (i--) {
     	        *scr++ = 0x00ff00;
     	        *scr++ = 0x00ff00;
     	        *scr++ = 0x00ff00;
     	        *scr++ = 0x00ff00;
     	    }    
     	}        
	}
}

void V9938renderer::dumpGraphic1(unsigned int firstChunk, unsigned int count) {

//	DBERR("GRAPH1\n");
	
	unsigned int nt = (nameTable & 0x7F) << 10;
	unsigned int firstChunkInLine = firstChunk & (DISPLAYCHUNKS-1);
	unsigned int vp, pg, color0, color1, ct, pattern;
	unsigned int pixel, lines, i, n;
	
	if ((firstChunkInLine + count) <= DISPLAYCHUNKS) {
     	while (count--) {
	        *scr++ = 0xff0000;
	        *scr++ = 0xff0000;
	    }    
	} else {
	    
	    n = firstChunk >> 7;
	    if (firstChunkInLine != 0) {
    		// regel uitvullen tot rechter border
    		i = DISPLAYCHUNKS - firstChunkInLine;
       		count -= i;
         	while (i--) {
    	        *scr++ = 0xff0000;
    	        *scr++ = 0xff0000;
    	    }
            n++;    
        }
       	lines = count >> 7;
   		while (lines--) {

   			vp = nt + ((n >> 3) * 32);
   			for (i=0;i<32;i++) {
   			    pattern = vram[vp++];
   			    ct = colorTable + (pattern >> 3);
   			    pg = patternGeneratorTable + (pattern << 3) + (n & 7);
   			    pixel = vram[pg];
   			    color0 = vram[ct];
   				color1 = RGBpalette[color0 >> 4];
				color0 = RGBpalette[color0 & 15];
   				*scr++ = (pixel & 0x80)? color1:color0;
   				*scr++ = (pixel & 0x40)? color1:color0;
   				*scr++ = (pixel & 0x20)? color1:color0;
   				*scr++ = (pixel & 0x10)? color1:color0;
   				*scr++ = (pixel & 0x08)? color1:color0;
   				*scr++ = (pixel & 0x04)? color1:color0;
   				*scr++ = (pixel & 0x02)? color1:color0;
   				*scr++ = (pixel & 0x01)? color1:color0;
   			}    
	    	n++;
     	}
     	i = count & (DISPLAYCHUNKS-1);
     	if (i) {
     	    while (i--) {
     	        *scr++ = 0x00ff00;
     	        *scr++ = 0x00ff00;
     	    }    
     	}        
	}
}

void V9938renderer::dumpGraphic2(unsigned int firstChunk, unsigned int count) {

//    DBERR("GRAPH2: " << dec << firstChunk << " count: " << count << endl );
// TODO: color 0 is now rendered as black in stead of borderColor (also check TP bit???)

	unsigned int nt = (nameTable & 0x7F) << 10;
	unsigned int firstChunkInLine = firstChunk & (DISPLAYCHUNKS-1);
	unsigned int vp, pg, color0, color1, ct, pattern;
	unsigned int pixel, lines, i, n;
	
	unsigned int spriteCount = count;

	if ((firstChunkInLine + count) <= DISPLAYCHUNKS) {
     	while (count--) {
	        *scr++ = 0xff0000;
	        *scr++ = 0xff0000;
	    }    
	} else {
		
        n = firstChunk >> 7;
        if (firstChunkInLine != 0) {
            // regel uitvullen tot rechter border
        	i = DISPLAYCHUNKS - firstChunkInLine;
        	count -= i;
          	while (i--) {
                *scr++ = 0xff0000;
                *scr++ = 0xff0000;
            }
            n++;
        }
        
       	lines = count >> 7;

        // patternTable and colorTable mirroring (TODO: check other screens and vdp registers)
        unsigned int pgMask = (patternGeneratorTable & 0x01800) | (~0x1800);
        unsigned int ctMask = (colorTable & 0x01fc0) | (~0x01fc0);
        
   		while (lines--) {

   			pg = (patternGeneratorTable & 0x1E000 ) + ((n >> 6) * 0x800);
            pg &= pgMask;
   			ct = (colorTable & 0x1E000) + ((n >> 6) * 0x800);
   			ct &= ctMask;
   			vp = nt + ((n >> 3) << 5);

   			for (i=0;i<32;i++) {

   				pattern = (vram[vp++] << 3) + (n & 7);
   				pixel = vram[pg + pattern];
   				color0 = vram[ct + pattern];
   				color1 = RGBpalette[color0 >> 4];
				color0 = RGBpalette[color0 & 15];

   				*scr++ = (pixel & 0x80)? color1:color0;
   				*scr++ = (pixel & 0x40)? color1:color0;
   				*scr++ = (pixel & 0x20)? color1:color0;
   				*scr++ = (pixel & 0x10)? color1:color0;
   				*scr++ = (pixel & 0x08)? color1:color0;
   				*scr++ = (pixel & 0x04)? color1:color0;
   				*scr++ = (pixel & 0x02)? color1:color0;
   				*scr++ = (pixel & 0x01)? color1:color0;
   			}    
	    	n++;
     	}
     	i = count & (DISPLAYCHUNKS-1);
     	if (i) {
     	    while (i--) {
     	        *scr++ = 0x00ff00;
     	        *scr++ = 0x00ff00;
     	    }    
     	}        
	}
	
	if (!(vdp->vdpReg[8] & 0x02)) {
		vdp->sprites->updateSpriteOverlayMode1(vdp->vdpReg[1]);
    	unsigned int sp = firstChunk << 1;
    	vp = sp;
    	sp = sp + (displayOffset << 8);

    	while (spriteCount--) {
       		sp &= 0xffff;
      		pixel = spriteOverlay[sp++]; if (pixel) MSXscreen[vp] = RGBpalette[pixel]; vp++;
       		pixel = spriteOverlay[sp++]; if (pixel) MSXscreen[vp] = RGBpalette[pixel]; vp++;
    	}
    }    
    
}

void V9938renderer::dumpMultiColor(unsigned int firstChunk, unsigned int count) {
	
	unsigned int nt = (nameTable & 0x7F) << 10;
	unsigned int firstChunkInLine = firstChunk & (DISPLAYCHUNKS-1);
	unsigned int p1, lines, i, n, pattern;
	unsigned int pg = (patternGeneratorTable & 0x1E000);
	
	if ((firstChunkInLine + count) <= DISPLAYCHUNKS) {
     	while (count--) {
	        *scr++ = 0xff0000;
	        *scr++ = 0xff0000;
	    }    
	} else {
		// regel uitvullen tot rechter border
		i = DISPLAYCHUNKS - firstChunkInLine;
   		count -= i;
     	while (i--) {
	        *scr++ = 0xff0000;
	        *scr++ = 0xff0000;
	    }    

       	lines = count >> 7;
       	n = ((firstChunk >> 7) + 1 + displayOffset) & 255;
   		while (lines--) {

   			pattern = nt + ((n >> 3) << 5);
   			for (int j=0;j<32;j++) {

   				i = vram[pg + (vram[pattern++] << 3) + ((n>>2) & 7)];
   				p1 = RGBpalette[i >> 4];
   				*scr++ = p1;
       			*scr++ = p1; 
   				*scr++ = p1;
       			*scr++ = p1; 
   				p1 = RGBpalette[i & 15];
   				*scr++ = p1;
       			*scr++ = p1; 
   				*scr++ = p1;
  				*scr++ = p1;
   			}    
	    	++n &= 255;
     	}
     	i = count & (DISPLAYCHUNKS-1);
     	if (i) {
     	    while (i--) {
     	        *scr++ = 0x00ff00;
     	        *scr++ = 0x00ff00;
     	    }    
     	}        
	}
}
    
// TODO: implement the vramMask more effecient!!! (no need to and it every byte)
#define G4chunk \
    pixel = vram[(vp++ & vramMask)]; 	\
    *scr++ = RGBpalette[pixel >> 4]; 	\
    *scr++ = RGBpalette[pixel & 15]
	
void V9938renderer::dumpGraphic4(unsigned int firstChunk, unsigned int count) {
	
// TODO: nameTable mirroring (R#2)

	unsigned int nt = (nameTable & 0x60) << 10;
	unsigned int vp = nt + ((firstChunk + (displayOffset << 7)) & 0x7FFF);
	unsigned int pixel, n;
	unsigned int spriteCount = count << 1;
	
	unsigned int vramMask = (nameTable & 0x1f) << 10;
	vramMask |= 0x183ff;
	
    n = (count + 7) >> 3;
    switch (count & 7) {
    case 0: do {	G4chunk;
    case 7: 		G4chunk;
    case 6: 		G4chunk;
    case 5: 		G4chunk;
    case 4: 		G4chunk;
    case 3: 		G4chunk;
    case 2: 		G4chunk;
    case 1: 		G4chunk;
          			vp = nt + (vp & 0x7FFF);
     			} while (--n > 0);
    }

	if (!(vdp->vdpReg[8] & 0x02)) {
  		unsigned int sp = firstChunk << 1;
  		vp = sp;
  		sp = sp + (displayOffset << 8);
  		while (spriteCount--) {
       		sp &= 0xffff;
    		pixel = spriteOverlay[sp++]; if (pixel) MSXscreen[vp] = RGBpalette[pixel]; vp++;
    		pixel = spriteOverlay[sp++]; if (pixel) MSXscreen[vp] = RGBpalette[pixel]; vp++;
		}    
	}
}

#define G5chunk \
    pixel = vram[vp++]; 					\
    *scr++ = RGBpalette[pixel >> 6]; 		\
    *scr++ = RGBpalette[(pixel >> 4) & 3];	\
    *scr++ = RGBpalette[(pixel >> 2) & 3];	\
    *scr++ = RGBpalette[pixel & 3]

void V9938renderer::dumpGraphic5(unsigned int firstChunk, unsigned int count) {
	
// TODO: nameTable mirroring (R#2)
	//DBERR("dumpGraphic5\n");
	
	unsigned int nt = (vdp->vdpReg[2] & 0x60) << 10;
	unsigned int vp = nt + ((firstChunk + (displayOffset << 7)) & 0x7FFF);
	unsigned int pixel, n;

    n = (count + 7) >> 3;
    switch (count & 7) {
    case 0: do {	G5chunk;
    case 7: 		G5chunk;
    case 6: 		G5chunk;
    case 5: 		G5chunk;
    case 4: 		G5chunk;
    case 3: 		G5chunk;
    case 2: 		G5chunk;
    case 1: 		G5chunk;
          			vp = nt + (vp & 0x7FFF);
     			} while (--n > 0);
    }	
}


#define G6chunk 						\
    pixel = vram[vp];					\
    *scr++ = RGBpalette[pixel >> 4]; 	\
    *scr++ = RGBpalette[pixel & 15];	\
    pixel = vram[vp++ | 0x10000];		\
    *scr++ = RGBpalette[pixel >> 4]; 	\
    *scr++ = RGBpalette[pixel & 15]


void V9938renderer::dumpGraphic6(unsigned int firstChunk, unsigned int count) {

// TODO: nameTable mirroring (R#2)
	unsigned int nt = (nameTable & 0x20) << 10;
	unsigned int vp = firstChunk + (displayOffset << 7); // =((vp >> 1)
	unsigned int pixel, n;
	
    n = (count + 7) >> 3;
    switch (count & 7) {
    case 0: do {	G6chunk;
    case 7: 		G6chunk;
    case 6: 		G6chunk;
    case 5: 		G6chunk;
    case 4: 		G6chunk;
    case 3: 		G6chunk;
    case 2: 		G6chunk;
    case 1: 		G6chunk;
  		  			vp = (vp & 0x7fff) | nt;
     			} while (--n > 0);
    }
}


#define G7chunk \
    *scr++ = RGBgraphic7[vram[(vp & vramMask)]]; \
    *scr++ = RGBgraphic7[vram[(vp++ & vramMask) | 0x10000]]
    	
void V9938renderer::dumpGraphic7(unsigned int firstChunk, unsigned int count) {

// TODO: nameTable mirroring (R#2)	
	unsigned int nt = (nameTable & 0x20) << 10;
	// nt moet er volgens mij ook nog bij op.... dan kan de AND constructie in de writeChunks misschien weg
	unsigned int vp = firstChunk + (displayOffset << 7); // =((vp >> 1)
	unsigned int n;

	unsigned int vramMask = (nameTable & 0x1f) << 10;
	vramMask |= 0x183ff;

    n = (count + 7) >> 3;
    switch (count & 7) {
    case 0: do {	G7chunk;
    case 7: 		G7chunk;
    case 6: 		G7chunk;
    case 5: 		G7chunk;
    case 4: 		G7chunk;
    case 3: 		G7chunk;
    case 2: 		G7chunk;
    case 1: 		G7chunk;
    			    vp = (vp & 0x7fff) | nt;
     			} while (--n > 0);
    }	
}


#define G7yjkChunk \
   	y0 = vram[vp];												\
    y1 = vram[vp++ | 0x10000];									\
   	y2 = vram[vp];												\
    y3 = vram[vp++ | 0x10000];									\
	jk = ((y0&7) | ((y1&7)<<3) | ((y2&7)<<6) | ((y3&7)<<9))<<5;	\
	*scr++ = YJK2RGB[(y0 >> 3) | jk];							\
	*scr++ = YJK2RGB[(y1 >> 3) | jk];							\
	*scr++ = YJK2RGB[(y2 >> 3) | jk];							\
	*scr++ = YJK2RGB[(y3 >> 3) | jk]

void V9938renderer::dumpGraphic7yjk(unsigned int firstChunk, unsigned int count) {

	// DEZE ROUTINE GAAT ALLEEN GOED ALS DE CHUCKSIZE 64 IS!!! (maar de rest van de routines trekt dat nog niet)

	bool eenChunkTeveel = false;
	if (firstChunk & 1) scr -= 2;
 	firstChunk >>= 1;
	if (count & 1) {
		eenChunkTeveel = true;
		count++;
	}		
 	count >>= 1;

	unsigned int nt = (nameTable & 0x20) << 10;
	unsigned int vp = firstChunk + (displayOffset << 7); // =((vp >> 1)
	unsigned int n;
    unsigned int y0, y1, y2, y3, jk;
	
    n = (count + 7) >> 3;
    switch (count & 7) {
    case 0: do {	G7yjkChunk;
    case 7: 		G7yjkChunk;
    case 6: 		G7yjkChunk;
    case 5: 		G7yjkChunk;
    case 4: 		G7yjkChunk;
    case 3: 		G7yjkChunk;
    case 2: 		G7yjkChunk;
    case 1: 		G7yjkChunk;
    			    vp = (vp & 0x7fff) | nt;
     			} while (--n > 0);
    }
	if (eenChunkTeveel) scr -= 2;
}

void V9938renderer::dumpDisabledScreen256(unsigned int firstChunk, unsigned int count) {

    if (count & 127) DBERR("This should not happen, a displayline might be lost (disable256) %u\n", count);
    count = (count / 128) * 256;
    for (unsigned int i=0;i<count;++i) *scr++ = backDropColor;
}

void V9938renderer::dumpDisabledScreen512(unsigned int firstChunk, unsigned int count) {
  
	// TODO: zou er geen geen onderscheid moeten worden gemaakt naar screenMode (scr6 is anders)???
    if (count & 127) DBERR("This should not happen, a displayline might be lost (disable512) %u\n", count);
    count = (count / 128) * 512;
    for (unsigned int i=0;i<count;++i) *scr++ = backDropColor;
}

void V9938renderer::afterCallScaler() {
	scr = MSXscreen;
	vdp->sprites->updateSpriteOverlayMode2(vdp->vdpReg[1]);
}

void V9938renderer::callScaler() {

	vdp->sprites->updateSpriteOverlayMode2(vdp->vdpReg[1]); // TODO: te laat hier eigenlijk...
}
