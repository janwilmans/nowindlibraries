
installTracer: 
        ret
        
        di
        
        ld a,32           ; enable line interrupt
        out ($99),a
        ld a,$80+1
        out ($99),a
        
        ld hl,h_trace
        ld de,$fd9a
        ld bc,5
        ldir
        
        ld hl,$5555       ; mark the stack  
        push hl
        ld de,$6666       ; mark the stack  
        push de
        
        DEBUGMESSAGE "start trace"
        ei          
        nop               ; interrupt will first occur after the instruction after ei
        halt
        DEBUGDISASM
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
        USB_SENDCPUINFO     ; now hardcoded 
        ret
    
