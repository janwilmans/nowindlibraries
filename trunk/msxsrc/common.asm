; Nowind specific

nowindInit:
        ;DEBUGMESSAGE "nowindInit"
        ld a,($faf8)										; moet volgens bifi ld a,($2d) zijn, $faf8 is op msx1 deel van de rs232 area?
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
        ld h,HIGH usbwr
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
;		ld h,HIGH usbrd
getHeader:
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
        ld (usbwr),a
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
        ;ld h,HIGH usbrd
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
        ld (usbwr),a
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
.loop:  ld a,(usbrd)
	ld (usbwr),a			; loop back
        ld (de),a
        inc de
        djnz .loop
        pop bc
        pop de
        ret
        
blockRead:
        ;DEBUGMESSAGE "br"
        ld h,HIGH usbrd
        call getHeader
        ret c                           ; return on timeout 
        and a
        ret z                           ; resume
        
        ld iy,0                         ; save stack pointer
        add iy,sp
        ld e,(hl)                       ; transfer address
        ld d,(hl)
        ex de,hl
        ld sp,hl
        ex de,hl
        ld b,(hl)                       ; amount of 128 byte blocks (max 32kB)       

.loop:
        ld c,(hl)                       ; header
        cp 255
        jp z,.error255

.good:        
        repeat 64                       ; blocks of 128 bytes hardcoded (NowindHost.cpp)
        ld d,(hl)
        ld e,(hl)
        push de
        endrepeat
        
        ld a,(hl)                       ; tail
        ld (usbwr),a
        
        cp c
        jr nz,.error
.nextLoop:
        dec b
        jp nz,.loop
        
        ld sp,iy                        ; restore stack pointer        
        jp blockRead
         
.error: 
        DEBUGMESSAGE ".err"
        ; TODO timeout
        ld a,(hl)
        cp c
        jp z,.nextLoop
        jr .error

.error255: 
        ; TODO timeout
        DEBUGMESSAGE ".err255"
        ld a,(hl)
        cp 255
        jr z,.error255
        ; b moet nog aangepast... (hoe?)
        ld c,a 
        jp .good
        
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
