; Nowind specific

nowindInit:
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
        jp restorePage0


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
        ld (hl),$af                     ; send header
        ld (hl),$05
        ld (hl),c
        ld (hl),b
        ld (hl),e
        ld (hl),d
        ld (hl),l
        ld (hl),a                       ; send register h
        pop de
        ld (hl),e                       ; send register f
        ld (hl),d                       ; send register a
        ret

getHeaderInPage0:
        DEBUGMESSAGE "gH0"
        ld h,HIGH usbReadPage0
.init:
        ld b,HIGH 65535                 ; 42000 * 60 states ~ 0,7 sec (time out)
.loop:  ld a,(hl)
.chkaf: cp $af
        jr z,.chk05
        dec bc
        ld a,b
        or c
        jr nz,.loop
        DEBUGMESSAGE "Timeout!"
        ld a,2                          ; not ready
        scf
        ret

.chk05: ld a,(hl)
        cp $05
        jr nz,.chkaf
        ld a,(hl)
        ret

        PHASE $ + $4000

getHeaderInPage2:
        DEBUGMESSAGE "gH2"
        ld h,HIGH usbReadPage2
        jr getHeaderInPage0.init + $4000 
        
        DEPHASE

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
        ld de,.restorePage
        push de
        push hl                         ; address callback
        push bc
        call enableNowindPage0
        pop bc
        ret                             ; execute callback and restore page afterwards

.restorePage:
        push bc
        call restorePage0
        pop bc
        ret

executeCommandNowindInPage2:
        ld de,.restorePage
        push de
        push hl                         ; address callback
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
        pop bc
        ret                             ; execute callback and restore page afterwards

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

        ld hl,blockRead01
        call executeCommandNowindInPage2
        ret c                           ; not ready
        ret z                           ; done! (no more data for page23)
        ;DEBUGMESSAGE "more data for .page23!"
.page23:
        ld hl,blockRead23
        jp executeCommandNowindInPage0
     
        PHASE $ + $4000

blockRead01:
        DEBUGMESSAGE "br01"
        call getHeaderInPage2
        ret c

        dec a
        jr z,.fastTransfer
        ret m                           ; exit (more data for page23)
        dec a
        ret z                           ; exit (done!)        

        call slowTransfer + $4000
        ld (usbWritePage2),a            ; return header
        jr blockRead01

.fastTransfer:
        call blockReadTranfer + $4000
        jr blockRead01

        DEPHASE

blockRead23:
        DEBUGMESSAGE "br23"
        call getHeaderInPage0
        ret c                           ; return on timeout

        dec a
        jr z,.fastTransfer
        ; ret m (not necessary)
        dec a
        ret z
        
        call slowTransfer
        ld (usbWritePage1),a            ; return header
        jr blockRead23

.fastTransfer:
        DEBUGMESSAGE "fast"
        call blockReadTranfer
        jr blockRead23

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
        jr blockReadTranfer.good

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
        ;DEBUGDUMPREGISTERS
.loop:
        ;DEBUGMESSAGE ".loop"
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
        jr z,.nextLoopInPage1
        jr .errorInPage1

.codeInPage2:
        ld (hl),a                       ; return tail
        cp c
        jr nz,.errorInPage2
.nextLoopInPage2:
        dec b
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

slowTransfer:
        DEBUGMESSAGE "SlowTR"
        ld e,(hl)                       ; slow transfer
        ld d,(hl)
        ld c,(hl)        
        ld b,(hl)
        ld a,(hl)                       ; header
        ldir
        ld b,(hl)
        cp a                            ; check block (header == tail?)
        ld a,b
        ret

blockWrite:
        rlca
        jr c,.page23

        ld hl,blockWrite01
        call executeCommandNowindInPage2
        ret c                           ; return error (error code in a)
        ret pe                          ; host returns 0xfe when data for page 2/3 is available
        DEBUGMESSAGE "doorgaan!"

.page23:
        ld hl,blockWrite23
        call executeCommandNowindInPage0
        DEBUGMESSAGE "back"
        ret c                           ; return error (error code in a)
        xor a                           ; some software (wb?) requires that a is zero, because they do not check the carry
        ret

        PHASE $ + $4000

blockWrite01:
        DEBUGMESSAGE "blkWr01"
        call getHeaderInPage2
        ret c                           ; exit (not ready)
        or a
        ret m                           ; exit (no error)
        jr nz,.error

        DEBUGMESSAGE "wr01"
        ld e,(hl)                       ; address
        ld d,(hl)
        ld c,(hl)                       ; number of bytes
        ld b,(hl)
        ld a,(hl)                       ; block sequence number

        ex de,hl
        ld de,usbWritePage2
        ld (de),a                       ; mark block begin
        ldir
        ld (de),a                       ; mark block end
        jr blockWrite01

.error: scf
        ld a,(hl)                       ; get error code
        ret

        DEPHASE

blockWrite23:
        DEBUGMESSAGE "blkWr23"
        call getHeaderInPage0
        ret c                           ; exit (not ready)
        or a
        ret m                           ; exit (no error)
        jr nz,.error

        DEBUGMESSAGE "wr23"
        ld e,(hl)                       ; address
        ld d,(hl)
        ld c,(hl)                       ; number of bytes
        ld b,(hl)
        ld a,(hl)                       ; block sequence number

        ex de,hl
        ld d,HIGH usbWritePage1
        ld (de),a                       ; mark block begin
        ldir
        ld (de),a                       ; mark block end
        jr blockWrite23

.error: scf
        ld a,(hl)                       ; get error code
        ret


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
