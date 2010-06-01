        define DEBUG
        
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

        defpage 0, $4000, $4000         ; MSXDOS2 bank 0
        defpage 1, $4000, 3 * $4000     ; MSXDOS2 bank 1..3
        defpage 2, $4000, $4000         ; MSXDOS1
        defpage 3, $4000, (512-80)*1024 ; empty
        
        ; insert MSXDOS2

        page 0
        module MSXDOS2_PART

        define MSXDOSVER 2
        define PRINTTEXT $728e

        incbin "..\roms\MSXDOS22.ROM", 0, $72f0-$4000

        code ! $4010
        jp DSKIO
        jp DSKCHG
        jp GETDPB
        jp CHOICE
        jp DSKFMT
        db 0,0,0                        ; no DRVOFF

        PATCH $47d7, INIHRD        ; INIHDR
        ;PATCH $47dd, 0                  ; do not check for MSX1
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

        code @ $72f0

        include "init.asm"

        include "common.asm"
        include "extendedBios.asm"
        include "slotRoutines.asm"
        include "nowindDriver.asm"
        include "romdisk.asm"
        include "flashWriter.asm"  ; todo: remove, and use bootcommand to flash
        include "dos_aux.asm"
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

        define MSXDOSVER 1

        incbin "..\roms\DISK.ROM", 0, $7405-$4000

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

        code @ $7405

        include "common.asm"
        include "extendedBios.asm"        
        include "slotRoutines.asm"
        include "nowindDriver.asm"
        include "romdisk.asm"
        include "device.asm"
        include "dos_aux.asm"
        
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

        module remainingRom

        page 3
        ;ds (512-80)*1024, $ff
        romheader 27, MSXDOS2_PART.nowindInit
