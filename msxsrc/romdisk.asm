; romdisk.asm contains the methods to support the romdisk that can reside
; in banks 6 and up of the Nowind Flashrom
; the romdisk can be used as a smallish read-only harddisk even when
; no host is connected

        define ROMDSKBANK 8                    ; romdisk starts in bank 8
        define ROMDSKLAST (FLASHROMSIZE/16)-1


ROMDISK_DSKIO:
        DEBUGMESSAGE "ROM_DSKIO"
        pop af
        ld a,0
        ret c                           ; write protected
        
        ld ix,copyFromBank
        ex de,hl
.loop:  push bc
        push hl        
        
        bit 7,d
        jr nz,.directCopy
        ld a,d
        cp $3e        
        jr c,.directCopy

        call .findSector
        push de
        ld de,($f34d)        
        call callInBank
        pop de
        ld bc,512
        ld hl,($f34d)
        call XFER
        jr .nextSector           
              
.directCopy:                
        call .findSector  
        call callInBank
.nextSector:
        pop hl
        inc hl
        pop bc
        djnz .loop
        ret

.findSector:        
        ld a,l                          ; determine bank
        and %11100000
        or h
        rlca
        rlca
        rlca
        ld b,a

        ld a,l
        and 31
        ld c,a
        ld a,ROMDSKLAST                 ; banks with b0..5=0 reside in the last bank
        jr z,.skip

        ld a,b
        add ROMDSKBANK
        ld b,c
        dec b
.skip:  ld c,a
        ld a,b
        rlca
        add $41                         ; disk images starts at $4100
        ld h,a
        ld l,0
        ld a,c
        ld bc,512
        ret
                                         
ROMDISK_DSKCHG:
        DEBUGMESSAGE "ROM_DSKCHG"
        pop af
        and a
        ld b,1                          ; not changed
        ret

ROMDISK_GETDPB:
        ; not implemented (standard mediadescriptor as used)
        
ROMDISK_DSKFMT:
        ; not implemented (no disk can be formatted)
        
