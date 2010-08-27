
        
define  DEBUG
        
        output "msxdostools\nowmap.com"

        include "..\debug.asm"
        include "..\labels.asm"        
        include "..\asmlibs\bdos\bdos.inc"
                        
        org $0100
        
        DEBUGMESSAGE "EXnowmap"
       
        ld a,($f343)                    ; RAM page 2 (niet zo netjes denk ik)
        push af
        DEBUGDUMPREGISTERS
        
        ld a,(CMD_LENGTH)
        ld c,a
        ld hl,CMD_LINE
        
        xor a                           ; first Nowind interface
        ld b,C_NOWMAP
        ld de,$4e04                     ; send command
        call EXTBIO

        xor a                           ; get slotid first Nowind interface
        ld de,$4e01
        call EXTBIO

        ld h,$80                        ; enable Nowind in page 2
        call ENASLT

        call getHeader
        jr c,exit
        
        ld hl,$8000
        ld c,a
        ld b,(hl)
        ld de,buffer
        DEBUGDUMPREGISTERS
        ldir
        
exit:   pop af
        ld h,$80
        call ENASLT

        ld de,buffer
        DEBUGDUMPREGISTERS
        call String.PrintAsciiz      
        ret
        
getHeader:
        DEBUGMESSAGE "gH2"
        ld h,HIGH usbReadPage2
.init:
        ld b,HIGH 65535                 ; 42000 * 60 states ~ 0,7 sec (time out)
.loop:  ld a,(hl)
.chkaf: cp $af
        jr z,.chk05
        dec bc
        ld a,b
        or c
        jr nz,.loop
        DEBUGMESSAGE "Timeout!"
        ld a,2                          ; not ready
        scf
        ret

.chk05: ld a,(hl)
        cp $05
        jr nz,.chkaf
        ld a,(hl)
        ret

buffer: db "No connection with host!",0

        include "..\asmlibs\string\string.asm"
