; Nowind specific

nowindInit:
        ;DEBUGMESSAGE "nowindInit"
        ld a,($faf8)                    ; moet volgens bifi ld a,($2d) zijn, $faf8 is op msx1 deel van de rs232 area?
        or a
        push af
        call z,CHGMOD                   ; SCREEN 0 for MSX1
        pop af
        ld ix,SDFSCR                    ; restore screen mode from clockchip
        call nz,EXTROM

        call PRINTTEXT
        ifndef DEBUG
        db "Nowind USB Diskrom!",0
        else
        db "Nowind USB Diskrom! [debug]",0
        endif

        ret

initDiskBasic:
        DEBUGMESSAGE "initDiskBasic"
        ld hl,DEVICE
        res 7,(hl)

        if MSXDOSVER = 1
        jp $5897
        else
        jp $495b
        endif

; search call statement or device name
findStatementName:
        DEBUGMESSAGE "findStatementName"
        ld de,PROCNM
.loop:  ld a,(de)
        cp (hl)
        jr nz,.nextStatement
        inc hl
        or a
        ret z                           ; name found
        inc de
        jr .loop
.nextStatement:
        xor a
        ld c,a
        cpir
        inc hl
        inc hl
        or (hl)
        jr nz,findStatementName
        scf                             ; not found
        ret

sendRegisters:
        push af
        ld a,h
        ld h,HIGH usbWritePage1
        ld (hl),$af      ; send header
        ld (hl),$05
        ld (hl),c
        ld (hl),b
        ld (hl),e
        ld (hl),d
        ld (hl),l
        ld (hl),a        ; send register h
        pop de
        ld (hl),e        ; send register f
        ld (hl),d        ; send register a
        ret

; GetHeader, returns a = 2 if timeout occurs
;getHeaderHigh:
;   ld h,HIGH usbReadPage0
getHeader:
        DEBUGMESSAGE "gH"
        ld b,HIGH 65535                 ; 42000 * 60 states ~ 0,7 sec (time out)
.loop:  ld a,(hl)
.chkaf: cp $af
        jr z,.chk05
        dec bc
        ld a,b
        or c
        jr nz,.loop
        DEBUGMESSAGE "getHeader timeout!"
        ld a,2                          ; not ready
        scf
        ret

.chk05: ld a,(hl)
        cp $05
        jr nz,.chkaf
        ld a,(hl)
        ret

sendMessage:
        ;DEBUGMESSAGE "sendMsg"
        ex (sp),hl
        push af
        push de
        push hl
        call sendRegisters
        ld (hl),C_MESSAGE
        pop hl
.loop:  ld a,(hl)
        inc hl
        ld (usbWritePage1),a
        or a
        jr nz,.loop
        pop de
        pop af
        ex (sp),hl
        ret

; AUX device

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

    ;call sendRegisters
        ;ld (hl),C_AUXIN
        ;call enableNowindPage0
        ;ld h,HIGH usbReadPage0
        ;call getHeader
    SEND_CMD_AND_WAIT C_AUXIN

        jp nc,.getCharacter

        DEBUGMESSAGE "not connected"
        ld a,$1a                        ; eof
.exit:  pop bc
        pop de
        pop hl
        jp restorePage0

.getCharacter:
        DEBUGMESSAGE "getChar"
        call getHeader
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


; send 32 bytes starting from address specified by DE to the usb
sdendFCB:
        push de
        push bc

        ld b,32
.loop:  ld a,(de)
        ld (usbWritePage1),a
        inc de
        djnz .loop
        pop bc
        pop de
        ret

; receive 32 bytes and write to the address specified by DE
receiveFCB:
        push de
        push bc

        ld b,32
.loop:  ld a,(usbReadPage0)
        ld (usbWritePage1),a      ; loop back
        ld (de),a
        inc de
        djnz .loop
        pop bc
        pop de
        ret

; in:  hl = callback to execute
;      bc = arguments for callback
; out: bc = callback return data
; changed: all
; requirements: stack available
executeCommandNowindInPage0:
        push hl
        push bc
        call enableNowindPage0
        ld h,HIGH usbReadPage0
        call getHeader
        jr c,.exit      ; timeout occurred?

        pop bc
        ld hl,.restorePage
        ex (sp),hl
        jp (hl)     ; jump to callback, a = first data read by getHeader, ret will resume at .restorePage

.exit:  call .restorePage
        pop hl
        pop bc
        ret

.restorePage:
        push bc
        call restorePage0
        pop bc
        ret

executeCommandNowindInPage2:
        push hl
        push bc

        call getSlotPage2               ; enable nowind in page 2
        ld ixl,a
        call getSlotPage1
        ld ixh,a                        ; ixh = current nowind slot
        ld h,$80
        call ENASLT
        jp .page2

        PHASE $ + $4000
.page2:
        ld a,(RAMAD1)                   ; enable RAM in page 1
        ld h,$40
        call ENASLT

        ld h,HIGH usbReadPage2
        call getHeader + $4000
        jr c,.exit      ; timeout occurred?

        pop bc
        ld hl,.restorePage
        ex (sp),hl
        jp (hl)     ; jump to callback, a = first data read by getHeader, ret will resume at .restorePage

.exit:  pop bc
        pop hl

.restorePage:
        push af
        ld a,ixh                        ; enable nowind in page 1
        ld h,$40
        call ENASLT
        jp .page1

        DEPHASE
.page1:
        ld a,ixl
        ld h,$80
        call ENASLT                     ; restore page 2
        pop af
        ret

blockRead:
; Input     A   Transfer address (high byte)

        rlca                            ; tranfer address < 0x8000 ?
        jr c,.page23

        ld hl,blockRead01 + $4000
        call executeCommandNowindInPage2
        DEBUGMESSAGE "more?"
        ret c                           ; not ready
        ; er kan nog meer komen voor de volgende page!
        DEBUGMESSAGE "doorgaan!"
        ret
.page23:
        ld hl,blockRead23
        jp executeCommandNowindInPage0

     
blockRead01:
        DEBUGMESSAGE "br01"
        ld h,HIGH usbReadPage2
        jr .start2
.start:
        call getHeader + $4000
.start2:
        DEBUGDUMPREGISTERS
        ret c                           ; return on timeout
        and a
        ret z                           ; exit blockRead01

        call blockReadTranfer + $4000
        jr .start

blockRead23:
        DEBUGMESSAGE "br23"
        DEBUGDUMPREGISTERS
        ld h,HIGH usbReadPage0
        jr .start2
.start:
        call getHeader
.start2:
        ret c                           ; return on timeout
        and a
        ret z                           ; exit blockRead23

        DEBUGMESSAGE "br23_a"
        
        call blockReadTranfer
        jr .start

; used by blockRead01 and blockRead23
; Input:    HL = usb read address (either usbReadPage0 or usbReadPage2)

error255:
        ; TODO timeout
        DEBUGMESSAGE ".err255"
        ld a,(hl)
        cp 255
        jr z,error255
        ; b moet nog aangepast... (hoe?)
        ld c,a
        jp blockReadTranfer.good

blockReadTranfer:
        DEBUGMESSAGE "btf"
        ld iy,0                         ; save stack pointer
        add iy,sp
        ld e,(hl)                       ; transfer address
        ld d,(hl)
        ex de,hl
        ld sp,hl
        ex de,hl
        ld b,(hl)                       ; amount of 128 byte blocks (max 32kB)
        DEBUGDUMPREGISTERS
.loop:
        DEBUGMESSAGE ".loop"
        ld a,(hl)                       ; header
        ld c,a
        cp 255
        jr z,error255

.good:
        repeat 64                       ; blocks of 128 bytes hardcoded (NowindHost.cpp)
        ld d,(hl)
        ld e,(hl)
        push de
        endrepeat
      
        bit 7,h                         ; if HL < 0x8000, this code is executing in page 1 (otherwise in page 2)
        ld a,(hl)
        jr nz,.codeInPage2

.codeInPage1:
        ld (usbWritePage1),a            ; return tail
        cp c
        jr nz,.errorInPage1
.nextLoopInPage1:
        dec b
        jp nz,.loop
        ld sp,iy                        ; restore stack pointer
        ret
        
.errorInPage1:
        DEBUGMESSAGE ".err1"
        ; TODO timeout
        ld a,(hl)
        cp c
        jp z,.nextLoopInPage1
        jr .errorInPage1

.codeInPage2:
        ld (hl),a                       ; return tail
        cp c
        jr nz,.errorInPage2
        dec b
.nextLoopInPage2:
        jp nz,.loop + $4000
        ld sp,iy                        ; restore stack pointer
        ret
        
.errorInPage2:
        DEBUGMESSAGE ".err2"
        ; TODO timeout
        ld a,(hl)
        cp c
        jr z,.nextLoopInPage2
        jr .errorInPage2


        ; include flash routine only once
        if MSXDOSVER = 2

flashWriter:
        ;DEBUGMESSAGE "flashWriter"
        ld a,3
        call SNSMAT
        and 8
        ret nz

        call PRINTTEXT
        db 10,13," FlashROM",10,13," "
        ds 33,"."
        db 13," ",0

        call getSlotPage1
        call enableSlotPage0

        ld hl,waitForFlashCommand
        ld de,$c000
        push de
        ld bc,flasherEnd - $c000
        ldir
        ret

        endif
