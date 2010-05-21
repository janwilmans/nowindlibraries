MYSIZE          equ 8                   ; size of work area (not used, some programs write here to stop drives spinning)
SECLEN          equ 512                 ; sector size

; SLTWRK entry
; +0    rom drive number
; +1..5 previous EXTBIO
; +6    not used
; +7    not used


; if this is defined we tell DOS2.3 that we _are DOS2.3 also, so
; it does not try to override our initilazations

define  PRETEND_2B_DOS23

INIHRD:
        DEBUGMESSAGE "INIHRD"

;        call getWorkArea
;        DEBUGDUMPREGISTERS

        call enableNowindPage0          ; clear hostToMSXFifo by reading 4Kb of random data
        ld bc,4096
.loop:  ld a,(usbReadPage0)
        dec bc
        ld a,b
        or c
        jr nz,.loop
        call restorePage0

        ld h,HIGH usbWritePage1
        ld (hl),$af                     ; INIHRD command
        ld (hl),$ff
        jp nowindInit

DRIVES:
        DEBUGMESSAGE "DRIVES"
        push af                         ; A, BC and DE should be preserved!
        push bc
        push de
        ld a,(DEVICE)

        push hl
        call sendRegisters
        ld (hl),C_DRIVES
        ld hl,drivesCommand
        call executeCommandNowindInPage0

        pop hl
        ld l,2
        jr c,.error
        ld l,a
.error:
        pop de
        pop bc
        pop af
        ret

drivesCommand:
        ld h,HIGH usbReadPage0
        ld (LASTDRV),a                  ; install phantom drives (CRTL key)?
        ld a,(hl)                       ; allow other drives (SHIFT key)?
        ld (DEVICE),a
        ld a,(hl)                       ; number of drives
        ret

INIENV:
; Interrupt handler can be installed here and
; work area can be initialized when it was requested
        DEBUGMESSAGE "INIENV"

        ifdef PRETEND_2B_DOS23
        DEBUGMESSAGE "Lie about being DOS v2.31"
        ld a,$23
        ld ($f313),a
        endif

        call installExtendedBios

        call sendRegisters
        ld (hl),C_DRIVES
        ld hl,inienvCommand
        call executeCommandNowindInPage0

        push af
        call getEntrySLTWRK
        pop af
        ld (hl),0                       ; default drive number for romdisk
        ret c
        DEBUGDUMPREGISTERS
        ld (hl),a                       ; drive number for romdisk
        ret

inienvCommand:
        ret


checkWorkArea:
;        ld a,1
        cp 1
        ret

        push bc
        push hl
        push af
;        call GETWRK
        call getEntrySLTWRK
        pop af
        cp (hl)
        pop hl
        pop bc
        ret

DSKIO:
; Input     F   Carry for set for write, reset for read
;           A   Drive number
;           B   Number of sectors to read/write
;           C   Media descriptor
;           DE  Logical sector number
;           HL  Transfer address
; Output    F   Carry set when not successful
;           A   Error code
;           B   Number of remaining sectors (TODO: check! even without error?)

        DEBUGMESSAGE "DSKIO"
        push af
        call checkWorkArea
        jp z,ROMDISK_DSKIO
        pop af

        call sendRegisters
        ld (hl),C_DSKIO
        jr c,dskioWrite                 ; read or write?

dskioRead:
        call blockRead
        DEBUGMESSAGE "exit_dskio"
        DEBUGDUMPREGISTERS
        ret c                           ; return error (error code in a)
        xor a                           ; no error, clear a
        ret
        
dskioWrite:
        DEBUGMESSAGE "dskwrite"
        rlca
        jr c,.page23

        DEBUGMESSAGE "p01"
        ld hl,blockWrite01
        call executeCommandNowindInPage2
        ret c                           ; return error (error code in a)
        ret pe                          ; host returns 0xfe when data for page 2/3 is available
        DEBUGMESSAGE "doorgaan!"

.page23:
        DEBUGMESSAGE "p23"
        ld hl,blockWrite23
        call executeCommandNowindInPage0
        DEBUGDUMPREGISTERS
        DEBUGMESSAGE "back"
        ret c                           ; return error (error code in a)
        xor a                           ; some software (wb?) requires that a is zero, because they do not check the carry
        ret

        PHASE $ + $4000

blockWrite01:
        DEBUGDUMPREGISTERS
        DEBUGMESSAGE "blkWr01"
        ld h,HIGH usbReadPage2
        jr .start2
.start:
        call getHeaderInPage2
.start2:
        ret c                           ; exit (not ready)
        or a
        ret m                           ; exit (no error)
        jr nz,.error

        DEBUGMESSAGE "wr01"
        ld e,(hl)                       ; address
        ld d,(hl)
        ld c,(hl)                       ; number of bytes
        ld b,(hl)
        ld a,(hl)                       ; block sequence number

        ex de,hl
        ld de,usbWritePage2
        ld (de),a                       ; mark block begin
        ldir
        ld (de),a                       ; mark block end
        jr .start

.error: scf
        ld a,(hl)                       ; get error code
        ret

        DEPHASE

blockWrite23:
        DEBUGDUMPREGISTERS
        DEBUGMESSAGE "blkWr23"
        ld h,HIGH usbReadPage0
        jr .start2
.start:
        call getHeaderInPage0
.start2:
        ret c                           ; exit (not ready)
        or a
        ret m                           ; exit (no error)
        jr nz,.error

        DEBUGMESSAGE "wr23"
        ld e,(hl)                       ; address
        ld d,(hl)
        ld c,(hl)                       ; number of bytes
        ld b,(hl)
        ld a,(hl)                       ; block sequence number

        ;DEBUGDUMPREGISTERS
        ex de,hl
        ld d,HIGH usbWritePage1
        ld (de),a                       ; mark block begin
        ldir
        ld (de),a                       ; mark block end
        jr .start

.error: scf
        ld a,(hl)                       ; get error code
        ret


DSKCHG:
; Input     A   Drive number
;           B   0
;           C   Media descriptor (previous)
;           HL  Base address of DPB
; Output    B   1   Disk unchanged
;               0   Unknown (DPB is updated)
;               -1  Disk changed (DPB is updated)
;           F   Carry set when not succesfull
;           A   Error code

        DEBUGMESSAGE "DSKCHG"
        push af
        call checkWorkArea
        jp z,ROMDISK_DSKCHG
        pop af

        push hl
        call sendRegisters
        ld (hl),C_DSKCHG
        ld hl,dskchgCommand
        call executeCommandNowindInPage0
        pop hl
        ret c                   ; not ready (reg_a = 2)

;        push hl
        ;call sendRegisters
        ;ld (hl),C_DSKCHG
        ;call enableNowindPage0
        ;ld h,HIGH usbReadPage0
        ;call getHeader

;    SEND_CMD_AND_WAIT C_DSKCHG

;        ld c,(hl)               ; media descriptor (when disk was changed)
;        push af
;        push bc
;        call restorePage0
;        pop bc
;        pop af
;        pop hl
;        ret c           ; not ready

        or a
        ld b,1
        ret z           ; not changed
        ld b,c
        call GETDPB     ; TODO: restore reg_a for drive number!
        ld a,10
        ret c
        ld b,255
        ret

dskchgCommand:
        ld h,HIGH usbReadPage0
        ld c,(hl)
        ret

GETDPB:
; Input     A   Drive number
;           B   Media descriptor (first byte of FAT)
;           C   Previous media descriptor (does not seem to be used in other drivers)
;           HL  Base address of HL
; Output    DPB for specified drive in [HL+1]..[HL+18]

        DEBUGMESSAGE "GETDPB"
        DEBUGDUMPREGISTERS
        ex de,hl
        inc de
        ld h,a
        ld a,b
        cp $f0
        ld a,h
        DEBUGDUMPREGISTERS
        ;jr z,.hddImage

;        MESSAGE "ROM GETDPB"

        ld a,b
        sub $f8
        ret c                           ; not supported in msxdos1
        rlca                            ; 2x
        ld c,a
        rlca                            ; 4x
        rlca                            ; 8x
        rlca                            ; 16x
        add a,c                         ; 18x
        ld c,a
        ld b,0
        ld hl,supportedMedia
        add hl,bc
        ld c,18
        ldir
        ret

.hddImage:
        DEBUGMESSAGE ".hddImage"
        ;MESSAGE "HOST GETDPB"

        call sendRegisters
        ld (hl),C_GETDPB
        call enableNowindPage0
        call getHeaderInPage0

        jr c,.exit                      ; not ready
        ld e,a                          ; destination
        ld d,(hl)
        ld bc,18
        DEBUGDUMPREGISTERS
        ldir
        ;DB $ed, $0a
.exit:  jp restorePage0

CHOICE:
        ;DEBUGMESSAGE "CHOICE"
        if MSXDOSVER = 2
        ld hl,.none
        ret
.none:  db 0
        else
        ld hl,0                         ; no choice
        ret
        endif

DSKFMT:
        scf
        ld a,16                         ; other error
        ret

OEMSTA:
        push hl
        ld hl,.statement
        call findStatementName
        ld e,(hl)
        inc hl
        ld d,(hl)
        pop hl
        ret c
        push de
        ret

.statement:
        db "IMAGE",0
        dw changeImage
;        db "VSTREAM",0
;        dw videoStream
        db 0

; send arguments, command, filename, end with ":"
changeImage:
        DEBUGMESSAGE "changeImage"
        push hl
        call sendRegisters
        ld (hl),C_CHANGEIMAGE
        pop hl

call_exit:
        DEBUGMESSAGE "call_exit"
.loop:  ld a,(hl)
        ld (usbWritePage1),a
        cp ":"
        ret z
        or a
        ret z
        inc hl
        jr .loop

;videoStream:
;        push hl
;        include "vram.asm"
;        pop hl
;        jp call_exit

; hl points to text
printVdpText2:
        push af
.loop:  ld a,(hl)
        out ($98),a
        inc hl
        or a
        jr nz,.loop
        pop af
        ret


supportedMedia:

.f8:    MAKEDPB $f8, 512, 2, 112, 1 * 80 * 9, 2, 2      ; 360 kB (1 side * 80 tracks * 9 tracks/sector)
.def:   MAKEDPB $f9, 512, 2, 112, 2 * 80 * 9, 3, 2      ; 720 kB
        MAKEDPB $fa, 512, 2, 112, 1 * 80 * 8, 1, 2      ; 320 kB
        MAKEDPB $fb, 512, 2, 112, 2 * 80 * 8, 2, 2      ; 640 kB
        MAKEDPB $fc, 512, 1, 64,  1 * 40 * 9, 2, 2      ; 180 kB
        MAKEDPB $fd, 512, 2, 112, 2 * 40 * 9, 2, 2      ; 360 kB
        MAKEDPB $fe, 512, 1, 64,  1 * 40 * 8, 1, 2      ; 160 kB
        MAKEDPB $ff, 512, 2, 112, 2 * 40 * 8, 1, 1      ; 320 kB

; WARNING: in some cases DEFDPB-1 is expected!
DEFDPB  equ supportedMedia.def
