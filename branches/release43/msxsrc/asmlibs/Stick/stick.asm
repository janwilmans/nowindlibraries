

		MODULE	Stick

PSGREG:	equ	#a0
PSGWDT:	equ	#a1
PSGRDT:	equ	#a2

PPIPRB:	equ	#a9
PPIPRC:	equ	#aa



GetStick:	xor	a
		call	get_stick
		or	a
		ret	nz
		ld	a,b
		or	c
		jr	nz,.triggers
	
	        ld	a,1
		call	get_stick
		or	a
		ret	nz
		ld	a,b
		or	c
		jr	nz,.triggers
	
		ld	a,2
		call	get_stick
		or	a
		ret	nz
		ld	a,b
		or	c
		jr	nz,.triggers
	        		
.triggers:	xor	a
		ret
				



get_stick:	ld	hl,joystick_type
		ld	c,a	;keep A
		add	a,a
		ADD_HL_A
		ld	a,c	;A back
		ld	c,(hl)
		inc	hl
		ld	h,(hl)
		ld	l,c
		jp	(hl)

joystick_type:
		defw	get_stick0,joy_player,joy_player,get_stick3


joy_player:     
;joystick 1/2
		cpl
		and	%00000001
		rrca
		rrca
		ld	b,a
		ld	c,PSGREG
		in	h,(c)
		ld	a,15
		out	(c),a
		in	a,(PSGRDT)
		and	%10111111
		or	b
		out	(PSGWDT),a
		ld	a,14
		out	(c),a
		in	a,(PSGRDT)
		out	(c),h

		ld	l,a
		and	%00001111

		ld	c,a
		ld	b,0
		ld	a,l
		ld	hl,DIRTBJ
		add	hl,bc

		cpl
		and	%00110000
		add	a,a
		add	a,a
		add	a,a
		rl	b

		rlca
		ld	c,a
		ld	a,(hl)	;richting
		ret

;richting-tabel voor joysticks
DIRTBJ:		defb	0,5,1,0,3,4,2,3,7,6,8,7,0,5,1,0

;lees keyboard uit
get_stick0:	in	a,(PPIPRC)
		ld	h,a
		and	%11110000
		ld	l,a

		or	4
		out	(PPIPRC),a
		in	a,(PPIPRB)
		xor	255
		and	%00001100	;M/N=vuurknop 1

		ld	b,a	;trigger1

		ld	a,8
		or	l
		out	(PPIPRC),a
		in	a,(PPIPRB)
		ld	l,a
		and	1
		xor	1
		ld	c,a	;trigger2
		push	bc

		ld	a,%11110000
		and	l
		rrca
		rrca
		rrca
		rrca
		ld	c,a
		ld	b,0
		ld	a,h
		ld	hl,DIRTBC
		add	hl,bc
		out	(PPIPRC),a
		ld	a,(hl)
		pop	bc

		ret

;lees keyboard 2 uit
; SHIFT=trigger1, CTRL=trigger2
get_stick3:	in	a,(PPIPRC)
		ld	h,a	;effe bewaren
		and	%11110000
		ld	l,a	;alleen bit 7-4 bewaren
		or	6
		out	(PPIPRC),a
		in	a,(PPIPRB)
		cpl
		ld	bc,0
		rrca
		rl	c	;trigger 1
		rrca
		rl	b	;trigger 2

		push	bc	;effe bewaren

		ld	a,l
		or	2
		out	(PPIPRC),a	;lees kolom met 'A' uit
		in	a,(PPIPRB)
		cpl
		and	%01000000
		ld	c,a
		
		ld	a,l
		or	4
		out	(PPIPRC),a
		in	a,(PPIPRB)      ;lees kolom met 'Q' uit (voor AZERTY keyboard support)
		cpl
		and	%01000000
		or	c
		rrca
		rrca
		rrca
		rrca
		rrca
		rrca
		ld	b,a

		ld	a,l
		or	3
		out	(PPIPRC),a
		in	a,(PPIPRB)	;lees kolom met 'D' uit
		cpl
		and	%00000010
		rlca
		rlca
		or	b
		ld	b,a

		ld	a,l
		or	5
		out	(PPIPRC),a
		in	a,(PPIPRB)	;kolom met S
		cpl
		ld	l,a
		and	1
		add	a,a
		add	a,a
		or	b
		ld	b,a

		ld	a,l              ;W en Z (voor AZERTY keyboard layout)
		and	%10010000
		cp	128
		jr	c,.skip
		ld	a,16
.skip:		
		rrca
		rrca
		rrca
		or	b
		xor	%00001111
		ld	c,a
		ld	b,0
		ld	a,h
		ld	hl,DIRTBC
		add	hl,bc
		out	(PPIPRC),a
		ld	a,(hl)
		pop	bc
		ret

DIRTBC:	defb	0,3,5,4,1,2,0,3,7,0,6,5,8,1,7,0


                
		ENDMODULE
		
		
