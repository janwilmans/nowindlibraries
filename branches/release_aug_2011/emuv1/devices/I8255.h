//! I8255.h
#ifndef I8255_H
#define I8255_H

#include <iostream>
#include "msxtypes.h"

class I8255 {
	
private:
	
	nw_byte control;

	nw_byte	latchA;
	nw_byte	latchB;
	nw_byte	latchC;

	bool   inputModePortA;
	bool   inputModePortB;
	bool   inputModePortC_lower;
	bool   inputModePortC_upper;

    void    outputPortA();
    void    outputPortB();
    void    outputPortC_lower();
    void    outputPortC_upper();
    
public:
	
	I8255();
	~I8255();
	void 	reset();

	void 	writePortA(nw_byte);
	void 	writePortB(nw_byte);
	void 	writePortC(nw_byte);
	void 	writeControlRegister(nw_byte);

	nw_byte	readPortA();
	nw_byte	readPortB();
	nw_byte	readPortC();
    nw_byte	readControlRegister();
};

#endif

