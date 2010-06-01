
installTracer: 
        ret
        
        di
        ld hl,h_trace
        ld de,$fd9a
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
        USB_SENDCPUINFO
        ret
    