; RS232 stuff
;FLAGS   equ $fb1b
;RSFCB   equ $fb04
;RSIQLN  equ $fb06
;DATCNT  equ $fb17

DVINFB  equ 0
HOKVLD  equ $fb20
EXTBIO  equ $ffca
DEVNUM  equ $fb16
MEXBIH  equ $fb07


rs232Init:
        DEBUGMESSAGE "devInit"
        ; EXTBIO already initialized by diskrom (so no need to check) 
        
        ld de,$0801
        xor a
        call EXTBIO
        ld hl,DEVNUM
        ld (hl),a
        
        ld de,$0001
        xor a
        call EXTBIO
        rlca
        rlca
        rlca
        rlca
        or (hl)
        ld (hl),a
        
        di
        ld de,MEXBIH            ; save EXTBIO
        ld hl,EXTBIO
        ld bc,5
        ldir
        ld hl,copyToEXTBIO
        ld de,EXTBIO
        ld bc,copyToEXTBIO.end - copyToEXTBIO
        ldir
        call getSlotPage1
        ld (EXTBIO+1),a
        ei
        ret

copyToEXTBIO:
        rst $30                 ; interslot call
        db 0                    ; dummy
        dw extendedBios
        ret
        push de                 ; DISINT ($ffcf)
        ld e,2
        jr .call
        push de                 ; ENAINT ($ffd4)
        ld e,3
.call:  ld d,0
        push ix
        push iy
        call EXTBIO
        ei
        pop iy
        pop ix
        pop de
        ret        
.end        
        
extendedBios:
        DEBUGMESSAGE "ExtBio"
        push af
        call .determineDevice
        pop af
        jp MEXBIH
        
.determineDevice:        
        ei
        ld a,d
        or a
        jr z,broadcast
        inc a
        jr z,systemExclusive
        cp 8+1
        ret nz

        ld a,e                  ; RS232C device (ID 8)
        or a
        jr z,buildSlotAddrTable
        dec a
        ret nz                  ; unknown function
        
        pop af                  ; return number of channels      
        pop af
        inc a                   ; add 1 channel
        push af
        inc sp
        inc sp
        ret

buildSlotAddrTable:
        call slotAddrAndJumpTable
        xor a
        jp buildTable

broadcast:
        DEBUGMESSAGE "broadcast"
        ld a,e
        or a
        jr z,buildNameTable
        dec a
        jr z,addTrapEntries
        dec a
        jr z,disableInterrupt
        dec a
        ret nz

enableInterrupt:        
        DEBUGMESSAGE "ei"
        ret
        
buildNameTable:
        DEBUGMESSAGE "buildNameTable"
        ld a,8                  
        call buildTable
        xor a
        jp buildTable
        
addTrapEntries:
        DEBUGMESSAGE "trap"
        ; TODO same as 'return number of channels'
        pop af                  ; return number of channels      
        pop af
        inc a                   ; add 1 entry
        push af
        inc sp
        inc sp
        ret
        
disableInterrupt:
        DEBUGMESSAGE "di"
        ret

systemExclusive:
        DEBUGMESSAGE "systemExclusive"
        ld a,e
        or a
        ret nz
        call slotAddrAndJumpTable
        xor a                   ; maker ID (ASCII)
        call buildTable     
        xor a                   ; reserved
        jp buildTable
        
slotAddrAndJumpTable:
        push bc
        push hl
        call getSlotPage0
        pop hl
        pop bc
        call buildTable
        ld a,HIGH jumpTable
        call buildTable
        ld a,LOW jumpTable

buildTable:
        push af ; nodig?
        push bc
        push de ; nodig?
        ld e,a
        ld a,b
        call WRSLT
        ; ei ????
        inc hl
        pop de ; nodig?
        pop bc
        pop af ; nodig?
        ret        
        
jumpTable:
        db DVINFB
        db 0,0                  ; version, reserved
        jp RS232_INIT
        jp RS232_OPEN
        jp RS232_STAT
        jp RS232_GETCHR
        jp RS232_SNDCHR
        jp RS232_CLOSE
        jp RS232_EOF
        jp RS232_LOC
        jp RS232_LOF
        jp RS232_BACKUP
        jp RS232_SNDBRK
        jp RS232_DTR
        ds 3,$c9                ; SETCHN
        ds 3,$c9                ; reserved
        ds 3,$c9                ; reserved
 
RS232_INIT
RS232_OPEN
RS232_STAT
RS232_GETCHR
RS232_SNDCHR
RS232_CLOSE
RS232_EOF
RS232_LOC
RS232_LOF
RS232_BACKUP
RS232_SNDBRK
RS232_DTR
        DEBUGASSERT
