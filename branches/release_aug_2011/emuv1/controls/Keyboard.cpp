#include "stdio.h"

#include "Keyboard.h"
#include "Debug.h"
#include "cpu/Z80.h"

using namespace std;
 
Keyboard::Keyboard() {

	DBERR("Keyboard constructor...\n");

	for(int i=0;i<16;i++) keyboardMatrix[i] = 0xFF;

	DBERR("Keyboard constructor...finished\n");

}

Keyboard *Keyboard::Instance() {

	// implies singleton class
	static Keyboard deInstantie;
	return &deInstantie;
}

Keyboard::~Keyboard() {
    
	DBERR("Keyboard destroyed.\n");
}



void Keyboard::selectKeyboardLine(nw_byte value) {

	keyboardLine = value & 0x0f;
}

nw_byte Keyboard::readKeyboardLine() {

	return keyboardMatrix[keyboardLine];
}

#define SETKB(ln, col) line = ln; column = col

void Keyboard::modifyMatrix(SDL_KeyboardEvent keyevent) {

	// DBERR("Keyboad modifyMatrix: " << (int)key << endl);
	int line = 0;
	int column = 0;
	
	switch (keyevent.keysym.sym) {

	case SDLK_7:                SETKB(0,7); break;
	case SDLK_6:                SETKB(0,6); break;
	case SDLK_5:                SETKB(0,5); break;
	case SDLK_4:                SETKB(0,4); break;
	case SDLK_3:                SETKB(0,3); break;
	case SDLK_2:                SETKB(0,2); break;
	case SDLK_1:                SETKB(0,1); break;
	case SDLK_0:                SETKB(0,0); break;

	case SDLK_SEMICOLON:        SETKB(1,7); break;
	case SDLK_RIGHTBRACKET:     SETKB(1,6); break;
	case SDLK_LEFTBRACKET:      SETKB(1,5); break;
	case SDLK_BACKSLASH:        SETKB(1,4); break;
	case SDLK_EQUALS:           SETKB(1,3); break;
	case SDLK_MINUS:            SETKB(1,2); break;
	case SDLK_9:                SETKB(1,1); break;
	case SDLK_8:                SETKB(1,0); break;

	case SDLK_b:                SETKB(2,7); break;
	case SDLK_a:                SETKB(2,6); break;
	// dead key
	case SDLK_SLASH:            SETKB(2,4); break;
	case SDLK_PERIOD:           SETKB(2,3); break;
	case SDLK_COMMA:            SETKB(2,2); break;
	case SDLK_BACKQUOTE:        SETKB(2,1); break;
	case SDLK_QUOTE:            SETKB(2,0); break;

	case SDLK_j:                SETKB(3,7); break;
	case SDLK_i:                SETKB(3,6); break;
	case SDLK_h:                SETKB(3,5); break;
	case SDLK_g:                SETKB(3,4); break;
	case SDLK_f:                SETKB(3,3); break;
	case SDLK_e:                SETKB(3,2); break;
	case SDLK_d:                SETKB(3,1); break;
	case SDLK_c:                SETKB(3,0); break;

	case SDLK_r:                SETKB(4,7); break;
	case SDLK_q:                SETKB(4,6); break;
	case SDLK_p:                SETKB(4,5); break;
	case SDLK_o:                SETKB(4,4); break;
	case SDLK_n:                SETKB(4,3); break;
	case SDLK_m:                SETKB(4,2); break;
	case SDLK_l:                SETKB(4,1); break;
	case SDLK_k:                SETKB(4,0); break;

	case SDLK_z:                SETKB(5,7); break;
	case SDLK_y:                SETKB(5,6); break;
	case SDLK_x:                SETKB(5,5); break;
	case SDLK_w:                SETKB(5,4); break;
	case SDLK_v:                SETKB(5,3); break;
	case SDLK_u:                SETKB(5,2); break;
	case SDLK_t:                SETKB(5,1); break;
	case SDLK_s:                SETKB(5,0); break;

	case SDLK_F3:               SETKB(6,7); break;
	case SDLK_F2:               SETKB(6,6); break;
	case SDLK_F1:               SETKB(6,5); break;
	case SDLK_RALT:             SETKB(6,4); break; // CODE (MSX)
	case SDLK_CAPSLOCK:         SETKB(6,3); break;
	case SDLK_LALT:             SETKB(6,2); break; // GRAPH (MSX)
	case SDLK_LCTRL:
	case SDLK_RCTRL:            SETKB(6,1); break;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:           SETKB(6,0); break;

	case SDLK_RETURN:
	case SDLK_KP_ENTER:         SETKB(7,7); break;
	case SDLK_END:              SETKB(7,6); break; // SELECT (MSX)
	case SDLK_BACKSPACE:        SETKB(7,5); break;
	case SDLK_PAUSE:            return;    // weird behaviour
    case SDLK_PAGEUP:           SETKB(7,4); break; // STOP (MSX)
	case SDLK_TAB:              if (keyevent.keysym.mod & KMOD_ALT) return; // this prevents GRAPH and TAB from being pressed together in the MSX
                                SETKB(7,3); break;
	case SDLK_ESCAPE:           SETKB(7,2); break;
	case SDLK_F5:               SETKB(7,1); break;
	case SDLK_F4:               SETKB(7,0); break;

	case SDLK_RIGHT:            SETKB(8,7); break;
	case SDLK_DOWN:             SETKB(8,6); break;
	case SDLK_UP:               SETKB(8,5); break;
	case SDLK_LEFT:             SETKB(8,4); break;
	case SDLK_DELETE:           SETKB(8,3); break;
	case SDLK_INSERT:           SETKB(8,2); break;
	case SDLK_HOME:             SETKB(8,1); break;
	case SDLK_SPACE:            SETKB(8,0); break;

	case SDLK_KP4:              SETKB(9,7); break;
	case SDLK_KP3:              SETKB(9,6); break;
	case SDLK_KP2:              SETKB(9,5); break;
	case SDLK_KP1:              SETKB(9,4); break;
	case SDLK_KP0:              SETKB(9,3); break;
	case SDLK_KP_DIVIDE:        SETKB(9,2); break;
	case SDLK_KP_PLUS:          SETKB(9,1); break;
	case SDLK_KP_MULTIPLY:      SETKB(9,0); break;

	case SDLK_KP_PERIOD:        SETKB(10,7); break;
	// No KP_COMMA key on PC keyboards	SETKB(10,6); break;
	case SDLK_KP_MINUS:         SETKB(10,5); break;
	case SDLK_KP9:              SETKB(10,4); break;
	case SDLK_KP8:              SETKB(10,3); break;
	case SDLK_KP7:              SETKB(10,2); break;
	case SDLK_KP6:              SETKB(10,1); break;
	case SDLK_KP5:              SETKB(10,0); break;

//	case SDLK_PAGEUP:           SETKB(11,1); break; // TURBO-R YES , SDLK_PAGEUP used for STOP
	case SDLK_PAGEDOWN:         SETKB(11,3); break; // TURBO-R NO
	default:
        DBERR("Key not handled by MSX!!! (%s)\n", SDL_GetKeyName(keyevent.keysym.sym));
        return;
	}
	
	if (keyevent.type == SDL_KEYDOWN) {
		keyboardMatrix[line] &= (~(1 << column));
	} else {
		keyboardMatrix[line] |= (1 << column);
	}
}

void Keyboard::modifyMatrixAscii(char key, bool keyDown) {

	int line = 0;
	int column = 0;
	bool shift = false;

    if (key >= 65 && key <= 90) shift = true; // capital letters
		
	switch (tolower(key)) {

	case '7':                SETKB(0,7); break;
	case '&':                SETKB(0,7); shift = true; break;
	case '6':                SETKB(0,6); break;
	case '^':                SETKB(0,6); shift = true; break;
	case '5':                SETKB(0,5); break;
	case '%':                SETKB(0,5); shift = true; break;
	case '4':                SETKB(0,4); break;
	case '$':                SETKB(0,4); shift = true; break;
	case '3':                SETKB(0,3); break;
	case '#':                SETKB(0,3); shift = true; break;
	case '2':                SETKB(0,2); break;
	case '@':                SETKB(0,2); shift = true; break;
	case '1':                SETKB(0,1); break;
	case '!':                SETKB(0,1); shift = true; break;
	case '0':                SETKB(0,0); break;
	case ')':      			 SETKB(0,0); shift = true; break;
	
	case ';':                SETKB(1,7); break;
	case ':':                SETKB(1,7); shift = true; break;
	case '\\':               SETKB(1,4); break;
	case '|':                SETKB(1,4); shift = true; break;
	case '=':                SETKB(1,3); break;
	case '+':                SETKB(1,3); shift = true; break;
	case '-':                SETKB(1,2); break;
	case '_':                SETKB(1,2); shift = true; break;
	case '9':                SETKB(1,1); break;
	case '(':			     SETKB(1,1); shift = true; break;
	case '8':                SETKB(1,0); break;
	case '*':                SETKB(1,0); shift = true; break;
	
	case 'b':                SETKB(2,7); break;
	case 'a':                SETKB(2,6); break;
	case '/':                SETKB(2,4); break;
	case '?':                SETKB(2,4); shift = true; break;
	case '.':                SETKB(2,3); break;
	case '>':                SETKB(2,3); shift = true; break;
	case ',':                SETKB(2,2); break;
	case '<':                SETKB(2,2); shift = true; break;
	case '`':                SETKB(2,1); break;
	case '~':                SETKB(2,1); shift = true; break;
	case '\'':               SETKB(2,0); break;
	case '\"':               SETKB(2,0); shift = true; break;
	
	case 'j':                SETKB(3,7); break;
	case 'i':                SETKB(3,6); break;
	case 'h':                SETKB(3,5); break;
	case 'g':                SETKB(3,4); break;
	case 'f':                SETKB(3,3); break;
	case 'e':                SETKB(3,2); break;
	case 'd':                SETKB(3,1); break;
	case 'c':                SETKB(3,0); break;

	case 'r':                SETKB(4,7); break;
	case 'q':                SETKB(4,6); break;
	case 'p':                SETKB(4,5); break;
	case 'o':                SETKB(4,4); break;
	case 'n':                SETKB(4,3); break;
	case 'm':                SETKB(4,2); break;
	case 'l':                SETKB(4,1); break;
	case 'k':                SETKB(4,0); break;

	case 'z':                SETKB(5,7); break;
	case 'y':                SETKB(5,6); break;
	case 'x':                SETKB(5,5); break;
	case 'w':                SETKB(5,4); break;
	case 'v':                SETKB(5,3); break;
	case 'u':                SETKB(5,2); break;
	case 't':                SETKB(5,1); break;
	case 's':				 SETKB(5,0); break;

	case '\n':				 SETKB(7,7); break;
	case '\t':               SETKB(7,3); break;
	case ' ':                SETKB(8,0); break;

	default:
		// convert the rest of the ascii values to spaces
		SETKB(8,0);
		break;
	}
	
	if (keyDown) {
		if (shift) keyboardMatrix[6] &= 0xfe;
		keyboardMatrix[line] &= (~(1 << column));
	} else {
		if (shift) keyboardMatrix[6] |= 1;
		keyboardMatrix[line] |= (1 << column);
	}
}
