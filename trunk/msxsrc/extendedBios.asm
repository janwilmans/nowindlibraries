
installExtendedBios:

        ; Called from INIENV, so EXTBIO hook is already initialized

        DEBUGMESSAGE "extbio"
        call getEntrySLTWRK             ; save previous EXTBIO hook
        inc hl
        ex de,hl        
        ld hl,EXTBIO
        ld bc,5
        ldir
        
        push hl                         ; determine device number
        xor a
        ld de,$4e01
        call EXTBIO
        pop hl
        ld (hl),a
                
        call GETSLT                     ; install new EXTBIO hook
        ld l,$f7
        ld h,a
        ld (EXTBIO),hl
        ld hl,extendedBios
        ld (EXTBIO+2),hl
        ld a,$c9
        ld (EXTBIO+4),a    
        ret

extendedBios:
        ; broadcast (0x00) not implemented
        ; system exclusive (0xff) not implemented

        ei
        push af
        ld a,d
        cp $4e
        jr z,determineFunction
.exit:
        push hl
        push bc
        call getEntrySLTWRK
        inc l                           ; previous EXTBIO hook 
        push hl
        pop ix
        pop bc        
        pop hl
        pop af
        jp (ix)                         ; process other EXTBIO handles

determineFunction:
        push hl
        ld hl,functionTable - 2 * $4e00
        add hl,de
        add hl,de
        ld a,(hl)
        inc hl
        ld h,(hl)
        ld l,a
        ex (sp),hl
        ret

functionTable:
        dw getNowindSlot
        dw numberOfDevices
        dw debugMessage

getNowindSlot:               
        DEBUGMESSAGE "getNowindSlot"
        pop af
        dec a
        push af
        jp p,extendedBios.exit         ; not this device
        
        pop af
        call getSlotPage1
        scf
        ret
        
numberOfDevices:
        DEBUGMESSAGE "numberOfDevices"
        pop af
        inc a
        push af
        jr extendedBios.exit
        
debugMessage:
        DEBUGMESSAGE "debugMessage"
        pop af
        dec a
        push af
        jp p,extendedBios.exit

        push hl
        call sendRegisters
        ld (hl),C_MESSAGE
        pop hl
.loop:  ld a,(hl)
        inc hl
        ld (usbwr),a
        or a
        jr nz,.loop
        
        pop af        
        ret
