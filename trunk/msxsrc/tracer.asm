
codeTracer: 
        ret
        
        di
        ld hl,h_trace
        ld de,$fd9f
        ld bc,5
        ldir
    
        ei          
.loop:
        inc bc
        ld a,b
        or c
        jr nz,.loop
        
        DEBUGMESSAGE "looping.."
        jr .loop
                       
h_trace:
        jp trace
        nop
        nop
   
trace:        
        ex (sp),hl
        ld d,h
        ld e,l
        ex (sp),hl
        
        USB_DBMSG "interrupt..."   
        ret
    