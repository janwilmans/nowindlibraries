; AUX device

beginCodeAux:
        if MSXDOSVER = 1
        
        code ! $595d
        ld hl,newAUX                    ; redirect AUX to host
        ld de,$f327
        ld bc,10
        ldir
        nop
        nop
        nop
        nop                             ; do not remove!
        
        else    
        
        ; MSXDOSVER = 2
        code ! $49a3
        ld hl,newAUX                    ; redirect AUX to host
        ld de,$f327
        ld bc,10
        ldir
        nop
        nop
        nop
        nop
        nop                             ; nops needed to override existing code!

        endif


        code @ beginCodeAux             ; restore position of the code below

newAUX: jp AUXin
        nop
        nop
        jp AUXout
        nop
        nop

AUXin:  DEBUGMESSAGE "AUX in"
        push hl
        push de
        push bc

        call sendRegisters
        ld (hl),C_AUXIN
        call enableNowindPage0
        call getHeaderInPage0

        jp nc,.getCharacter

        DEBUGMESSAGE "not connected"
        ld a,$1a                        ; eof
.exit:  pop bc
        pop de
        pop hl
        jp restorePage0

.getCharacter:
        DEBUGMESSAGE "getChar"
        call getHeaderInPage0
        jr c,.getCharacter
        jr .exit


AUXout: DEBUGMESSAGE "AUX out"
        DEBUGDUMPREGISTERS
        push hl
        push de
;        push bc
;        ld a,(RAMAD1) ; TODO: WTF???
;        call RDSLT
        push af
        call sendRegisters
        ld (hl),C_AUXOUT
        pop af
;        pop bc
        pop de
        pop hl
        ret
