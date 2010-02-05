; Product      : Math routines
; Version      : 0.1 
; Main code by : Arjan


		ifndef MATH_ASM
		define MATH_ASM
		
		MODULE	Math

; Function: MultiplyDEA	
; Purpose: Multiplies A with DE
; Parameters: A: multiplier
;	     DE: multiplicant
; Result: HL: 16 bit result
; Modifies: all		
MultiplyDEA:  
	 	ld	hl,0
.loop:		srl	a
		jr	z,.end
		jr	nc,.skip
		add	hl,de
.skip:		sla	e
		rl	d
		jr	.loop		
		
.end:		ret	nc
		add	hl,de
		ret	
			
; Function: HLDivDE	
; Purpose: Divides HL by DE
; Result HL = 
;        DE = Remainder
; 				
HLDivDE:
		LD	B,D	 
		LD	C,E
		LD	DE,0
		LD	A,16
HLDivDE.0:
		ADD	HL,HL
		EX	DE,HL
		ADC	HL,HL
		INC	DE
		OR	A
		SBC	HL,BC
		JR	NC,HLDivDE.1
		ADD	HL,BC
		DEC	DE
HLDivDE.1:
		EX	DE,HL
		DEC	A
		JR	NZ,HLDivDE.0
		RET		

; Function: HLMulDE	
; Purpose: Multiplies HL with DE
; Result HL 
; Modfies AF,BC,DE,HL
HLMulDE:
		LD	B,16
		LD	C,D
		LD	A,E

		EX	DE,HL
		LD	HL,0
HLMulDE.0	
		SRL	C
		RRA	
		JR	NC,HLMulDE.1
		ADD	HL,DE
HLMulDE.1
		SLA	E
		RL	D
		DJNZ	HLMulDE.0
		RET


; Function: BCMulDE32	
; Purpose:  Multiply 16-bit values (with 32-bit result)
; In: Multiply BC with DE
; Result BC:HL 
; Modfies AF,BC,HL 
 
BCMulDE32:
		LD A,C
		LD C,B
		LD HL,0
		LD B,16
.Loop
		ADD HL,HL
		RLA
		RL    C
		JR    NC,.NoAdd
		ADD   HL,DE
		ADC   A,0
		JP    NC,.NoAdd
		INC   C
.NoAdd
		DJNZ .Loop
		LD    B,C
		LD    C,A
		RET


CPHLDE
; Compare HL with DE
; Modifies A
		LD	A,H
		CP	A,D
		RET	NZ
		LD	A,L
		CP	A,E
		RET
		

	
		ENDMODULE
		
		endif
		
