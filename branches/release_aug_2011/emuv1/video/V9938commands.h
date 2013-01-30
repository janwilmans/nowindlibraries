//! V9938commands.h
#ifndef V9938COMMANDS_H
#define V9938COMMANDS_H

#include "msxtypes.h"

#define GRAPHICMODE4 0x06
#define GRAPHICMODE5 0x08
#define GRAPHICMODE6 0x0A
#define GRAPHICMODE7 0x0E

#define MAJ 0x01
#define EQ 0x02
#define DIX 0x04
#define DIY 0x08


enum { IMP,  AND,  OR,  EOR,  NOT, INV5, INV6, INV7,
	   TIMP, TAND, TOR, TEOR, TNOT };

class V9938;

/*!
 * The V9938commands class contains the V9938 copy/draw commands methods
 */
class V9938commands {

private:

		nw_byte 	*vram;
		nw_byte	   *statusRegister;
		nw_byte	   *modeRegister0;

		// vdp command registers 32/46
		unsigned int sxl, sxh, syl, syh;
  		unsigned int dxl, dxh, dyl, dyh;
    	unsigned int nxl, nxh, nyl, nyh;
		unsigned int color, argument, command;
		
		unsigned int sx, sy, dx, dy, nx, ny;
		unsigned int borderX;
        unsigned int screenMode;

        // commands
		void       commandPoint();
		void       commandPset();
		void       commandSearch();
		void       commandLine();
		void       commandLmmv();
		void       commandLmmm();
		void       commandLmcm();
		void       commandLmmc();
		void       commandHmmv();
		void       commandHmmm();
		void       commandYmmm();
		void       commandHmmc();

		void       executeCommand();
		void       transferLmmc();
		void       transferHmmc();
		
		// TODO: moeten deze int's geen unsigned ints worden??!!!!!!
		void       psetG4(int, int, nw_byte);
		void       psetG5(int, int, nw_byte);
		void       psetG6(int, int, nw_byte);
		void       psetG7(int, int, nw_byte);
		
		void 	   psetG4highspeed(int, int, nw_byte);
		void 	   psetG5highspeed(int, int, nw_byte);
		void 	   psetG6highspeed(int, int, nw_byte);
		void 	   psetG7highspeed(int, int, nw_byte);

		nw_byte       pointG4(int, int);
		nw_byte       pointG5(int, int);
		nw_byte       pointG6(int, int);
		nw_byte       pointG7(int, int);

		void       logicalOperation(int, nw_byte, nw_byte);
		void       commandDone();

		unsigned int        tx;
		unsigned int        ty;
		unsigned int        _nx;
		unsigned int        _ny;
		unsigned int        adrp;

public:

		V9938commands(V9938*);
		~V9938commands();
		void		reset();

		int			getBorderCoordinate();
		void        writeCommandRegister(nw_byte, nw_byte);
		nw_byte        transferLmcm();

};

#endif /* V9938COMMANDS_H */

