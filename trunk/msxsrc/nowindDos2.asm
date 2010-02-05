romInit:        equ $47d6
printText:      equ $728e

        define  MSXDOS2
        define  DEBUG
        define  ROMDSKBANK 4            ; first bank of ROMdisk image
        define  ROMDSKLAST 4+24-1
        
        output  "nowindDos2.rom"
        include "labels.asm"
        
        defpage 0, $4000, $4000
        defpage 1, $4000, 3 * $4000
        defpage 2, $4000, 28 * $4000
        
        page 0

        incbin "..\roms\MSXDOS22.ROM", 0, $72f0-$4000
                
        PATCH $4006, device

        code ! $4010
        jp DSKIO
        jp DSKCHG
        jp GETDPB
        jp CHOICE
        jp DSKFMT
        db 0,0,0                        ; no DRVOFF
        
        PATCH $47d7, INIHRD
;        PATCH $47dd, 0                  ; do not check for MSX1
        PATCH $488d, MYSIZE
        PATCH $489f, SECLEN
        PATCH $48b9, DRIVES
        PATCH $48eb, DEFDPB - 1
        PATCH $48f7, INIENV
        PATCH $5797, OEMSTA

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
        nop                             ; do not remove!
        
        code @ $72f0
        
        include "common.asm"
        include "extendedBios.asm"
        include "slotRoutines.asm"
        include "nowindDriver.asm"
        include "romdisk.asm"
        include "flashWriter.asm"
        include "device.asm"

        ds $8000-$-12, $ff

; bank switching and data transfer
copyFromBank:
        ld (mapper),a                   ; no not remove!
        ldir                            ; no not remove!
enableBank0:
        xor a                           ; no not remove!
        push af                         ; no not remove!
        ld (mapper),a                   ; no not remove!
        pop af                          ; no not remove!
        ret                             ; no not remove!

        page 1

        incbin "..\roms\MSXDOS22.ROM", $4000, 3 * $4000

        page 2  
        incbin "..\roms\romdisk.bin"        
        romheader 28
