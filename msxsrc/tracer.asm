
installTracer: 
        ret
        
        di
        ld hl,h_trace
        ld de,$fd9a
        ld bc,5
        ldir
        
        ld hl,$5555
        push hl
        ld de,$6666
        push de
        
        USB_SENDCPUINFO
        
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
        ;USB_SENDCPUINFO
        ret
    