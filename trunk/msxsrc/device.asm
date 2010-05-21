; device.asm
; implements a basic now: device

device:
        push hl
        ld hl,deviceFunctions
        rrca
        inc a
        rlca
        add a,l
        ld l,a
        jr nc,.nocy
        inc h
.nocy:  ld a,(hl)
        inc hl
        ld h,(hl)
        ld l,a
        ex (sp),hl
        ret

deviceFunctions:
        dw identifyDevice               ; 0xff
        dw open                         ;  0
        dw close                        ;  2
        dw randomIO                     ;  4
        dw write                        ;  6
        dw read                         ;  8
        dw loc                          ; 10
        dw lof                          ; 12
        dw eof                          ; 14
        dw fpos                         ; 16
        dw putback                      ; 18

identifyDevice:
        DEBUGMESSAGE "identifyDevice"
        ld hl,deviceNameList
        call findStatementName
        ld a,(hl)                       ; device number
        ret                             ; carry is set when invalid device name

deviceNameList:
        db "NOW",0,0,0                  ; name, end marker, device number, dummy
        ;db "STDIN",0,1,0
        db 0

; Input     D   Global device code
;           E   File mode
;           HL  address fcb
open:
;        DEBUGMESSAGE "open"
;        DEBUGDUMPMEMHL 9
        ld (PTRFIL),hl
        call sendRegisters
        ld (hl),C_DEVICEOPEN
        ex de,hl
        ld bc,11
        ld hl,FILNAM
        ldir

        call enableNowindPage0
        call getHeaderInPage0
        jr c,deviceIoError              ; time out?
        or a
        jr nz,openError

        ld e,(hl)                       ; update fcb (including buffer)
        ld d,(hl)
        ld c,(hl)
        ld b,(hl)
        ldir
        jp restorePage0

deviceIoError:
        ld a,19

openError:
        call restorePage0
        ld e,a

basicError:
        ld ix,$406f
        ld iy,(EXPTBL-1)
        jp CALSLT

close:
;        DEBUGMESSAGE "close"
        call sendRegisters
        ld (hl),C_DEVICECLOSE
        ret

randomIO:
;        DEBUGMESSAGE "randomIO"
        ld e,61                         ; bad file mode
        jr basicError

write:
;        DEBUGMESSAGE "write"
        call sendRegisters
        ld (hl),C_DEVICEWRITE
        ret

read:
;        DEBUGMESSAGE "read"
        ld de,6
        add hl,de
        push hl
        ld e,(hl)
        inc hl
        inc hl
        inc hl
        add hl,de
        ld a,(hl)
        pop hl
        cp $1a
        scf
        ret z                           ; end of file
        ccf
        inc (hl)                        ; increment position
        ret nz                          ; buffer empty?

        push af
        dec hl
        inc (hl)                        ; increment position (high)
        ld de,-5
        add hl,de
        call sendRegisters
        ld (hl),C_DEVICEREAD
        call enableNowindPage0
        call getHeaderInPage0
        jr c,deviceIoError

        ld e,(hl)
        ld d,(hl)
        ld c,(hl)
        ld b,(hl)
        ldir                            ; update fcb buffer
        call restorePage0
        pop af                          ; return last character
        ret

eof:
;        DEBUGMESSAGE "eof"
        ld a,(hl)
        cp 1                            ; input mode?
        ld e,61                         ; bad file mode
        jp nz,basicError

        ld de,6
        add hl,de
        ld e,(hl)
        inc hl
        inc hl
        inc hl
        add hl,de
        ld a,(hl)
        sbc hl,hl
        cp $1a
        jr nz,.skip
        dec hl
.skip:  ld (DAC+2),hl
        ld a,2
        ld (VALTYP),a
        ret

loc:
        push hl
        pop iy
        ld l,(iy+6)
        ld h,(iy+5)
;        DEBUGASSERT
        ld (DAC+2),hl
        ld a,2
        ld (VALTYP),a
        ret

putback:
;        DEBUGMESSAGE "putback"
;        push hl
;        pop iy
;        ld (iy+3),c
;        DEBUGASSERT
;        ret

lof:
fpos:
        DEBUGMESSAGE "no support!"

illegalFunctionCall:
        ld e,5
        jp basicError

;FCB for DISK BASIC
; +0 FL.MOD     file mode
; +1 FL.FCA     Pointer to FCB for BDOS (low)
; +2 FL.LCA     Pointer to FCB for BDOS (high)
; +3 FL.LSA     Back up character
; +4 FL.DSK     device number
; +5 FL.SLB
; +6 FL.BPS     Position in FL.BUF
; +7 FL.FLG     Holds various information
; +8 FL.OPS     Pseudo head position
; +9 FL.BUF     256-byte file buffer

; device codes
;          SS0           SS1           SS2           SS3
;    ---------------------------------------------------------
;    | 00 04 08 0C | 10 14 18 1C | 20 24 28 2C | 30 34 38 3C | PS0
;    ---------------------------------------------------------
;    | 40 44 48 4C | 50 54 58 5C | 60 64 68 6C | 70 74 78 7C | PS1
;    ---------------------------------------------------------
;    | 80 84 88 8C | 90 94 98 9C | A0 A4 A8 AC | B0 B4 B8 BC | PS2
;    ---------------------------------------------------------
;    | C0 C4 C8 CC | D0 D4 D8 DC | E0 E4 E8 EC | F0 F4 F8 FC | PS3
;    ---------------------------------------------------------

; fileModes
; FOR INPUT (01H)
; FOR OUTPUT (02H)
; FOR APPEND (08H)
; random mode (04H)

; maximum number of files open: MAXFILES=15

; File Control Block
;       0     1     2     3     4     5     6     7     8
;    -------------------------------------------------------
;    | Mod | 00H | 00H | 00H | DEV | 00H | POS | 00H | PPS |
;    -------------------------------------------------------
;                  err? bckup       posHi
; followed by a 256 byte buffer
