		ifndef Sfx_ASM
		define Sfx_ASM
		
		include "Sfx.inc" 
		 
		MODULE Sfx

Play:
; A  = sfx nr.
; B  = Panning
; C  = Volume
; D  = continuous
		
		
		LD	C,OPL4_CHN_CONTROL
		ADD	A,C
		LD	C,A
		LD	A,KEY_ON
		DI
		CALL	Opl4.WriteRegister		
		EI
		RET

		
Load:
		RET
				

Stop:

		; Calc sfx channel
		LD	C,OPL4_CHN_CONTROL
		ADD	A,C
		LD	C,A
		LD	A,DAMP
		DI
		CALL	Opl4.WriteRegister		
		EI
		RET
		ENDMODULE
		
		endif