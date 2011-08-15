EXTBIO 	  EQU	0FFCAh	;   Extended Bios entry

EXPTBL0   EQU 0FCC1h
EXPTBL1		EQU	0FCC2h
EXPTBL2		EQU	0FCC3h
EXPTBL3		EQU	0FCC4h

SLOT_ID_RAM_0   EQU     0F341h
SLOT_ID_RAM_1   EQU     0F342h
SLOT_ID_RAM_2   EQU     0F343h
SLOT_ID_RAM_3   EQU     0F344h

; BIOS

RDSLT           EQU     0Ch
CALSLT          EQU     1Ch
ENASLT          EQU			24h

CHGET           EQU     9Fh
KILBUF          EQU     156h


IDBYT2          EQU     2Dh

; Msx Versions
MSX_1		EQU	0
MSX_2		EQU	1
MSX_2_PLUS	EQU	2
MSX_TURBO_R	EQU	3

; Hooks
H.TIMI          EQU     0FD9Fh


		macro	BIOS adres	
		ld      iy,(EXPTBL0-1)       ;BIOS slot in iyh
		ld      ix,adres             ;address of BIOS routine
		call    CALSLT               ;interslot call
		endm

