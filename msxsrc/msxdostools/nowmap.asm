
        output "msxdostools\nowmap.com"

        include "..\labels.asm"        
        include "..\asmlibs\bdos\bdos.inc"
                        
        org $0100
        
        ld b,C_NOWMAP
        ld de,$4e03                     ; send command
        call EXTBIO

        ld a,(CMD_LENGTH)
        ld b,0
        ld c,a
        ld hl,CMD_LINE
        
        xor a
        ld de,$4e04                     ; block write
        call EXTBIO
        
        ; host will return message
        ; print message here
        ret
        

;        include "..\asmlibs\string\string.asm"
