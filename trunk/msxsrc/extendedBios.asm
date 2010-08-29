
; These function can be used by calling EXTBIO with these registers:
; D = 0x4e
; E = 0 -> determine number of Nowind interfaces
;
; For the following functions, reg_a determines which interface will handle the call (0 is first interface)
; E = 1 -> get slotid
; E = 2 -> send debug message to host
; E = 3 -> block write (to host)
; E = 4 -> block read

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

        DEBUGMESSAGE "EXTBIO"
        ei
        push af
        ld a,d
        sub $4e                         ; Nowind device code?
        jr nz,exit

determineFunction:
        or e                            ; reg_e contains function number
        jr z,numberOfDevices            ; this function is processed by all interfaces

        pop af                          ; this interface?
        dec a
        jp p,exit2                      ; other Nowind interface is requested
        
        add e
        jr z,getNowindSlot              ; function 1
        dec a
        jr z,debugMessage               ; function 2
        dec a
        jr z,sendCommand                ; function 3
        dec a
        jr z,sendCommandWithData        ; function 4
        dec a
        jp z,sendCommandWithString      ; function 5
        dec a
        jp z,readData                   ; function 6

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

numberOfDevices:
        DEBUGMESSAGE "numberOfDevices"
        pop af
        inc a                           ; increment number of devices
        jr exit2
        
getNowindSlot:
        DEBUGMESSAGE "getNowindSlot"
        call getSlotPage1
        scf                             ; can be used to detect Nowind interface 
        ret

debugMessage:
        DEBUGMESSAGE "debugMessage"
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

; Input:    C   command
;           B   argument for commmand
;           HL  argument for command
;           other registers cannot be used
            
sendCommand:
        DEBUGMESSAGE "EXTBIO sendCmd"
        call sendRegisters
        ld (hl),C_COMMAND
        ret


; Input:    C   command
;           B   length of data (max 240, because of hardware buffer)
;           HL  pointer to data
;           IY  argument for command (is copied to DE)
;           other registers cannot be used

sendCommandWithData:
        DEBUGMESSAGE "EXTBIO cmdWithData"
        DEBUGDUMPREGISTERS
        ld a,e
        push iy
        pop de        
        push hl
        call sendRegisters
        ld (hl),C_COMMAND
        pop de
        ex de,hl
        ld c,b
        ld b,0
        ldir
        ret
        
        
; Input:    C   command
;           B   string terminator
;           HL  pointer to string
;           other registers cannot be used

sendCommandWithString:
        DEBUGMESSAGE "EXTBIO cmdWithString"
        push hl
        call sendRegisters
        ld (hl),C_COMMAND
        pop de

.loop:  ld a,(de)
        cp b
        ret z
        ld (hl),a
        inc de
        jr .loop


; Input:    HL  transfer address

readData:
        DEBUGMESSAGE "EXTBIO readData"
        ld a,h
        call blockRead
        DEBUGDUMPREGISTERS
        DEBUGMESSAGE "EXTBIO readDataEnd"
        ret
