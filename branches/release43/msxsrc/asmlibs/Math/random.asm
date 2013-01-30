		MODULE	Random

RANDOM_PATTERNH: 	equ #B5
RANDOM_PATTERNL: 	equ #AD

;In: RandomSeed
;Out: A = random value 0-63
; RandomSeed updated
;Chg: AF
RandomNext:
		push	bc
		push	hl
		ld 	hl,(.randomSeed)
		ld 	c,l
		ld 	b,h
		add 	hl,hl ;2x
		add 	hl,bc ;3x
		add 	hl,hl ;6x
		add 	hl,bc ;7x
		add 	hl,hl ;14x
		add 	hl,hl ;28x
		add 	hl,bc ;29x

		ld 	a,l
		xor 	RANDOM_PATTERNL
		ld 	l,a
		ld 	a,h
		xor 	RANDOM_PATTERNH
		ld 	h,a
		ld	(.randomSeed),hl
		ld	a,l
		rrca
		rrca
		rrca
		xor	h
		pop	hl
		pop	bc
		ret

.randomSeed 	defw	1337

		ENDMODULE