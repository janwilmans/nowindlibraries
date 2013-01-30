			MODULE	Opl4
			
			include "opl4.inc"
;----------------------------------------------------------------------------;
; Opl4 general purpose functions (7.16Mhz and R800 compatible)               ;
;----------------------------------------------------------------------------;				
			
ReadRegister:
; Input  C = register
; Output A = Value
			LD	A,C
			OUT	(OPL4_WAVE_REG_SELECT),A
			PUSH	BC
			POP	BC
			IN	A,(OPL4_WAVE_REG_DATA)
			RET
			
WriteRegister:	
; Input C = register
;       A = Value
			EX	AF,AF
			LD	A,C
			PUSH	BC
			POP	BC
			OUT	(OPL4_WAVE_REG_SELECT),A
			EX	AF,AF
			PUSH	BC
			POP	BC
			OUT	(OPL4_WAVE_REG_DATA),A
			RET

SetSramAddress:		
; Input : EHL - Opl4 ram addres
			LD	C,2	; ENABLE SRAM ACCESS
			LD	A,10001B
			CALL	WriteRegister
			INC	C
			LD	A,E
			AND	111111B
			CALL	WriteRegister
			INC	C
			LD	A,H
			CALL	WriteRegister
			INC	C
			LD	A,L
			CALL	WriteRegister
			LD	A,6
			OUT	(OPL4_WAVE_REG_SELECT),A
			LD	C,OPL4_WAVE_REG_DATA
			RET
			
Detect:
; Output NZ = No opl4 detected, Z = opl4 found and put in opl4 mode
			LD	A,OPL4_REG_EXPANSION
			OUT	(OPL4_FM1_REG_SELECT),A
			PUSH	BC
			POP	BC
			LD	A,OPL4_ENABLE_OPL3_REG|OPL4_ENABLE_OPL4_REG
			OUT	(OPL4_FM1_REG_DATA),A
			PUSH	BC
			POP	BC
			IN	A,(OPL4_FM1_REG_DATA)
			CP	A,OPL4_ENABLE_OPL3_REG|OPL4_ENABLE_OPL4_REG
			RET	
			
 

GetSramSize:
; Output HL = Size of sram in KB
			LD	HL,0			; Loop door SRAM heen in blokken van 64kB
			LD	B,020h			; Maximaal 32 blokken van 64kB aanwezig (=2MB)
.writeLoop		
			LD	A,B			; B loopt van 32 tot 1
			ADD	A,01Fh
			LD	E,A			; E loopt van 32 tot 63 (sla eerste 2MB ROM over)
			CALL	Opl4.SetSramAddress	; Schrijf SRAM adres in Wave registers
			LD	C,06h
			LD	A,B
			CALL	Opl4.WriteRegister	; Schrijf waarde B (32 - 1) in 1e byte van SRAM
			DJNZ	.writeLoop		; Volgende 64kB SRAM blok
			
			LD	D,B			; D=0 (is maximaal aantal aanwezige 64kB blokken)
			LD	B,020h			; Controleer alle 32 SRAM blokken
.readLoop	
			LD	A,B
			ADD	A,01Fh
			LD	E,A			; Zie boven
			CALL	Opl4.SetSramAddress	; Scrijf SRAM adres in Wave registers
			LD	C,06h
			CALL	Opl4.ReadRegister	; Lees waarde uit 1ebyte van huidige SRAM blok
			CP	B			; Gelijk aan wat er oorspronkelijk ingezet is?
			JR	Z,.valueEqual
			LD	D,0			; Nee, dan maak D weer nul
			JR	.nextBlock		; Controleer volgende blok
.valueEqual		
			LD	A,D			; Wel de juiste waarde? Dan:
			OR	A			; Als het VORIGE blok een foute waarde teruggaf...
			JR	NZ,.nextBlock		; ...dan is het huidige blok het eerst mogelijke
			LD	D,B			; juiste SRAM blok; in D
.nextBlock	
			DJNZ	.readLoop			; Controleer volgende SRAM blok
			
			SRL	D			; Minimale blok eenheid is 128kB
			LD	L,0
			LD	H,D
			SRL	H
			RR	L
			RET
			
			
			ENDMODULE