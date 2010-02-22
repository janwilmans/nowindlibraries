        define  DEBUG
        
        define  FLASHROMSIZE 512
        define  PRINTTEXT $5f86
        define  MSXDOSVER 1

romInit         equ $576f

        define  MSXDOS1 ; TODO!!!!!
        define  ROMDSKBANK 1            ; first bank of ROMdisk image
        define  ROMDSKLAST 1+24-1
        define  BDOS_NOWIND_DISABLED

        
        output "nowindDos1.rom"
        include "labels.asm"
                
        defpage 0, $4000, $4000
        defpage 1, $4000, 31 * $4000
        
        page 0

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

        ifdef BDOS_NOWIND
        
                                ; just patching the BDOS hook will not work; not everybody uses the hook
        ;PATCH $5d20, BDOSNW                                              ; overwrite the standard BDOS hook "DW $56D3" with BDOSNW
                
        ; even patching the BDOS jump table will not work; internal calls (even in command.com) bypass it
        ; jump table patches
        ;PATCH $572b, BDOS_0FH_J                                  ; overwrite specific function 0Fh in jump table
        ;PATCH $572f, BDOS_11H_J                                  ; overwrite specific function 11h in jump table
        ;PATCH $5731, BDOS_12H_J                                  ; overwrite specific function 12h in jump table
                
        ; these patches are at the start of the routine themselves, the addresses are more or less "standardized" 
        ; over several brands of diskroms       
        ; in-routine patches
        PATCH $4463, BDOS_0FH                                     ; overwrite function 0Fh itself!
        PATCH $4fb9, BDOS_11H                             ; overwrite function 11h itself!
        PATCH $5007, BDOS_12H                             ; overwrite function 12h itself!
                
        endif
                                
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
        include "flashWriter.asm"
        include "device.asm"
        
        ifdef BDOS_NOWIND
        include "nowindbdos.asm"
        endif
        
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
        
        incbin "..\roms\romdisk.bin"        
        romheader 31
