// OnScreenDisplay.ccp

#include "OnScreenDisplay.h"
#include "cpu/Z80.h"
#include "Debug.h"
#include "Command.h"
//#include "SDL_image.h"

#include <stdio.h>

using namespace std;

OnScreenDisplay::OnScreenDisplay() {

	DBERR("OnScreenDisplay constructor...");

	osdTop = 100;
	osdLeft = 10;
	osdWidth = 522;
	osdHeight = 300;
	osdAlpha = 250;

    initOSD_Surface(osdWidth, osdHeight);
  	initializeFont();

/*    
    string backgroundImage = Debug::Instance()->getPath() + "osd/BrushedMetal.bmp";

    SDL_RWops *rwop = SDL_RWFromFile(backgroundImage.c_str(), "rb");
    SDL_Surface *image=IMG_LoadPNG_RW(rwop);

    if(!image) {
        DBERR("IMG_LoadPNG_RW: %s\n", IMG_GetError());
        DBERR("filename: %s\n", backgroundImage.c_str());
        exit(1);
    }
    
	background = SDL_DisplayFormat(image);
	SDL_FreeSurface(image);
*/
	osdEnabled = false;
	DBERR("OnScreenDisplay constructor...finished");
}

OnScreenDisplay * OnScreenDisplay::Instance() {

	/* implies singleton class */
	static OnScreenDisplay deInstantie;
	return &deInstantie;
}

OnScreenDisplay::~OnScreenDisplay() {
	
	DBERR("OnScreenDisplay destroyed.\n");
}

void OnScreenDisplay::initOSD_Surface(int osdWidth, int osdHeight) {
    
    osdSurface = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA, osdWidth, osdHeight, 32, 0, 0, 0, 0);
//	SDL_SetColorKey(osdSurface, SDL_SRCCOLORKEY, 0xff0000); // alleen nodig voor transparatie
    SDL_FillRect(osdSurface, NULL, 0xFF0000);
}

void OnScreenDisplay::initializeFont() {
	 
	 if(TTF_Init() == -1) {
	     DBERR("TTF_Init: %i", TTF_GetError());
		 assert(false);
	 }
	 
	 
//	 string filename = Debug::Instance()->getPath() + "freetype/FreeSans.ttf";
     string filename = Debug::Instance()->getPath() + "freetype/VeraMono.ttf";
     DBERR("loading font: %s\n", filename.c_str());
	 TTF_Font *font = TTF_OpenFont(filename.c_str(), 11);
	 if(!font) {
	     DBERR("TTF_OpenFont: %i", TTF_GetError());
	     assert(false);
	 }
	 
	 TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

     DBERR("fontAscent: %i\n", TTF_FontAscent(font));
     DBERR("fontDescent: %i\n", TTF_FontDescent(font));
     DBERR("fontLineSkip: %i\n", TTF_FontLineSkip(font));

     fontHeight = TTF_FontHeight(font); 
     DBERR("fontHeight: %i\n", fontHeight);
	 SDL_Color black = { 0, 0, 0, 0 };
fontHeight = TTF_FontAscent(font);
	 for (int i=0;i<128;i++) {
        glyph[i].surface = TTF_RenderGlyph_Blended(font, i, black);
        int fontMaxY;
        int fontAdvance;
        TTF_GlyphMetrics(font, i, NULL, NULL, NULL, &fontMaxY, &fontAdvance);
        glyph[i].advance = fontAdvance;
        glyph[i].maxY = fontMaxY;
         
	 }
	 TTF_CloseFont(font);
}

void OnScreenDisplay::toggleOSD(bool enable) {
	 
	 if (!enable) osdAlpha = 0;
	 SDL_EnableKeyRepeat(enable? SDL_DEFAULT_REPEAT_DELAY:0, SDL_DEFAULT_REPEAT_INTERVAL);
	 osdEnabled = enable; 
}

void OnScreenDisplay::renderOSD(list<string> *textBuffer) {
	 
	list<string>::iterator it;
	SDL_Rect dest, src;

    src.x = 0;
    src.y = 0;
    src.h = osdHeight;
    src.w = osdWidth;
    SDL_FillRect(osdSurface, NULL, 0xFF0000);
    //SDL_BlitSurface(background, &src, osdSurface, NULL);
    
	maxLines = 30;
    lastLine = 300;

	it = textBuffer->begin();

    for (int line=0;line<maxLines;line++,it++) {

		 if (it == textBuffer->end()) break;
		 int len = it->length();
		 dest.x = 0;
         for (int i=0;i<len;i++) {
		 	 int c = it->at(i);
		 	 if (c == 10) c = 32;
             if ((dest.x + glyph[c].advance) > osdWidth) break;
             dest.y = fontHeight + (fontHeight * line) - glyph[c].maxY;
             SDL_BlitSurface(glyph[c].surface, NULL, osdSurface, &dest);
             dest.x += glyph[c].advance;
		 }
    }
}

void OnScreenDisplay::displayOSD(SDL_Surface *screen) {
    
    // TODO: niet hier pas checken, uberhaupt niet aanroepen lijkt me beter
    if (!osdEnabled) return;
   
    list<string> *textBuffer = Command::Instance()->getUpdatedTextBuffer();
    if (textBuffer != NULL) renderOSD(textBuffer);

    SDL_Rect dest;
    dest.x = osdLeft;
    dest.y = osdTop;
    SDL_BlitSurface(osdSurface, NULL, screen, &dest);    
}

void OnScreenDisplay::xorBar(SDL_Surface *screen, int row) {

    unsigned int barHeight = 20;

	SDL_Surface *temp = SDL_CreateRGBSurface(SDL_SWSURFACE, osdWidth, barHeight, 32, 0x0,0x0,0x0,0);

	int colorkey = SDL_MapRGB(temp->format, 55, 255, 55);
	SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, colorkey);
	SDL_SetAlpha(temp, SDL_SRCALPHA | SDL_RLEACCEL, 175);

	SDL_FillRect(temp, NULL, 0x5F05FF);

	SDL_Rect dest, src;
	src.x = 0;
	src.y = 0;
	src.w = osdWidth;
	src.h = barHeight;
	
	dest.x = 0;
	dest.y = osdTop + (row*barHeight);
	
	SDL_BlitSurface(temp, &src, screen, &dest);

}

void OnScreenDisplay::setAlphaBlending(int alpha) {

    SDL_SetAlpha(osdSurface, SDL_SRCALPHA | SDL_RLEACCEL, alpha);   
}
