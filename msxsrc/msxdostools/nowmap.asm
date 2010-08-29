
        
define  DEBUG
        
        output "msxdostools\nowmap.com"

        include "..\debug.asm"
        include "..\labels.asm"        
        include "..\asmlibs\bdos\bdos.inc"
                        
        org $0100
       
        DEBUGMESSAGE "NOWMAP.COM"
        ld a,(CMD_LENGTH)
        ld b,a
        ld hl,CMD_LINE
        
        xor a                           ; first Nowind interface
        ld c,API_NOWMAP
        ld iy,message
        ld de,$4e04                     ; send command with data
        call EXTBIO

        xor a                           ; read data from host
        ld hl,message
        ld de,$4e06
        call EXTBIO
        
        DEBUGDUMPREGISTERS
        ld de,error
        jr c,exit
      
        ld de,message
exit:   jp String.PrintAsciiz      

error:  db "No connection with host!",0


message:
        db 0

        include "..\asmlibs\string\string.asm"
