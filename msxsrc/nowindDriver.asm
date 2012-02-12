; TODO: holding 'INS' should disable the interface completely (this is also what Sunrise IDE interface does)
; this is used to boot not using nowind, without having to remove the cartridge. (also usefull if flashing fails?)
;
; holding 'TAB' should prevent boot from nowind, even if a disk is present. This is usefull for copying disks when 
; the 'source' is nowind, and the disk normally boots from sector (so cannot be stopped).

MYSIZE          equ 8                   ; size of work area (not used, some programs write here to stop drives spinning)
SECLEN          equ 512                 ; sector size

; SLTWRK entry
; +0    rom drive number
; +1..5 previous EXTBIO
; +6    not used
; +7    not used

INIHRD:
        DEBUGMESSAGE "INIHRD"
        ld h,HIGH usbWritePage1
        ld (hl),$af                     ; signal MSX reset to host
        ld (hl),$ff
        ret


; function: DRIVES, returns the number of connected drives (for this diskrom)
;
; in:  none (Z flag set if CTRL is pressed??)
; out: l = number of drives
;
; remark: registers A, BC and DE should be preserved!
DRIVES:
        DEBUGMESSAGE "DRIVES"
        push af
        push bc
        push de
        ld a,(DEVICE)
        
        push hl
        call sendRegisters
        ld (hl),C_DRIVES
        ld hl,drivesCommand
        call executeCommandNowindInPage0
        
        pop hl
        ld l,1                          ; install one drive when not connected to host
        jr c,.error
        ld l,a                          ; number of drives
.error:
        pop de
        pop bc
        pop af
        ret

drivesCommand:
        call getHeaderInPage0
        ret c
        ld (LASTDRV),a                  ; install phantom drives (CRTL key)?
        ld a,(hl)                       ; allow other drives (SHIFT key)?
        ld (DEVICE),a
        ld ($f347),a                    ; TODO: move to SLTWRK entry
        ld a,(hl)                       ; number of drives
        ret

INIENV:
; Interrupt handler can be installed here and
; work area can be initialized when it was requested
        DEBUGMESSAGE "INIENV"
      
        call installExtendedBios        ; TODO: move to init.asm (als set HKLVLK self)

        call sendRegisters
        ld (hl),C_INIENV
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
        call getHeaderInPage0
        ret


; function: DSKIO ($4010), transfers logical sectors from memory to disk (write) or from disk to memory (read)
;
; in:  f = carry for set for write, reset for read
;      a = drive number
;      b = number of sectors to read/write
;      c = media descriptor
;     de = logical sector number
;     hl = transfer address
;     
; out: f = carry set when not successful
;      a = 0, write protected
;      a = 2, not ready
;      a = 4, data (CRC) error
;	   a = 6, seek error
;	   a = 8, record not found
;      a = 10, write fault
;      a = 12, other error
;      b = number of remaining sectors (TODO: check! even without error?)
;
; remark: original DSKIO changes all registers including ix, iy but not the shadow registers.

DSKIO:
        ;DEBUGMESSAGE "DSKIO"
        ;DEBUGDUMPREGISTERS
        ;USB_DBMSG "DSKIO"

        push af
        call checkWorkArea
        jp z,ROMDISK_DSKIO
        pop af

        call sendRegisters
        ld (hl),C_DSKIO
        jp nc,blockRead

        call blockWrite         ; todo: value of A depends on blockWrite? change this.
        ret c
        xor a
        ret
        
; function: DSKCHG ($4013), check whether the disk has been changed
;
; in:  a = drive number (0=A:, etc.)
;      b = 0
;      c = media descriptor (previous)
;     hl = base address of DPB
;     
; out: a = error code, like DSKIO
;      b = -1, disk changed (DPB is updated)
;      b = 0, unknown (DPB is updated)
;      b = 1, disk unchanged
;      f = carry set when not succesful
;
; changed: c, de, hl, ix
        
DSKCHG:

        ;DEBUGMESSAGE "DSKCHG"
        ;DEBUGDUMPREGISTERS
        push af
        call checkWorkArea
        jp z,ROMDISK_DSKCHG
        pop af

        ;USB_DBMSG "DSKCHG"
        
        push af
        push hl
        call sendRegisters
        ld (hl),C_DSKCHG
        ld hl,dskchgCommand
        call executeCommandNowindInPage0
        pop hl
        pop de
        ret c                           ; not ready (reg_a = 2)

        or a
        ld b,1
        ret z                           ; not changed

        if MSXDOSVER == 1
        
        ld a,d                          ; update dpb in MSXDOS1 (TODO: other diskrom can still be master and DOS1 !!!!)
        ld b,c
        call GETDPB
        ld a,10
        ret c
        
        endif
        
        ld b,255
        ret

dskchgCommand:
        call getHeaderInPage0
        ret c                
        ld c,(hl)
        ret

; function: GETDPB ($4016), get DPB for specified media descriptor
;
; in:  a = drive number
;      b = media descriptor (first byte of FAT)
;      c = previous media descriptor (does not seem to be used in other drivers)
;     hl = base address of DPB
;     
; out: DPB for specified drive in [HL+1]..[HL+18]
;
; unchanged: iy

GETDPB:
        ;DEBUGMESSAGE "GETDPB"
        ex de,hl
        inc de
        ld h,a                          ; store drive number 
        ld a,b
        cp $f0
        jr z,.makeDPB
        sub $f8
        ret c                           ; not supported

        rlca                            ; multiply by 18
        ld c,a
        rlca
        rlca
        rlca
        add a,c
        
        ld c,a                          ; update DPB
        ld b,0
        ld hl,supportedMedia
        add hl,bc
        ld c,18
        ldir
        ret

.makeDPB:
        ;DEBUGMESSAGE ".makeDPB"
        ld a,($f313)                    ; don't support mediadescriptor F0 under MSXDOS1       
        cp 1                            ; large clustersize causes much memory to be allocated (causing crash)
        ret c

        ld a,h                          ; restore drive number
        call sendRegisters
        ld (hl),C_GETDPB
        ld hl,getdpbCommand
        jp executeCommandNowindInPage0

getdpbCommand:
        call getHeaderInPage0
        ret c
        ld e,a                          ; get destination from host
        ld d,(hl)
        ld bc,18
        ldir
        ret

; function: CHOICE ($4019)
;
; in:  none
; out: HL = Address of zero terminated character string with the text with choices for a DSKFMT
;      If there is no choice (only 1 format supported) return [HL]=0
; unchanged: ?

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

; function: DSKFMT, Format disk
;    format a disk and writes a MSX boot sector at sector 0, clears all FATs.
;    (media descriptor at first byte, 0FFh at second/third byte and rest zero)
;    and clears the directory (filling it with zeros)
;
; in:  a = choice specified by user (1-9). See CHOICE
;      d = drive number (0=A:)
;     hl = begin address of work area
;     bc = length of work area
; out: f = carry set when not succesful 
;      a = error code, if not succesful
;      a = 0, write protected
;      a = 2, not ready
;      a = 4, data (CRC) error
;      a = 6, seek error
;      a = 8, record not found
;      a = 10, write fault
;      a = 12, bad parameter
;      a = 14, insufficient memory
;      a = 16, other errors

DSKFMT:
        ld a,16                         ; other error
        scf
        ret
        
; function: OEMSTA, ?
;
; in:  ?
; out: ?
; unchanged: ?

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


checkWorkArea:
        push bc
        push hl
        push af
        call getEntrySLTWRK
        pop af
        cp (hl)
        pop hl
        pop bc
        ret


supportedMedia:

        MAKEDPB $f8, 2, 112, 1 * 80 * 9, 2, 2      ; 360 kB (1 side * 80 tracks * 9 tracks/sector)
DEFDPB: MAKEDPB $f9, 2, 112, 2 * 80 * 9, 3, 2      ; 720 kB
        MAKEDPB $fa, 2, 112, 1 * 80 * 8, 1, 2      ; 320 kB
        MAKEDPB $fb, 2, 112, 2 * 80 * 8, 2, 2      ; 640 kB
        MAKEDPB $fc, 1, 64,  1 * 40 * 9, 2, 2      ; 180 kB
        MAKEDPB $fd, 2, 112, 2 * 40 * 9, 2, 2      ; 360 kB
        MAKEDPB $fe, 1, 64,  1 * 40 * 8, 1, 2      ; 160 kB
        MAKEDPB $ff, 2, 112, 2 * 40 * 8, 1, 1      ; 320 kB

; use this DPB as default to reserve a larger fat buffer
;DEFDPB:
;        db $f0
;        dw 512
;        db $f,4,3,3,1,0,2,0,$21,0,$f8,$09,8,$11,0
