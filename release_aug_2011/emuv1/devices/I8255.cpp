// I8255.ccp


#include "stdio.h"
#include "I8255.h"
#include "Debug.h"
#include "SlotSelector.h"
#include "controls/Keyboard.h"
#include "audio/PPISound.h"

// All of the output registers, including the status flip-flops,
// will be reset whenever the mode is changed.

// TODO: misschien onderscheid maken tussen out/inputs en latches
// - wat gebeurt er als je leest van een port die als output geschakeld is (en anders om)
// - onderscheid maken tussen de verschillende modes in read en write van de poort?

using namespace std;

I8255::I8255() {

	DBERR("I8255 constructor...\n");
	DBERR("I8255 constructor...finished\n");
}

I8255::~I8255() {
	
	DBERR("I8255 destroyed.\n");
}

void I8255::reset() {

	latchA = 0;  // TODO: check!
	latchB = 0;
	latchC = 0;
	control = 0;
	
	inputModePortA = 0;
	inputModePortB = 0;
	inputModePortC_lower = 0;
	inputModePortC_upper = 0;
}

void I8255::writeControlRegister(nw_byte value) {
	
	if (value & 0x80) {

        outputPortA();
        outputPortB();
        outputPortC_lower();
        outputPortC_upper();

		control = value;
		
		switch (control & 0x60) {
		case 0x00:
			// group A mode 0
			inputModePortA = (control & 0x08) ? true:false;
			inputModePortC_lower = (control & 0x01) ? true:false;
			break;
		case 0x20:
			// group A mode 1
		default:
			// group A mode 2
			DBERR("I8255: Group A Mode 1&2 not supported!\n");
			break;
		}
		
		if (control & 0x04) {
			// group B mode 1
			DBERR("I8255: Group B Mode 1 not supported!\n");			
		} else {
			// group B mode 0
			inputModePortB = (control & 0x02) ? true:false;
			inputModePortC_lower = (control & 0x01) ? true:false;
		}

	} else {
		// bit set/reset (only in mode 0?)
		// Checked on 8250: In mode 0, setting a bit affects the latch. When
		// reading port C (even though in output mode), the value will be changed.
		
		unsigned int mask = 1 << ((value >> 1) & 7);

		if (value & 1) {
			latchC |= mask;
		} else {
			latchC &= ~mask;
		}
		outputPortC_lower();
		outputPortC_upper();
	}
}

nw_byte I8255::readControlRegister() {
	
	return (control | 0x80);
}

void I8255::writePortA(nw_byte value) {

	latchA = value;
	if (!inputModePortA) outputPortA();        // TODO: na reset moet port niet als input staan? dan kan A8 namelijk geen rom inschakelen? nakijken...
}

void I8255::writePortB(nw_byte value) {
	
	latchB = value;
	if (!inputModePortB) outputPortB();
}

void I8255::writePortC(nw_byte value) {

	latchC = value;
	if (!inputModePortC_lower) outputPortC_lower();
	if (!inputModePortC_upper) outputPortC_upper();
}


nw_byte I8255::readPortA() {

	if (inputModePortA) {
        // TODO: check of deze waarde ook in latchA moet worden geschreven (ook bij port B & C)
        return 0xff;
    } else {
        return latchA;
    }
}

nw_byte I8255::readPortB() {

	if (inputModePortB) {
        return Keyboard::Instance()->readKeyboardLine();
    } else {
        return latchB;
    }
}

nw_byte I8255::readPortC() {

    // TODO: check!!!
    return latchC;
}

void I8255::outputPortA() {

    SlotSelector::Instance()->setMainSlot(latchA);
}

void I8255::outputPortB() {

    // Nothing connected on MSX
}

void I8255::outputPortC_lower() {

	Keyboard::Instance()->selectKeyboardLine(latchC & 15);
}

void I8255::outputPortC_upper() {

	PPISound::Instance()->write((latchC & 0x80) != 0);
    // TODO: cassete motor/write, caps
}

