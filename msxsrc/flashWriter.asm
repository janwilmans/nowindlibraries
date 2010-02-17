; flashWriter.asm
; Flashes and erases the AMD29F040/M29F032-D
     
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

        ld hl,.source
        ld de,$c000
        push de
        ld bc,flasherEnd - $c000
        ldir
        ret
        
.source:     
        PHASE $c000  
        
waitForHeader:
        ld h,HIGH usbrd
        ld a,(hl)
.chkbb: cp $bb
        jr nz,waitForHeader
        ld a,(hl)
        cp $55
        jr nz,.chkbb       

        ld a,(hl)
        cp $a2
        jp z,verifyFlash
        cp $a3
        jr z,writeFlash
        cp $a4
        jr z,chipErase
        cp $a5
        jr z,eraseSector
        cp $a6
        jr nz,waitForHeader
    
autoselectMode:
        ld a,$90
        call writeCommandSequence

        ld hl,($4000)
        ex de,hl
        ld h,HIGH usbwr
        ld (hl),$aa
        ld (hl),$55
        ld (hl),e                       ; manufacturer ID (0x01 = AMD)
        ld (hl),d                       ; device ID (0xA4 = AM29F040, 0x41 = AM29F032B)

        call resetFlashDevice
        jr waitForHeader

eraseSector:
        ;DEBUGMESSAGE "sector erase"
        ld a,"e"
        out ($98),a

        ld a,(hl)                       ; get sector number 0..63
        sla a
        sla a
        ld (mapper),a                   ; select sector

        ld a,$80
        call writeCommandSequence
        ld a,$30        
        call writeCommandSequence

        call checkCommandExecution
        jp acknowledge
        

chipErase:
        ;DEBUGMESSAGE "chip erase"
        ld a,"E"
        out ($98),a

        ld a,$80
        call writeCommandSequence
        ld a,$10
        call writeCommandSequence

        call checkCommandExecution
        jr acknowledge


writeFlash:
        ;DEBUGMESSAGE "write"
        ld e,(hl)                       ; address
        ld d,(hl)

        ld a,d
        or e
        call z,updateBar

        ld a,(hl)                       ; bank
        ld (mapper),a
        
        ld b,128                        ; data is written in blocks of 128 bytes
.loop:  ld a,$a0
        call writeCommandSequence
        ld a,(usbrd)
        ld (de),a                       ; write data to flash
        inc de
        call checkCommandExecution
        djnz .loop
        
        ld a,2
        jr acknowledge        
        
verifyFlash:
        ;DEBUGMESSAGE "verify"
        ld hl,usbrd
        ld e,(hl)                       ; address
        ld d,(hl)
        ld a,(hl)                       ; bank
        ld (mapper),a

        ld b,128
.loop:  ld a,(de)
        ld (usbwr),a
        inc de
        djnz .loop

        ld a,3

acknowledge:        
        ld h,HIGH usbwr
        ld (hl),$aa
        ld (hl),$55
        ld (hl),a
        jp waitForHeader

updateBar:
        ld a,"w"  
        out ($98),a
        ret

writeCommandSequence:
        push af
        ld a,$aa
        ld ($0555),a
        cpl
        ld ($02aa),a
        pop af
        ld ($0555),a
        ret

checkCommandExecution:
        ld h,HIGH $4000
.wait:  ld a,(hl)
        ld c,(hl)
        xor c
        bit 6,a                         ; check toggle bit I (DQ6)
        ret z                           ; command complete
        bit 5,c
        jr z,.wait                      ; time limit not exceeded (DQ5)

        ld a,(hl)                       ; recheck (command might have been completed at the same time DQ5 was set)
        xor (hl)
        and %01000000
        ret z                           ; command complete

resetFlashDevice:        
        ld a,$f0                        ; write RESET command
        ld (0),a
        ret
                      
flasherEnd:
        DEPHASE
