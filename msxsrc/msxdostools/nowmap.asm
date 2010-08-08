
        output "msxdostools\nowmap.com"
        
        include "..\asmlibs\bdos\bdos.inc"
                
        org $0100
        
        ld a,(CMD_LENGTH)
        ld b,a
        ld hl,CMD_LINE
        
       
        


      ret
        
        
        
helpMessage:
        db "NOWIND partition mapper",0
errorDriveNumber:
        db "Invalid drive number!",0

        
        include "..\asmlibs\string\string.asm"
