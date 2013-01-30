		ifndef MOUSE_ASM
		define MOUSE_ASM
		
		MODULE Mouse
		
		include "mouse.inc"
InitDelays:
; A = 0, init for z80 1, init for r800		
		LD	HL,D_Z80
		OR	A,A
		JR	Z,$+5
		   LD   HL,D_R800
		LD	(delay+1),HL
		RET

Install:
; A = 0, init for z80 1, init for r800	
; Cy = 0, mouse found
		CALL	InitDelays
		LD	A,MOUSE_PORT_1
		LD	(MOUSE_PORT),A
		CALL	Check
		RET	NC
		LD	A,MOUSE_PORT_2
		LD	(MOUSE_PORT),A
		CALL	Check
		RET	NC
		LD	A,0
		LD	(MOUSE_PORT),A
		RET		
		
Check:
         	LD	B,40		; check 40 times
Check.0:
		PUSH	BC
		CALL	GetXY
		CP	A,1		; Y-off
		JP	NZ,Check.1
		LD	A,L		; X-off
		CP	A,1
		JP	NZ,Check.1
		POP	BC
		DJNZ	Check.0
		SCF			; Cy:1 [not found]
		RET
Check.1:
		POP	BC		; found
		AND	A,A		; Cy:0
		RET



; Get xy data (offsets)
; Out: HL, YYXX offsets
GetXY:          LD	HL,0
		LD	A,(MOUSE_PORT)
		OR	A,A
		RET	Z
		LD	DE,(MOUSE_PORT)
		LD	A,15	; Read PSG r15 port B
		OUT	(PSG_REG_SELECT),A
		IN	A,(PSG_REG_READ)
		AND	A,%10001111	; interface 1
		OR	A,E			; mouse NR
		LD	E,A

; X offset
		LD	BC,40 * 256 + 20	; delay Z80 ; Delay R800
		CALL	GPAD.2
		CALL	GPAD.0
		LD	L,A
; Y offset
		CALL	GPAD.1
		CALL	GPAD.0
		LD	H,A
		RET

GPAD.0:
		RLCA
		RLCA
		RLCA
		RLCA
		LD	D,A
		CALL	GPAD.1
		OR	A,D
		NEG
		RET

GPAD.1:
		LD	BC,0706h	; delays
GPAD.2:	
		LD	A,15
		OUT	(PSG_REG_SELECT),A
		LD	A,E
		OUT	(PSG_REG_WRITE),A
	
		LD	A,(MOUSE_PORT)
		AND	A,30h
		XOR	A,E
		LD	E,A

delay:		CALL	D_Z80		
		
		LD	A,14
		OUT	(PSG_REG_SELECT),A
		IN	A,(PSG_REG_READ)
		AND	A,0Fh
		RET

; - Delay routines -
; Z80. In: B, delay
D_Z80:		
		DJNZ	D_Z80
		RET
		
; R800. In: C, delay
D_R800:
		IN	A,(TURBO_R_SYS_TIMER_LSB)	; Timer
		LD	B,A
DR80.0:	
		IN	A,(TURBO_R_SYS_TIMER_LSB)
		SUB	A,B
		CP	A,C
		JP	C,DR80.0
		RET
		
MOUSE_PORT      DB	0
				
		ENDMODULE
		        
		endif
