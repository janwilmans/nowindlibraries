
installTracer: 
        DEBUGMESSAGE "tracer"
        ld a,5
        call SNSMAT
        and 2
        ret nz              ; 't' pressed?
        
        DEBUGDISASM
        
        ld a,%11110100
        out ($a8),a         ; rom in page 0, nowind in page 1
        
        ld a,0      ; drivenr
        ld b,64     ; 32kb
        ld de,0     ; start sector
        ld hl,0     ; start address
        scf         ; write
        call $4010  ; dskio
        
        di
        halt       
        
        
        
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
        ;DEBUGDISASM
        ei       
        halt            
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
    
