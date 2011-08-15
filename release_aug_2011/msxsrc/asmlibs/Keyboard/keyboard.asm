
		MODULE	Keyboard

;------------------------------------------------
;--- Direct Key scan for Key input
;------------------------------------------------

; IN : C = Row number
; Out: A = Keys
GetDirectKey:
		PUSH	BC
		DI
		IN	A,(#AA) 
		LD	B,A
		AND	A,#F0
		ADD	A,C
		NOP
		OUT	(#AA),A
		IN	A,(#A9)
		LD	C,A
		LD	A,B
		EI
		OUT	(#AA),A
		LD	A,C
		POP	BC
		RET
	
		IN	A,(#AA) 
				
		ENDMODULE
		
