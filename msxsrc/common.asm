; common.asm contains all code that is shared
; between DOS1 (bank 4) and DOS2 (bank 0-3)
; this means that this code is assembled twice, once with MSXDOSVER = 1
; and again with MSXDOSVER = 2 and used in different contexts.

initDiskBasic:
        DEBUGMESSAGE "initDiskBasic"

        ld a,($f347)                    ; TODO! use SLTWRK
        rlca
        jr nc,.continue
        srl a
        inc a
        ld (DEVICE),a
.continue:        
        jp ORIGINAL_HOOK_RUNC

; function: search for a clockchip and initialize it if present
;           the host decides to use the msx' own clockchip date/time if present, or
;           to set the host date/time.
; in:  ?
; out: zero flag set if present??
; changed: af, bc
initClockchip:
        DEBUGMESSAGE "initClockchip"
        
        push af
        push bc
        push de
        push hl
        
        call sendRegisters
        ld (hl),C_GETDATE       

        pop hl
        pop de
        pop bc 
        pop af
        xor a
        ret
        
;       YF248   equ     0F248H                  ; current day (1..31)
;115	YF249   equ     0F249H                  ; current month (1..12)
;116	YF24A   equ     0F24AH                  ; current year (offset to 1980)
;117	YF24C   equ     0F24CH                  ; current days since 1-1-1980
;118	YF24E   equ     0F24EH                  ; current day of week (0=sunday)        
        

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

; function: send af, bc, de and hl registers to the host, but dont change any registers.
;           mainly used for quick first implementations.
; in: af, bc, de, hl
; out: none
; changed: none
sendRegistersSafe:
        push af    
        push de
        push hl
        call sendRegisters
        pop hl
        pop de
        pop af
        ret

; function: send af, bc, de and hl registers to the host 
; in: af, bc, de, hl
; out: none
; changed: a, h, de

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

; function: receive a value for AF, BC, DE and HL from the host.
; in: none
; out: a, bc, de, hl, ix, iy, 
;      all flags in f except bit 0 (carry is set on timeout)
; requirements: stack available, page 0 will switched to the slot of page 1 and restored after communication.

receiveRegisters:
        ;DEBUGMESSAGE "receiveRegisters"
            
        call enableNowindPage0
.loop:        
        call getHeaderInPage0       ; h = HIGH usbReadPage0
        jp c, restorePage0          ; timeout, exit with carry (F is saved by restorePage0)
        
        ; dummy 0xff was already been read in A, by getHeaderInPage0
        ld e,(hl)                   ; header

        ; incoming data order F A C B E D L H X I Y I
        ld c,(hl)
        ld b,(hl)
        push bc
        ld c,(hl)
        ld b,(hl)
        push bc
        ld c,(hl)
        ld b,(hl)
        push bc
        ld c,(hl)
        ld b,(hl)
        push bc
        ld c,(hl)
        ld b,(hl)
        push bc
        ld c,(hl)
        ld b,(hl)
        push bc
        
        ld a,(hl)                   ; tail
        ld (usbWritePage1),a
        cp e
        jr z,.done
        
.retry:
        pop bc        ; throw away data
        pop bc        
        pop bc        
        pop bc     
        pop bc        
        pop bc     
        jr  .loop   
        
.done:
        call restorePage0
        pop iy              ; pop our return values
        pop ix
        pop hl                  
        pop de
        pop bc
        pop af        
        scf             ; reset carry (normal exit)
        ccf
        ret

; function: waits for a header (0xAF 0x05) to be received, 
; in: none
; out: A = first byte of data after the header
;      H = high byte of usbReadPage0 address
;      carry flag is set and A=2 if a timeout occurs
; changed: BC, 
; requirements: nowind must be enabled in page0

getHeaderInPage0:
        ;DEBUGMESSAGE "gH0"
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
        DEBUGMESSAGE "GetHeader Timeout!"
        ld a,2                          ; not ready , todo: why is A set here? is that not DISKIO specific?
        scf
        ret

.chk05: ld a,(hl)
        cp $05
        jr nz,.chkaf
        ld a,(hl)                       ; read the first data byte, todo: dont do this here, it is confusing.
        ret

        PHASE $ + $4000

getHeaderInPage2:
        ;DEBUGMESSAGE "gH2"
        ld h,HIGH usbReadPage2
        jr getHeaderInPage0.init + $4000 
        
        DEPHASE

; used by the USB_DBMSG macro
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

; used by the USB_SENDCPUINFO macro
; show now only be called from $FD9A (for correct stack unwind)
sendCpuInfo:
        push af
        push hl
        push de
        call sendRegisters
        ld (hl),C_CPUINFO

        ld (usbWritePage1),ix
        ld (usbWritePage1),iy
        ld (usbWritePage1),sp           ; needs adjustment! (host does this)

        in a,($a8)                      ; current slotselection
        ld (hl),a
        ld de,($fcc5)                   ; secondairy slot selection
        ld (hl),e
        ld (hl),d
        ld de,($fcc7)
        ld (hl),e
        ld (hl),d

; on the stack:
; 6 bytes (push af,hl,de at start of this routine
; 2 bytes (the 'call sendCpuInfo' return address)
; 2 bytes (0x0c4d, the rst $38 -> jp $0c3d -> call $FD9A return address)
; 20 bytes (push hl,de,bc,af,hl',de',bc'af',iy,ix)
; 2 bytes: the PC where the interrupt occurred!

        ld hl,30                        ; stack dump
        add hl,sp
        ld d,34                         ; dump the PC + 16 last bytes on the stack
.loop:  ld a,(hl)
        inc hl
        ld (usbWritePage1),a
        dec d
        jr nz,.loop

        pop de
        pop hl
        pop af
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

; function: execute a routine specified by HL, nowind will be enabled in page0 before the call
;           and switched back afterwards
; in:  hl = callback to execute
;      bc = arguments for callback
; out: bc = callback return data
; changed: all?
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

; function: execute a routine specified by hl, nowind will be enabled in page2 before the call
;           and switched back afterwards
; in:  hl = callback to execute
;      bc = arguments for callback
; out: bc = callback return data
; changed: af, de, hl, ix
;
; iy and shadow registers might be unchanged, but it depends on the implementation of ENASLT, its documentation says, changed: all registers.
; requirements: stack available

executeCommandNowindInPage2:
        ld de,.restorePage
        push de
        push hl                         ; address callback
        push bc
        call getSlotPage2               ; get current slot of page2 
        ld ixl,a                        ; store it
        call getSlotPage1
        ld ixh,a                        ; ixh = current nowind slot
        ld h,$80
        call ENASLT                     ; enable nowind in page 2
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
        ld a,ixh                        ; restore nowind in page 1
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

; function: read a block of data from the host. The host tells us what kind(s) of transfer(s) to do.
;           a data block is automatically split up into multiple transfers by the host when page borders are crossed. 
;           Also when the amount of bytes is odd, a different byte-for-byte transfer is done (which is slower, but rarely needed)
;
; in:  A = Transfer address (high byte)
; out: carry = error / timeout occurred
; changed: all?
; requirements: stack available
; todo: refactor 'blockRead' so the host detemines what kind of transfer to do (loose the requirement for A as input )
;       just do a blockRead01 always and have the host switch to > 0x8000 when needed.
blockRead:
        rlca                            ; tranfer address < 0x8000 ?
        jr c,.page23

        ld hl,blockRead01
        call executeCommandNowindInPage2
        ret c                           ; not ready
        ret nz                          ; done! (no more data for page23)

        ;DEBUGMESSAGE "more data for .page23!"
.page23:
        ld hl,blockRead23
        jp executeCommandNowindInPage0
     
        PHASE $ + $4000

blockRead01:
        ;DEBUGMESSAGE "br01"
        call getHeaderInPage2
        ret c

        cp 1
        jr z,.fastTransfer
        and a
        ret z                           ; exit (more data for page23)
        cp 2
        ret nz                          ; exit (return code 2-255, bit7 signals error)

        call slowTransfer + $4000
        ld (usbWritePage2),a            ; return header
        jr blockRead01

.fastTransfer:
        call blockReadTranfer + $4000
        jr blockRead01

        DEPHASE

blockRead23:
        //DEBUGMESSAGE "br23"
        call getHeaderInPage0
        //DEBUGDUMPREGISTERS
        ret c                           ; return on timeout

        cp 1
        jr z,.fastTransfer
;        and a
;        ret z                          ; exit (more data for page23)
        cp 2
        ret nz                          ; exit (return code 2-255, bit7 signals error)
        
        //DEBUGMESSAGE "slw23"
        call slowTransfer
        ld (usbWritePage1),a            ; return header
        jr blockRead23

.fastTransfer:
        //DEBUGMESSAGE "fast23"
        call blockReadTranfer
        jr blockRead23

; used by blockRead01 and blockRead23
; Input:    HL = usb read address (either usbReadPage0 or usbReadPage2)
; Routine: 'blockReadTranfer'
; todo: we should re-think this so 'deadlocks' cannot occur, using a timeout or retry mechnism
invalidHeader:
        ; TODO timeout
        DEBUGMESSAGE "invalidHeader"
        ld a,(hl)
        cp 255
        jr z,invalidHeader
        ld c,a
        jr blockReadTranfer.good

blockReadTranfer:
        ;DEBUGMESSAGE "btf"
        ld iy,0                         ; save stack pointer
        add iy,sp
        ld e,(hl)                       ; transfer address
        ld d,(hl)
        ex de,hl
        ld sp,hl
        ex de,hl
        ld b,(hl)                       ; amount of 128 byte blocks (max 32kB)
.loop:
        ;DEBUGMESSAGE ".loop"
        ld a,(hl)                       ; header
        ld c,a
        inc a
        jr z,invalidHeader

.good:
        repeat 64                       ; blocks of 128 bytes, this should match HARDCODED_READ_DATABLOCK_SIZE, NowindHost.cpp
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
        ;DEBUGMESSAGE ".err1"
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
        ;DEBUGMESSAGE ".err2"
        ; TODO timeout
        ld a,(hl)
        cp c
        jr z,.nextLoopInPage2
        jr .errorInPage2

slowTransfer:
        ;DEBUGMESSAGE "SlowTR"
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

; function: write a block of data to the host. This works like the blockRead function, the host tells us what kind(s) of transfer(s) to do.
;           a data block is automatically split up into multiple transfer assignments by the host when page borders are crossed. 
;           Unlike the blockRead function only one kind of transfer routine is used, blocks of <sequencenr> <d0> .. <dx> <sequencenr> are send
;           The sequence numbers are generated and checked by the host.
;
; in:  A = Transfer address (high byte)
; out: carry = error / timeout occurred
; changed: all, see executeCommandNowindInPageX
; requirements: stack available

blockWrite:
        rlca
        jr c,.page23

        ld hl,blockWrite01
        call executeCommandNowindInPage2
        ret c                           ; return error (error code in a)
        ret pe                          ; host returns 0xfe when data for page 2/3 is available
                                        ; todo: using ret pe, relying on 0xfe makes returning other error codes in A impossible?
        ;DEBUGMESSAGE "doorgaan!"

.page23:
        ld hl,blockWrite23
        call executeCommandNowindInPage0
        ;DEBUGMESSAGE "back"
        ret c                           ; return error (error code in a)
        xor a                           ; some software (wb?) requires that a is zero, because they do not check the carry
        ret                             ; todo: why is the zero not implemented for blockWrite01 above?

        PHASE $ + $4000

blockWrite01:
        ;DEBUGMESSAGE "blkWr01"
        call getHeaderInPage2
        ret c                           ; exit (not ready)
        or a
        ret m                           ; exit (no error)
        jr nz,.error

        ;DEBUGMESSAGE "wr01"
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
        ;DEBUGMESSAGE "blkWr23"
        call getHeaderInPage0
        ret c                           ; exit (not ready)
        or a
        ret m                           ; exit (no error)
        jr nz,.error

        ;DEBUGMESSAGE "wr23"
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
