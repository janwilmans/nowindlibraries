
		ifndef STRING_ASM
		define STRING_ASM
		
		MODULE String


                
                        
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
		ld	a,(de)		; get character from second string
		cp	(hl)
		ret	nz		; exit if strings not equal
		
		or	a               ; end of string?
		ret	z		; quit if string is terminated
		
		inc	hl
		inc	de
		jr	StringCompareZ	; check next characters


StringCompare:
;  input  HL = String 1
;         DE = String 2
;          B = Length string
;  output  nz = not equal  z=equal

		LD	A,(DE)
		CP      A,(HL)
                RET     NZ
                INC     DE
                INC     HL
                DJNZ    StringCompare
                RET             
		
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
				
		ENDMODULE
		
		endif
		    
