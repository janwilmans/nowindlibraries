//! V9938sprites.h
#ifndef V9938sprites_H
#define V9938sprites_H

#include <iostream>
#include "msxtypes.h"


#define EC 0x80     // early clock
#define CC 0x40     // priority enable
#define IC 0x20     // collision detect

// Register 1
#define MAG 0x01    // sprite magnification
#define SI 0x02     // sprite size

// Register 8
#define MS 0x80     // mouse select
#define LP 0x40     // light pen
#define TP 0x20     // transparent
#define SPD 0x02    // sprite display

class V9938;

/*!
 * The V9938sprites class contains the sprite-display methods for all screens and sprite-modes
 */
class V9938sprites {

private:

        V9938 *vdp;
		nw_byte *vram;
		unsigned int *spriteOverlay;
		
        unsigned int patternGeneratorTable;
        unsigned int attributeTable;
        unsigned int lineCount[256];
        int collisionX;
        int collisionY;
        
        void drawSpriteMode1(int, int, unsigned int, unsigned int);
        void drawSpriteMode2(int, int, unsigned int, unsigned int);
        void ninthSpriteDetected(int);

public:

		V9938sprites(V9938*);
		~V9938sprites();
		void reset();

        void setPatternGeneratorTable(nw_byte);
        void setAttributeTableLow(nw_byte);
        void setAttributeTableHigh(nw_byte);
        nw_byte getCollisionCoordinate(nw_byte);
        
        void updateSpriteOverlayMode1(nw_byte);
        void updateSpriteOverlayMode2(nw_byte);

};

#endif

