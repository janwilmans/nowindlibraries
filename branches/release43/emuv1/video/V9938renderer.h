#ifndef V9938RENDERER_H
#define V9938RENDERER_H

#include "msxtypes.h"
#include <string>

class GUI;

static const nw_byte convertTo8Bit[8] = { 0,36,73,109,145,182,219,255 };

static const unsigned int defaultPalette[16][3] = {
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 6, 1, 1 },
    { 7, 3, 3 },
    { 1, 1, 7 },
    { 3, 2, 7 },
    { 1, 5, 1 },
    { 6, 2, 7 },
    { 1, 7, 1 },
    { 3, 7, 3 },
    { 6, 6, 1 },
    { 6, 6, 4 },
    { 4, 1, 1 },
    { 2, 6, 5 },
    { 5, 5, 5 },
    { 7, 7, 7 }    
};    

const unsigned int RGBspritesGraphic7[16] = {
	
	0x000000, 0x000024, 0x490000, 0x490024,
	0x004900, 0x004924, 0x494900, 0x494924,
	0xFF6D24, 0x0000FF, 0xFF0000, 0xFF00FF,
	0x00FF00, 0x00FFFF, 0xFFFF00, 0xFFFFFF
};

#define RENDERBORDER(value, border)	\
 	switch (value) {				\
	case 16: *scr++ = border;		\
	case 15: *scr++ = border;		\
	case 14: *scr++ = border; 		\
	case 13: *scr++ = border;		\
	case 12: *scr++ = border;		\
	case 11: *scr++ = border;		\
	case 10: *scr++ = border;		\
	case  9: *scr++ = border;		\
	case  8: *scr++ = border;		\
	case  7: *scr++ = border;		\
	case  6: *scr++ = border;		\
	case  5: *scr++ = border;		\
	case  4: *scr++ = border;		\
	case  3: *scr++ = border;		\
	case  2: *scr++ = border; 		\
	case  1: *scr++ = border;		\
	}

class V9938;

/*!
 * The V9938renderer class contains the V9938 screen mode methods 
 */
class V9938renderer {

private:

        V9938 			*vdp;
		nw_byte         *vram;
		unsigned int 	*spriteOverlay;
		unsigned int	*MSXscreen;
    	
    	unsigned int	RGBpalette[16];
    	unsigned int	RGBgraphic7[256];
		unsigned int 	YJK2RGB[32*64*64];
		unsigned int	paletteColor0;
		bool			transparent;
		bool			inTextMode;
		
		// gemeenschappelijke variabelen voor de dumpscreens
		unsigned int	nameTable;
		unsigned int	displayOffset;
		unsigned int	*scr;
		bool			spritesEnabled;
		unsigned int	screenMode;
		
		unsigned int	colorTable;
		unsigned int	backDropColor;
		unsigned int	textColor;
		unsigned int	patternGeneratorTable;
		unsigned int	colorRegister;
		unsigned int	blinkColor;
		unsigned int	blinkText;
		unsigned int	blinkBackDrop;
		bool			blinking;
		unsigned int	blinkPeriod;
		unsigned int	blinkTimer;
		
		void			checkTransparency();
					    V9938renderer();

		
public:
		static 		V9938renderer* Instance();
					~V9938renderer();
		void		setVdp(V9938*);
		void 		reset();
		
		void 		writePalette(unsigned int, nw_byte, nw_byte);
		void		setNameTable(nw_byte);
		void		setModeRegister1(nw_byte);
		void		setModeRegister2(nw_byte);
		void		setDisplayOffset(nw_byte);
		void		setColorTableLow(nw_byte);
		void		setColorTableHigh(nw_byte);
		void		setPatternGeneratorTable(nw_byte);
		void		setBorderColor(nw_byte);
		void		setBlinkColor(nw_byte);
		void		setBlinkPeriod(nw_byte);
    
		void		dumpText1(unsigned int, unsigned int);
		void		dumpText2(unsigned int, unsigned int);
		void		dumpMultiColor(unsigned int, unsigned int);
		void		dumpGraphic1(unsigned int, unsigned int);
		void		dumpGraphic2(unsigned int, unsigned int);
		void		dumpGraphic4(unsigned int, unsigned int);
		void		dumpGraphic5(unsigned int, unsigned int);
		void		dumpGraphic6(unsigned int, unsigned int);
		void		dumpGraphic7(unsigned int, unsigned int);
		void		dumpGraphic7yjk(unsigned int, unsigned int);
		
		std::string copyFromText2(unsigned int x, unsigned int y, unsigned int w, unsigned int h);
		
		void		dumpDisabledScreen256(unsigned int, unsigned int);
		void		dumpDisabledScreen512(unsigned int, unsigned int);
		void		dumpDisabledScreenG5(int, int);
		void		dumpDisabledScreenG7(int, int);
		void		callScaler();
		void		afterCallScaler();
};

#endif /* V9938RENDERER_H */

