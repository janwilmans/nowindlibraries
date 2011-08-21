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
        ifdef USBDEBUG
        call sendMessage
        db string
.skip2: nop
        endif
        endmacro
        
; SAFE_CALL
        macro SAFE_CALL address
        push af
        push bc
        push de
        push hl
        call address
        push hl
        push de
        push bc
        push af
        endmacro        
        
; USB_SENDCPUINFO
        macro USB_SENDCPUINFO
        ifdef USBDEBUG
        call sendCpuInfo        ; modifing this macro will affect the stack-unwind requirements, 
        endif                   ; do not change it unless you know what your doing, see sendCpuInfo
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
        ifdef DEBUG
        db $ed, $0b
        endif        
        endmacro
        
; DEBUGDISASMOFF
        macro DEBUGDISASMOFF
        ifdef DEBUG
        db $ed, $0c
        endif        
        endmacro        

; BREAKPOINT
        macro BREAKPOINT
        ifdef DEBUG
        ld b,b
        jr $+2
        endif        
        endmacro
        
; EMU_DUMPSLOTSELECTION
        macro DEBUGDUMPSLOTSELECTION
        ifdef DEBUG
        db $ed,8
        endif        
        endmacro

; EMU_ASSERT
        macro EMU_ASSERT
        ifdef DEBUG
        db $ed, $0a
        endif        
        endmacro

; EMU_DUMPMEMHL
        macro EMU_DUMPMEMHL len
        ifdef DEBUG
        db $ed,1,len
.skip:  nop
        endif        
        endmacro

; EMU_DUMPMEM
        macro EMU_DUMPMEM addr, len
        ifdef DEBUG
        db $ed, 2
        dw addr
        db len
.skip:  nop
        endif        
        endmacro

; BLUEMSX_SETBREAKPOINT
        macro BLUEMSX_SETBREAKPOINT address
        ifdef DEBUG
        ld b,b
        jr $+4
        dw address
        endif        
        endmacro   
