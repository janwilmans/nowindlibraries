; debug.asm contains macros to support debugging the Nowind Interface firmware.

; DEBUGMESSAGE
        macro DEBUGMESSAGE string
        ifdef DEBUG
        ld d,d
        jr .skip
        db string
.skip:  
        endif
        endmacro

; USB_DBMSG
        macro USB_DBMSG string
        ifdef DEBUG
        call sendMessage
        db string
.skip2: nop
        endif
        endmacro

; these macros have no effect on real hardware 
; but provide extra debug information/functionality in the Nowind Emulator

; DEBUGDUMPREGISTERS
        macro DEBUGDUMPREGISTERS
        ifdef DEBUG
        db $ed,7
        endif
        endmacro

; DEBUGDISASM
        macro DEBUGDISASM
        db $ed, $0b
        endmacro
        
; DEBUGDISASMOFF
        macro DEBUGDISASMOFF
        db $ed, $0c
        endmacro        

; BREAKPOINT
        macro BREAKPOINT
        ld b,b
        jr $+2
        endmacro
        
; EMU_DUMPSLOTSELECTION
        macro DEBUGDUMPSLOTSELECTION
        IFDEF DEBUG
        db $ed,8
        ENDIF
        endmacro

; EMU_ASSERT
        macro EMU_ASSERT
        IFDEF DEBUG
        db $ed, $0a
        ENDIF
        endmacro

; EMU_DUMPMEMHL
        macro EMU_DUMPMEMHL len
        IFDEF DEBUG
        db $ed,1,len
.skip:  nop
        ENDIF
        endmacro

; EMU_DUMPMEM
        macro EMU_DUMPMEM addr, len
        IFDEF DEBUG
        db $ed, 2
        dw addr
        db len
.skip:  nop
        ENDIF
        endmacro

; BLUEMSX_SETBREAKPOINT
        macro BLUEMSX_SETBREAKPOINT address
        ld b,b
        jr $+4
        dw address
        endmacro   


