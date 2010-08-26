
        output "msxdostools\nowmap.com"

        include "..\debug.asm"
        include "..\labels.asm"        
        include "..\asmlibs\bdos\bdos.inc"
                        
        org $0100
        
        DEBUGMESSAGE "EXnowmap"
        DEBUGDUMPREGISTERS
        xor a                           ; first Nowind interface
        ld b,C_NOWMAP
        ld de,$4e03                     ; send command
        call EXTBIO

        ld a,(CMD_LENGTH)
        ld b,0
        ld c,a
        ld hl,CMD_LINE
        
        xor a                           ; first Nowind interface
        ld de,$4e04                     ; block write
        DEBUGDUMPREGISTERS
        call EXTBIO
        DEBUGMESSAGE "boe"
        DEBUGDUMPREGISTERS
        
        ; host will return message
        ; print message here
        ret
        

;        include "..\asmlibs\string\string.asm"
