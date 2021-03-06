        
        ; enabled bdos patches, this is an experimental new mode
        ; which allows direct access to the host harddisk through patch BDOS calls 
        ; this is potentially much faster and eliminates the need for disk-images.
        ;define BDOS_NOWIND
        
        ; enables messages that can only be viewed in emulation, 
        ; are harmless on real hardware but cause small delays
        ; recommened: turn off for releases
        ;define DEBUG      
        
        ; enables messages for debugging can be logged even on real hardware 
        ; by sending them to the host over USB, causes somewhat larger delays
        ; but can be used to diagnose problems that are otherwise hard to debug        
        ;define USBDEBUG
        
        ;define NOWINDVERSION_FIRSTBATCH   ; our handmade first batch
        define NOWINDVERSION_FIRSTBATCH    ; sunrise first batch

        ifdef NOWINDVERSION_FIRSTBATCH
        define FLASHROMSIZE 512
        endif

        ifdef NOWINDVERSION_SUNRISE
        define FLASHROMSIZE 4096
        endif

        output "nowind.rom"
        include "labels.asm"
        include "debug.asm"

        ; bank 0..3     MSXDOS2
        ; bank 4        MSXDOS1 
        ; bank 5        Nowind functions
        ; bank 6..7     empty
        ; bank 8..31    reserved
       
pageNumber := 0
        repeat 32
        defpage pageNumber, $4000, $4000
pageNumber := pageNumber + 1        
        endrepeat
         
        ; insert MSXDOS2

        page 0
        module MSXDOS2_MODULE

        define MSXDOSVER 2
        define PRINTTEXT $728e
        define ORIGINAL_HOOK_RUNC $495b

        incbin "..\roms\MSXDOS22.ROM", 0, $72f0-$4000

        PATCH $4002,bankInit

        code ! $4010
        jp DSKIO
        jp DSKCHG
        jp GETDPB
        jp CHOICE
        jp DSKFMT
        db 0,0,0                        ; no DRVOFF

        PATCH $47d7, INIHRD
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

        include "common.asm"
        include "extendedBios.asm"
        include "slotRoutines.asm"
        include "nowindDriver.asm"
        include "romdisk.asm"
        include "dos_aux.asm"
        include "device.asm"

        BANKSWITCHING 0

        page 1
        incbin "..\roms\MSXDOS22.ROM", $4000, $4000
        PATCH $4002, bankInit 
        PATCH $4093, mapper        
        BANKSWITCHING 1

        page 2
        incbin "..\roms\MSXDOS22.ROM", $8000, $4000
        PATCH $4002, bankInit 
        PATCH $4093, mapper        
        BANKSWITCHING 2
        
        page 3
        incbin "..\roms\MSXDOS22.ROM", $c000, $4000
        PATCH $4002, bankInit 
        PATCH $4093, mapper        
        BANKSWITCHING 3

        ; areas not used in MSXDOS22.ROM
        ; bank 1: 0x5CA0 - 0x7FFF (9056 bytes)
        ; bank 2: 0x7F30 - 0x7FFF (208 bytes)
        ; bank 3: 0x7E70 - 0x7FFF (400 bytes)

        endmodule MSXDOS2_MODULE

; insert MSXDOS1
        page 4                          ; overwrite page2 with our patched DOS1 diskrom
        module MSXDOS1_MODULE

        define MSXDOSVER 1
        define ORIGINAL_HOOK_RUNC $5897
        
        incbin "..\roms\DISK.ROM", 0, $7405-$4000

        PATCH $4002,bankInit

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
        ;PATCH $5b9a, getHostDate        ; get date from host when no clockchip found (different 5b95)              ; does not appear to work?
        ;PATCH $599c, initClockchip      ; intercept 'check for and initialize clockchip' normally 0x40B8 is called ; does not appear to work?
        ;PATCH $5b9a, initClockchip      ; intercept 'check for and initialize clockchip' normally 0x40B8 is called ; does not appear to work?

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
    
        BANKSWITCHING 4
        endmodule MSXDOS1_MODULE
        
        page 5
        module NOWIND_MODULE
        MSXROMHEADER
        include "init.asm"
        include "flashWriter.asm"
        include "slotRoutines.asm"
        include "common.asm"
        BANKSWITCHING 5
        endmodule NOWIND_MODULE        
        
        page 6
        code @ $4000
        MSXROMHEADER
        BANKSWITCHING 6

        page 7
        code @ $4000
        MSXROMHEADER
        BANKSWITCHING 7
        
        module REMAINING_ROM_MODULE
                       
        INCLUDE_ROMDISK_360KB "..\disks\dos2.dsk"
    
        endmodule REMAINING_ROM_MODULE