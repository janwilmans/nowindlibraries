
		ifndef STRING_ASM
		define STRING_ASM
		
		MODULE String
                
		include "..\macros.inc"

;---------------------------------------------------------------------------------
                        
ByteToHex:
; HL = Pointer to buffer
;  A = number to convert to hex
		LD	B,A
		RRA
		RRA
		RRA
		RRA
		AND	A,#0F
		CP	A,10
		SBC	A,#69
		DAA
		LD	(HL),A
		INC	HL
		LD	A,B
		AND	A,#0F
		CP	A,10
		SBC	A,#69
		DAA
		LD	(HL),A
		INC	HL
		RET      
		
;---------------------------------------------------------------------------------

	
PrintDebug:	
; DE = Pointer to data
; B  = Bytes to print	
		PUSH	BC
		LD	A,(DE)
                INC	DE
                PUSH	DE
                LD	HL,.HEX_OUT
                CALL	ByteToHex                
    		LD	DE,.HEX_OUT
                BdosCall _STROUT
                POP	DE
                POP	BC
                DJNZ	PrintDebug 
                RET	
                
.HEX_OUT:	DS	2,0
		DB	"$"

;---------------------------------------------------------------------------------

; Function: StringCompare
; Purpose: Compares two zero-terminated strings
; Parameters: DE: pointer to first string
;     	      HL: pointer to second string
; Result: flags: Z: strings are equal
;		 NZ: strings are not equal
;			C: first string < second string  
;			NC: first string > second string
; Modifies: all
StringCompareZ:	
		LD	A,(DE)		; get character from second string
		CP	A,(HL)
		RET	NZ		; exit if strings not equal
		
		OR	A               ; end of string?
		RET	Z		; quit if string is terminated
		
		INC	HL
		INC	DE
		JR	StringCompareZ	; check next characters

;---------------------------------------------------------------------------------

;  input  HL = String 1
;         DE = String 2
;          B = Length string
;  output  nz = not equal  z=equal
StringCompare:
		LD	A,(DE)
		CP      A,(HL)
                RET     NZ
                INC     DE
                INC     HL
                DJNZ    StringCompare
                RET             

;---------------------------------------------------------------------------------
		
; Function: StringCopy
; Purpose:  copies a string
; input:     HL: pointer to string
;            DE: Destination, DE is pointing to the zero termination char after copying     
; Modifies: AF,HL,DE

StringCopy:
		LD	A,(HL)
		LD	(DE),A
		OR	A,A
		RET	Z	; End of string
		INC	DE
		INC	HL		
		JP	StringCopy

;---------------------------------------------------------------------------------
		
; Function: PrintAsciiz
; Purpose: Prints a zero terminated string
; Parameters: DE: pointer to string
; Result: flags: 
; Modifies: AF,HL,DE

PrintAsciiz:

		EX	DE,HL
.loop
		LD	A,(HL)
		OR	A,A
		RET	Z
		PUSH	HL
		LD	E,A
		BdosCall _CONOUT
		POP	HL
		INC	HL
		JP 	.loop
		
;---------------------------------------------------------------------------------
; Function: StringLenght
; Purpose: return the lenght of an ASCIIZ string in register B
; Parameters: DE: pointer to string
;    Output : B string lenght
;           : DE = pointer to term char
; Result: flags: 
; Modifies: AF,HL,DE

StringLenght:
		LD	B,0
.loop:		
		LD	A,(DE)
		OR	A,A
		RET	Z
		INC	DE
		INC	B
		JP	.loop


;---------------------------------------------------------------------------------
; Function: UnsignedInt16ToStr
; Purpose:  Converts an unsigned 16 bit int to a string
; Parameters: HL: value to convert
;    Output : UnsignedInt16ToStr.STRING, string containing converted string
; Result: flags: 
; Modifies: AF,HL,DE

UnsignedInt16ToStr:
			PUSH	HL
			PUSH	DE
			PUSH	BC
			LD	DE,.STRING+1
			CALL	.Convert
			XOR	A,A
			LD	(DE),A
			LD	HL,.STRING
			LD	(HL),' '
			POP	BC
			POP	DE
			POP	HL
			RET

.Convert
			XOR	A
			LD	B,16
			LD	C,A
.a	
			ADD	HL,HL
			RL	C
			LD	A,C
			SUB	A,10
			JR	C,.b
			LD	C,A
			INC	HL
.b
			DJNZ	.a
			
			LD	A,C
			ADD	A,'0'
			PUSH	AF
			LD	A,H
			OR	L
			CALL	NZ,.Convert
			POP	AF
			LD	(DE),A
			INC	DE
			RET

.STRING			DS	10


;---------------------------------------------------------------------------------
; Function: StrToUnsignedInt16
; Purpose:  Converts a string to an unsigned 16 bit int 
; Parameters: DE: pointer to zero terminated string
;    Output : HL valid value if nc = 0
; Result: flags: C = 1, invalid char in string, or overflow
; Modifies: AF,BC,DE,HL
StrToUnsignedInt16:
			LD	HL,0
.loop
			LD	A,(DE)
			INC	DE
			OR	A,A
			RET	Z
		
			; Check character
			SUB	A,'0'		
			RET	C
			CP	A,10 
			RET	NC
			
			; HL * 10
			ADD	HL,HL ; * 2
			LD	B,H
			LD	C,L
			ADD	HL,HL ; * 4
			ADD	HL,HL ; * 8
			ADD	HL,BC	
			ADD_HL_A		
			JP 	.loop					
		ENDMODULE
		
		endif
		    
