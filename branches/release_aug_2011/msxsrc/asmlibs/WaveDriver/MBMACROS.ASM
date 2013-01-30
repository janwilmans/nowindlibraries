	

; --- Nic macro's ---

	macro	BIOS adres
	ld	iy,(0fcc1h)
	ld	ix,adres
	call	01ch
	endm

	macro	BDOS callnr
	ld	c,callnr
	call	bdoscall
	endm

;--- extra assembly commands ---

	macro   add_hl_a			; add A to HL
	add	a,l	; notice: A is modified!
	jr	nc,$+3
	inc	h
	ld	l,a
	endm

	macro add_de_a			; add A to DE
	add	a,e	; notice: A is modified!
	jr	nc,$+3
	inc	d
	ld	e,a
	endm

	macro ld_bc bv,cv
	ld	bc, 256 * bv + cv
	endm


	macro opl4_wait	
	IF	R800ASM
	push	bc
	pop	bc
	ENDIF
	endm
	macro opl4_7mhz	
	IF	Z80HASM
	nop
	ENDIF
	endm
