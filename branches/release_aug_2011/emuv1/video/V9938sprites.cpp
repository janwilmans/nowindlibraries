// V9938sprites.cpp

#include "V9938sprites.h"
#include "V9938.h"
#include "Debug.h"

using namespace std;

/* TODO:
    
	- sprite maginificatie
	- what happens exactly when TP=1 (collision and display of color 0???)
	- what happens when sprite priority is cancelled and when 2 sprites are
	already OR'ed and a third sprite with (CC=1) is at the same location?
	- in case of more collisions in mode2, the coordinates of the last collision
	is stored. (it makes more sense that only the first collision is stored)
	- in stead of one big spriteOverlay: store only and array of addresses which
	contain a sprite line (list of max 32 sprites x 32 lines (in case of 16x16)
 	 = 1024 addresses and spritelines of 8 pixels)
	
*/


V9938sprites::V9938sprites(V9938 *thevdp) {

    vdp = thevdp;
	vram = vdp->videoRam;
	spriteOverlay = vdp->spriteOverlay;
}

V9938sprites::~V9938sprites() {

   DBERR("V9938sprites destroyed.\n");
}

void V9938sprites::reset() {
    
    DBERR("RESET V9938sprites\n");
    patternGeneratorTable = 0;
    attributeTable = 0;
	collisionX = 0;
	collisionY = 0;	
}    

void V9938sprites::setAttributeTableLow(nw_byte value) {

	// vdp 5 
    attributeTable = (attributeTable & 0x18000) | (value << 7);
}

void V9938sprites::setPatternGeneratorTable(nw_byte value) {

	// vdp 6
    patternGeneratorTable = (value & 0x3f) << 11;
}

void V9938sprites::setAttributeTableHigh(nw_byte value) {

	// vdp 11
	attributeTable = (attributeTable & 0x07f80) | ((value & 3) << 15);
}

nw_byte V9938sprites::getCollisionCoordinate(nw_byte value) {

    nw_byte coordinate;

    switch (value) {
    case 3: coordinate = collisionX & 255; break;
    case 4: coordinate = (collisionX >> 8) | 0xFE; break;
    case 5:
        // collision detect bit (S#0 bit6) is reset when S#0 is read
        coordinate = collisionY & 255;
        collisionX = 0;
        collisionY = 0;
        break;
    case 6: coordinate = (collisionY >> 8) | 0xFC; break;
    default:
        assert(false);
    }
    return coordinate;
}
void V9938sprites::updateSpriteOverlayMode1(nw_byte vdpModeRegister1) {
	
    unsigned int sx, sy, p1, p2;
    unsigned int patternColumn, color;
    unsigned int attr = attributeTable;

    // spriteOverlay and lineCount
    for (int i=0;i<(256*256);++i) spriteOverlay[i] = 0;

	for (unsigned int i=0;i<256;i++) {
		unsigned int spriteCount = 0;
		for (int sprite=0;sprite<32;sprite++) {
			attr = attributeTable | (sprite << 2);
			sy = vram[attr];
			if (sy == 208) break;
			sy = (sy + 1) & 255;
			if ((i >= sy) && (i < (sy+16))) {
				sx = vram[attr+1];
				patternColumn = patternGeneratorTable + (8 * (vram[attr+2] & 0xfc)) + (i - sy);
				color = vram[attr+3];
				p1 = vram[patternColumn];
				p2 = vram[patternColumn + 16];
				if (p1 || p2) {
					spriteCount++;
					if (spriteCount > 4) {
						ninthSpriteDetected(sprite);
						break;
					} else {
						drawSpriteMode1(sx, i, p1, color);
						drawSpriteMode1(sx+8, i, p2, color);
					}
				}
			}
		}
	}	
}

/*
void V9938sprites::updateSpriteOverlayMode1(nw_byte vdpModeRegister1) {

    unsigned int sx, sy, p1, p2;
    unsigned int patternColumn, color;
    unsigned int attr = attributeTable;

    // spriteOverlay and lineCount
    for (int i=0;i<256;++i) lineCount[i] = 0;
    for (int i=0;i<(256*256);++i) spriteOverlay[i] = 0;

	if (vdpModeRegister1 & SI) {

        // size 16x16
        for (int sprite=0;sprite<32;sprite++) {
            sy = vram[attr++];
            if (sy == 208) break;
            sx = vram[attr++];
            patternColumn = patternGeneratorTable + 8 * (vram[attr++] & 0xfc);
            color = vram[attr++];

            for (int y=0;y<16;y++) {
                ++sy &= 255;
                p1 = vram[patternColumn];
                p2 = vram[16 + patternColumn++];
                if (p1 || p2) {
                    lineCount[sy]++;
                    if (lineCount[sy] > 4) {
                        ninthSpriteDetected(sprite);
                    } else {
                        drawSpriteMode1(sx, sy, p1, color);
                        drawSpriteMode1(sx+8, sy, p2, color);
                    }
                }
            }
        }
    } else {

        // size 8x8
        for (int sprite=0;sprite<32;sprite++) {
            sy = vram[attr++];
            if (sy == 208) break;
            sx = vram[attr++];
            patternColumn = patternGeneratorTable + 8 * vram[attr];
            color = vram[attr++];
            
            for (int y=0;y<8;y++) {
                ++sy &= 255;
                p1 = vram[patternColumn++];
                if (p1) {
                    lineCount[sy]++;
                    if (lineCount[sy] > 4) {
                        ninthSpriteDetected(sprite);
                    } else {
                        drawSpriteMode1(sx, sy, p1, color);
                    }
                }
            }
        }
    }
}
*/

void V9938sprites::updateSpriteOverlayMode2(nw_byte vdpModeRegister1) {

    unsigned int sx, sy, p1, p2;
    unsigned int patternColumn, color;
    // bit 9 maakt onderscheid tussen attribute en color
    // TODO: vallen attribute en color samen indien de bit9 op 0 wordt gezet???
    
	unsigned int colorTable = attributeTable & 0x1fc00; // bit 9 is reset
	unsigned int attr = colorTable | 512;

    // spriteOverlay and lineCount
    for (int i=0;i<256;++i) lineCount[i] = 0;
    for (int i=0;i<(256*256);++i) spriteOverlay[i] = 0;

	if (vdpModeRegister1 & SI) {

        // size 16x16
        for (int sprite=0;sprite<32;sprite++, attr += 2) {
            sy = vram[attr++];
            if (sy == 216) break;
            sx = vram[attr++];
            patternColumn = patternGeneratorTable + 8 * (vram[attr] & 0xfc);

            for (int y=0;y<16;y++,colorTable++) {
                ++sy &= 255;
                p1 = vram[patternColumn];
                p2 = vram[16 + patternColumn++];
                if (p1 || p2) {
                    lineCount[sy]++;
                    if (lineCount[sy] > 8) {
                        ninthSpriteDetected(sprite);
                    } else {
                        color = vram[colorTable];
                        drawSpriteMode2(sx, sy, p1, color);
                        drawSpriteMode2(sx+8, sy, p2, color);
                    }
                }
            }
        }
    } else {

        // size 8x8
        for (int sprite=0;sprite<32;sprite++, attr +=2) {
            sy = vram[attr++];
            if (sy == 216) break;
            sx = vram[attr++];
            patternColumn = patternGeneratorTable + 8 * vram[attr];
            
            for (int y=0;y<8;y++,colorTable++) {
                ++sy &= 255;
                p1 = vram[patternColumn++];
                if (p1) {
                    lineCount[sy]++;
                    if (lineCount[sy] > 8) {
                        ninthSpriteDetected(sprite);
                    } else {
                        drawSpriteMode2(sx, sy, p1, vram[colorTable]);
                    }
                }
            }
        }
    }
}

void V9938sprites::drawSpriteMode1(int x, int y, unsigned int pattern, unsigned int color) {

    int xc;
    if (color & EC) x=-32;

    for (int x1=0;x1<8;x1++) {
        xc = x + x1;
        if ((xc<0) || (xc>255)) continue;
        if (pattern & (0x80 >> x1)) {
            if (spriteOverlay[(y << 8) + xc]) {
                // collision detected
                vdp->statusReg[0] |= 0x20;
            } else {
                // no sprite at present location
                spriteOverlay[(y << 8) + xc] = color & 15;
            }
        }
    }
}

void V9938sprites::drawSpriteMode2(int x, int y, unsigned int pattern, unsigned int color) {

    unsigned int previousColor;
    int xc;
    bool priorityCancelled = color && CC;
    bool collisionDetectionEnabled = !(color && IC);
    if (color & EC) x=-32;

    for (int x1=0;x1<8;x1++) {
        xc = x + x1;
        if ((xc<0) || (xc>255)) continue;
        if (pattern & (0x80 >> x1)) {
            previousColor = spriteOverlay[(y << 8) + xc];
            if (previousColor) {
                // sprite with higher priority at present location
                if (priorityCancelled) {
                    // priority is cancelled, sprite is OR'ed with previous
                    spriteOverlay[(y << 8) + xc] = (color | previousColor) & 15;
                } else {
                    if (collisionDetectionEnabled) {
                        collisionX = xc + 12;
                        collisionY = y + 8;
                        vdp->statusReg[0] |= 0x20;
                    }
                }
            } else {
                // no sprite at present location
                spriteOverlay[(y << 8) + xc] = color & 15;
            }
        }
    }
}

void V9938sprites::ninthSpriteDetected(int spriteNumber) {

    // 5th/9th spritenumber in S#0 and set bit 6
    vdp->statusReg[0] = (vdp->statusReg[0] & 0xE0) | spriteNumber | 0x40;
}

