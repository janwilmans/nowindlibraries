
		IFNDEF	MACROS_INC
		DEFINE	MACROS_INC


;macros to load value conditionally
;load if not same
		MACRO	LD_NZ	REG,VALUE
		jr	z,1f
		ld	REG,VALUE
1		
		ENDM
		
		
;load if same
		MACRO	LD_Z	REG,VALUE
		jr	nz,1f
		ld	REG,VALUE
1
		ENDM
		
		
;load if smaller
		MACRO	LD_C	REG,VALUE
		jr	nc,1f
		ld	REG,VALUE
1		
		ENDM
		
		
;load if not smaller
		MACRO	LD_NC	REG,VALUE
		jr	c,1f
		ld	REG,VALUE
1
		ENDM
		
; following macros destroy register A!

;decrease BC and jump relative if non zero
		MACRO	DJRNZ_BC	LOCATION
		dec	bc
		ld	a,b
		or	c
		jr	nz,LOCATION
		ENDM
		
;decrease BC and jump if non zero
		MACRO	DJPNZ_BC	LOCATION
		dec	bc
		ld	a,b
		or	c
		jp	nz,LOCATION
		ENDM	
		
;decrease DE and jump relative if non zero
		MACRO	DJRNZ_DE	LOCATION
		dec	de
		ld	a,d
		or	e
		jr	nz,LOCATION
		ENDM
		
;decrease DE and jump if non zero
		MACRO	DJPNZ_DE	LOCATION
		dec	de
		ld	a,d
		or	e
		jp	nz,LOCATION
		ENDM					
		
;decrease HL and jump relative if non zero
		MACRO	DJRNZ_HL	LOCATION
		dec	hl
		ld	a,h
		or	l
		jr	nz,LOCATION
		ENDM
		
;decrease HL and jump if non zero
		MACRO	DJPNZ_HL	LOCATION
		dec	hl
		ld	a,h
		or	l
		jp	nz,LOCATION
		ENDM		
	
;add A to BC	
		MACRO 	ADD_BC_A
		add	a,c
		ld	c,a
		jr	nc,1f
		inc	b
1		
		ENDM
		
;add A to DE	
		MACRO 	ADD_DE_A
		add	a,e
		ld	e,a
		jr	nc,1f
		inc	d
1		
		ENDM					
		
;add A to HL	
		MACRO 	ADD_HL_A
		add	a,l
		ld	l,a
		jr	nc,1f
		inc	h
1		
		ENDM	
		
                MACRO LD_EHL  value
                LD      E,value >> 16
                LD      HL,value & 0xffff
                ENDM	
                          
		ENDIF
			
						
