
        output "msxdostools\nowmap.com"

        include "..\debug.asm"
        include "..\labels.asm"        
        include "..\asmlibs\bdos\bdos.inc"
                        
        org $0100
        
        DEBUGMESSAGE "EXnowmap"
        DEBUGDUMPREGISTERS

        ld a,(CMD_LENGTH)
        ld c,a
        ld hl,CMD_LINE
        
        xor a                           ; first Nowind interface
        ld b,C_NOWMAP
        ld de,$4e04                     ; send command
        call EXTBIO

        ; host will return message
        ; print message here
        ret
        

;        include "..\asmlibs\string\string.asm"
