; init.asm contains initialization and startup code 

nowindInit:
        call flashWriter

        DEBUGMESSAGE "nowindInit"
        ld a,($2d)
        or a
        push af
        call z,INITXT                   ; SCREEN 0 (MSX1)
        pop af
        ld ix,SDFSCR                    ; restore screen mode from clockchip (on MSX2 and higher)
        call nz,EXTROM

        call PRINTTEXT
        ifdef DEBUG
        db "Nowind Interface v4.0 [beta]",0
        else
        db "Nowind Interface v4.0",0
        endif

        call enableNowindPage0          ; clear hostToMSXFifo by reading 4Kb of random data
        ld bc,4096
.loop:  ld a,(usbReadPage0)
        dec bc
        ld a,b
        or c
        jr nz,.loop
        call restorePage0
        
getBootArgs:
        DEBUGMESSAGE "Any commands?"
        call enableNowindPage0
        ld c,0                      ; c=0 means reset startup queue index
.loop:  ld b,0                      ; b=0 means request startup command
        call sendRegisters
        ld (hl),C_CMDREQUEST
        call getHeaderInPage0
        jr c,noNextCommand

        ld d,(hl)
        ld d,(hl)
        ld d,(hl)

        ; todo: read command here
        and a
        jr z,noNextCommand
        DEBUGMESSAGE "Got cmd!"

        ld a,(hl)
        ;cp 1
        ;jr z, Com

        ld c,1
        jr .loop

noNextCommand:
        call restorePage0
        DEBUGMESSAGE "End of startup cmds"

        call sendRegisters
        ld (hl),C_GETDOSVERSION
        call enableNowindPage0
        call getHeaderInPage0

        call restorePage0
        jp c,bootMSXDOS1                ; no reply (host not connected?)
                                        
        and a
        jp nz,$47d6                     ; boot MSXDOS2

bootMSXDOS1:
        DEBUGMESSAGE "switch to DOS1"
        ld hl,$576f                     ; boot MSXDOS1
        push hl
        ld a,4
        jp switchBank
