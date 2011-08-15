MYSIZE          equ 8
SECLEN          equ 512
MAPPER23        equ $6000

        define DEBUG

        include "labels.asm"
        include "debug.asm"
              
        defpage 0, $4000, $4000
        defpage 1, $4000, $4000
        defpage 2, $4000, $4000
        defpage 3, $4000, $4000

        
        incbin "..\roms\MSXDOS23.ROM", 0, $4000

        org $4000

        code ! $4010
        jp DSKIO
        jp DSKCHG
        jp GETDPB
        jp CHOICE
        jp DSKFMT
        db 0,0,0                        ; no DRVOFF

        PATCH $4828, noCHGCPU_bank0
        PATCH $5196, noCHGCPU_bank0
        PATCH $51b2, noCHGCPU_bank0        

        PATCH $47d7, INIHRD
        PATCH $489b, MYSIZE     ; *
        PATCH $48ad, SECLEN     ; *
        PATCH $48c7, DRIVES     ; *
        PATCH $48f9, DEFDPB - 1 ; *
        PATCH $4905, INIENV     ; *

        PATCH $5797, OEMSTA

        PATCH $7fd1, MAPPER23

; initDiskBasic equ $4969

        code ! $7c52

INIHRD: DEBUGMESSAGE "INIHRD23"
        ;DEBUGDISASM
        ret
DRIVES: DEBUGMESSAGE "DRIVES23"
        ld l,1
        ;DEBUGDISASM
        ret
INIENV: DEBUGMESSAGE "INIENV23"
        ;DEBUGDISASM
        ret
        
DSKIO:  DEBUGMESSAGE "DSKIO23"
        ld a,2
        scf
        ret
DSKCHG: DEBUGMESSAGE "DSKCHG23"
        ld a,2
        scf
        ret
GETDPB: DEBUGMESSAGE "GETDPB23"
        ret
CHOICE: DEBUGMESSAGE "CHOICE23"
        ret
DSKFMT: DEBUGMESSAGE "DSKFMT23"
        ret
OEMSTA: DEBUGMESSAGE "OEMSTA23"
        scf
        ret

noCHGCPU_bank0:
        DEBUGMESSAGE "CHGCPU caught! (bnk0)"
        DEBUGDUMPREGISTERS
        ret

        MAKEDPB $f8, 2, 112, 1 * 80 * 9, 2, 2      ; 360 kB (1 side * 80 tracks * 9 tracks/sector)
DEFDPB: MAKEDPB $f9, 2, 112, 2 * 80 * 9, 3, 2      ; 720 kB

       
        
        page 1        

        org $4000
        incbin "..\roms\MSXDOS23.ROM", $4000, $4000

        code ! $4497
        DEBUGMESSAGE "blocked CPU switch"
        ex af,af
        call CALSLT
        jr $44bf
        ds $44bf - $, 0

        PATCH $7fd1, MAPPER23



        page 2

        org $4000
        incbin "..\roms\MSXDOS23.ROM", $8000, $4000

        PATCH $7fd1, MAPPER23



        module MSXDOS1_MODULE
        page 3

        org $4000
        incbin "..\roms\MSXDOS23.ROM", $c000, $4000

        code ! $4010
        jp DSKIO
        jp DSKCHG
        jp GETDPB
        jp CHOICE
        jp DSKFMT
        db 0,0,0                        ; no DRVOFF

;        PATCH $5770, INIHRD
;        PATCH $57aa, $f380 + MYSIZE
;        PATCH $581e, MYSIZE
;        PATCH $582f, SECLEN
;        PATCH $5851, DRIVES
;        PATCH $5884, DEFDPB - 1
;        PATCH $5890, INIENV
;        PATCH $5ae8, DEFDPB             ; different address in some roms
;        PATCH $65af, OEMSTA

        PATCH $7fd1, MAPPER23

        code ! $7e00

INIHRD: DEBUGMESSAGE "INIHRD23dos1"
        ;DEBUGDISASM
        ret
DRIVES: DEBUGMESSAGE "DRIVES23dos1"
        ld l,1
        ;DEBUGDISASM
        ret
INIENV: DEBUGMESSAGE "INIENV23dos1"
        ;DEBUGDISASM
        ret
        
DSKIO:  DEBUGMESSAGE "DSKIO23dos1"
        DEBUGDISASM
        ld a,2
        scf
        ret
DSKCHG: DEBUGMESSAGE "DSKCHG23dos1"
        ld a,2
        scf
        ret
GETDPB: DEBUGMESSAGE "GETDPB23dos1"
        ret
CHOICE: DEBUGMESSAGE "CHOICE23dos1"
        ret
DSKFMT: DEBUGMESSAGE "DSKFMT23dos1"
        ret
OEMSTA: DEBUGMESSAGE "OEMSTA23dos1"
        scf
        ret
