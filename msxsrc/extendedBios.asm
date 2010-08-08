
installExtendedBios:

        ; Called from INIENV, so EXTBIO hook is already initialized

        DEBUGMESSAGE "extbio"
        call getEntrySLTWRK             ; save previous EXTBIO hook
        inc hl
        ex de,hl
        ld hl,EXTBIO
        ld bc,5
        ldir

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
        ; broadcast (D = 0x00) not implemented
        ; system exclusive (D = 0xff) not implemented

        ei
        push af
        ld a,d
        sub $4e                         ; Nowind device code?
        jr nz,exit

determineFunction:
        or e                            ; reg_e contains function number
        jr z,getNowindSlot
        dec a
        jr z,numberOfDevices
        dec a
        jr z,debugMessage
        dec a
        jr z,writeBlock
exit:
        pop af
exit2:
        push hl                         ; pass control to previous EXTBIO implementations
        push bc
        push af
        call getEntrySLTWRK
        pop af
        pop bc
        inc l                           ; previous EXTBIO hook
        ex (sp),hl
        ret

        
getNowindSlot:
        DEBUGMESSAGE "getNowindSlot"
        pop af                          ; this interface?
        dec a
        jp p,exit2

        call getSlotPage1
        scf                             ; can be used to detect Nowind interface 
        ret

numberOfDevices:
        DEBUGMESSAGE "numberOfDevices"
        pop af
        inc a                           ; increment number of devices
        jr exit2

debugMessage:
        DEBUGMESSAGE "debugMessage"
        pop af                          ; this interface?
        dec a
        jp p,exit2

        push hl
        call sendRegisters
        ld (hl),C_MESSAGE
        pop hl
.loop:  ld a,(hl)
        inc hl
        ld (usbWritePage1),a
        or a
        jr nz,.loop
        ret

writeBlock:
        DEBUGMESSAGE "writeBlock"
        pop af                          ; this interface?
        dec a
        jp p,exit2
        
        ; TODO: use blockWrite here!
        ret  
