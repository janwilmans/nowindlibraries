; debug.asm
; TODO: remove 16-bit writes!
; TODO: implement variable amount of debug data send to host

        MACRO DEBUGMESSAGE string
        IFDEF DEBUG
        ; blueMSX debug info
        ld d,d
        jr .skip
        db string
.skip:
        ENDIF

        IFDEF USBDEBUG
;        ASSERT ($ < $8000)
        push af
        exx
        ld hl,.data
        ld d,HIGH usbwr
        ld bc,.end-.data
        ldir
        jr .end
.data:  db $af, $66, 0, string, 0, 0
.end:   exx
        pop af
        ENDIF
        ENDM

        MACRO DEBUGDUMPSLOTSELECTION
        IFDEF DEBUG
        db $ed,8
        ENDIF
        ENDM

        MACRO DEBUGENABLEDISASM
        IFDEF DEBUG
        db $ed,$0b
        ENDIF
        ENDM

        MACRO DEBUGDISABLEDISASM
        IFDEF DEBUG
        db $ed,$0c
        ENDIF
        ENDM

        MACRO DEBUGASSERT
        IFDEF DEBUG
        db $ed,$0a
        ENDIF
        ENDM

        MACRO DEBUGDUMPMEMHL len
        IFDEF DEBUG
        db $ed,1,len
.skip:  nop
        ENDIF
        ENDM

        MACRO DEBUGDUMPMEM addr, len
        IFDEF DEBUG
        db $ed,2
        dw addr
        db len
.skip:  nop
        ENDIF
        ENDM


        MACRO DEBUGDUMPREGISTERS
        IFDEF DEBUG
        db $ed,7
        ENDIF

        IFDEF USBDEBUG
        ASSERT($ < $8000)
        call sendCpuInfo
        ENDIF
        ENDM

        MACRO DEBUGROUTINESPAGE1
sendCpuInfo:
        push af
        push hl
        push de
        call sendRegisters
        ld (hl),128

        ld (usbwr),ix
        ld (usbwr),iy
        ld (usbwr),sp                   ; needs adjustment! (host does this)

        in a,($a8)                      ; current slotselection
        ld (hl),a
        ld de,($fcc5)                   ; secondairy slot selection
        ld (hl),e
        ld (hl),d
        ld de,($fcc7)
        ld (hl),e
        ld (hl),d

        ld hl,-6                        ; stack dump
        add hl,sp
        ld d,16
.loop:  ld a,(hl)
        dec hl
        ld (usbwr),a
        dec d
        jr nz,.loop

        pop de
        pop hl
        pop af
        ret
        ENDM

; blueMSX
        MACRO BLUEMSX_BREAKPOINT
        IFDEF DEBUG
        ld b,b
        jr $+2
        ENDIF
        ENDM

        MACRO BLUEMSX_SETBREAKPOINT address
        IFDEF DEBUG
        ld b,b
        jr $+4
        dw address
        ENDIF
        ENDM

