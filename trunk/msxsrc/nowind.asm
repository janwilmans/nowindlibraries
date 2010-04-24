        define  DEBUG
        
        ;define NOWINDVERSION_FIRSTBATCH   ; our handmade first batch
        define NOWINDVERSION_SUNRISE    ; sunrise first batch
        
        ifdef NOWINDVERSION_FIRSTBATCH
        define FLASHROMSIZE 512
        endif
        
        ifdef NOWINDVERSION_SUNRISE
        define FLASHROMSIZE 4096
        endif
        
        output "nowind.rom"
        include "labels.asm"
        
        defpage 0, $4000, $4000      ; MSXDOS2 bank 0
        defpage 1, $4000, 3 * $4000     ; MSXDOS2 bank 1..3
        defpage 2, $4000, $4000      ; MSXDOS1
        
        ; insert MSXDOS2

        page 0
        module MSXDOS2_PART
        
        define  MSXDOSVER 2
        ; define ROMINIT $47d6
        define PRINTTEXT $728e
        
        incbin "..\roms\MSXDOS22.ROM", 0, $72f0-$4000
        
        PATCH $4006, device
        
        code ! $4010
        jp DSKIO
        jp DSKCHG
        jp GETDPB
        jp CHOICE
        jp DSKFMT
        db 0,0,0                        ; no DRVOFF
        
        PATCH $47d7, getBootArgs        ; INIHDR
        ;        PATCH $47dd, 0                  ; do not check for MSX1
        PATCH $488d, MYSIZE
        PATCH $489f, SECLEN
        PATCH $48b9, DRIVES
        PATCH $48eb, DEFDPB - 1
        PATCH $48f7, INIENV
        PATCH $5797, OEMSTA
        
        PATCH $4093, mapper
        
        code ! $4881
        db LOW initDiskBasic
        code ! $4884
        db HIGH initDiskBasic
        
        code ! $49a3
        ld hl,newAUX                    ; redirect AUX to host  
        ld de,$f327
        ld bc,10
        ldir
        nop
        nop
        nop
        nop
        nop                             ; nops needed to override existing code!
        
        code @ $72f0
        
getBootArgs:
; tijdelijke test
        
        call flashWriter                ; TODO: zoek betere plek (tijdelijk geen ruimte in dos1 rom)
        DEBUGMESSAGE "Any commands?"   
        call enableNowindPage0
        ld c,0                      ; c=0 means reset startup queue index 
.loop:  ld b,0                      ; b=0 means request startup command
        SEND_COMMAND C_CMDREQUEST
        GET_RESPONSE
        jr c,noNextCommand
        
        ld d,(hl)
        ld d,(hl)
        ld d,(hl)
        
        ; todo: read command here
        and a
        jr z,noNextCommand
        DEBUGMESSAGE "Got cmd!"   
 
        ld a,(hl)
        ;cp 1
        ;jr z, Com
                
        ld c,1
        jr .loop

noNextCommand:
        call restorePage0
        DEBUGMESSAGE "End of startup cmds"   
        
        call sendRegisters
        ld (hl),C_GETDOSVERSION
        call enableNowindPage0
        ld h,HIGH usbrd
        call getHeader
        
        call restorePage0
        jp c,bootMSXDOS1                ; no reply (host not connected?)
        
        and a
        jp nz,INIHRD                    ; boot MSXDOS2
        
bootMSXDOS1:
        ld hl,$576f                     ; boot MSXDOS1
        push hl
        ld a,4
        jp switchBank
            
        include "common.asm"
        include "extendedBios.asm"
        include "slotRoutines.asm"
        include "nowindDriver.asm"
        include "romdisk.asm"
        include "flashWriter.asm"  ; todo: remove, and use bootcommand to flash
        include "device.asm"
        
        ds $8000-(endCopyFromBank-copyFromBank)-$, $ff
        
        ; bank switching and data transfer
copyFromBank:
        ld (mapper),a
        ldir
enableBank0:
        xor a
switchBank:
        push af
        ld (mapper),a
        pop af
        ret
endCopyFromBank:
        
        page 1
        incbin "..\roms\MSXDOS22.ROM", $4000, 3 * $4000
        PATCH $4093, mapper
        PATCH $8093, mapper
        PATCH $C093, mapper
        
        ; areas not used in MSXDOS22.ROM
        ; bank 1: 0x5CA0 - 0x7FFF (9056 bytes)
        ; bank 2: 0x7F30 - 0x7FFF (208 bytes)
        ; bank 3: 0x7E70 - 0x7FFF (400 bytes)
        
; insert MSXDOS1
        page 2
        module MSXDOS1_PART
        
        define  MSXDOSVER 1
        ; define ROMINIT $576f
        define PRINTTEXT $5f86
        
        incbin "..\roms\DISK.ROM", 0, $7405-$4000
        
        PATCH $4006, device
        
        code ! $4010
        jp DSKIO
        jp DSKCHG
        jp GETDPB
        jp CHOICE
        jp DSKFMT
        db 0,0,0                        ; no DRVOFF
        
        PATCH $5770, INIHRD
        PATCH $57aa, $f380 + MYSIZE
        PATCH $581e, MYSIZE
        PATCH $582f, SECLEN
        PATCH $5851, DRIVES
        PATCH $5884, DEFDPB - 1
        PATCH $5890, INIENV
        PATCH $5ae8, DEFDPB             ; different address in some roms
        PATCH $65af, OEMSTA
        PATCH $5809, initDiskBasic      ; HRUNC
        ;PATCH $5b9a, getHostDate        ; get date from host when no clockchip found (different 5b95)
        
        code ! $595d
        ld hl,newAUX                    ; redirect AUX to host  
        ld de,$f327
        ld bc,10
        ldir
        nop
        nop
        nop
        nop                             ; do not remove!
        
        code @ $7405
        
        include "common.asm"
        include "extendedBios.asm"
        include "slotRoutines.asm"
        include "nowindDriver.asm"
        include "romdisk.asm"
        include "device.asm"
        
        ifdef BDOS_NOWIND
        include "nowindbdos.asm"
        endif
        
        ds $8000-(endCopyFromBank-copyFromBank)-$, $ff
        
        ; bank switching and data transfer
copyFromBank:
        ld (mapper),a
        ldir
enableBank0:
        xor a
        push af
        ld (mapper),a
        pop af
        ret
endCopyFromBank:

        
        
        defpage 3, 0, (512-80)*1024
        page 3
        ds (512-80)*1024, $ba
