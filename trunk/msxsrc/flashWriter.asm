; flashWriter.asm
; Flashes and erases the AMD29F040/M29F032-D
            
; this label is before the _PHASE_ so it's address refers to the actual rom location
; not the 'copied' $c000 location            
waitForFlashCommand:

        PHASE $c000  

flasherStart:     

        di
        
        in a,($aa)
        and $f0
        out ($aa),a     ; select keyboard row 0 
        
.loop
        in a,($a9)
        bit 1,a
        ld b,1
        jr z,.switchSlot            ; '1' pressed?
        bit 2,a
        ld b,2
        jr z,.switchSlot            ; '2' pressed?
        
        ld h,HIGH usbReadPage0
        ld a,(hl)                   ; header $BB ?
.chkbb: cp $bb
        jr nz,.loop
        ld a,(hl)
        cp $55
        call z,.cmd
        jr .loop  

.switchSlot:
        call enableFlashMainslot
        jr .loop  

.cmd:
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
        jr z,autoselectMode     

        ; unknown command, just go back
        jr .loop
     
; in: b = main slot to enable for page 0 and 1   
enableFlashMainslot: 

        
        ld a,(currectSlot)
        cp b
        ret z               ; do nothing if already selected
        ld a,b
        ld (currectSlot),a
        
        push bc
        and 3
        ld b,a
        rlca
        rlca
        or b
        ld c,a

  		in a,($a8)
		and $f0
		or c
		out ($a8),a         ; switch mainslot of page 0 and 1 
		pop bc
		
		ld a,48
		add b
		out ($98),a         ; print 1/2	
		ret        
    
; 'autoselect' is a flashrom feature and has nothing to do with 'selectSlot'
; the 'autoselect' feature is used to identify the type of flashrom
autoselectMode:
        ld a,$90
        call writeCommandSequence

        ld hl,($4000)
        ex de,hl
        ld h,HIGH usbWritePage1
        ld (hl),$aa
        ld (hl),$55
        ld (hl),e                       ; manufacturer ID (0x01 = AMD)
        ld (hl),d                       ; device ID (0xA4 = AM29F040, 0x41 = AM29F032B)

        call resetFlashDevice
        ret

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
        ld a,(usbReadPage0)
        ld (de),a                       ; write data to flash
        inc de
        call checkCommandExecution
        djnz .loop
        
        ld a,2
        jr acknowledge        
        
verifyFlash:
        ;DEBUGMESSAGE "verify"
        ld hl,usbReadPage0
        ld e,(hl)                       ; address
        ld d,(hl)
        ld a,(hl)                       ; bank
        ld (mapper),a

        ld b,128
.loop:  ld a,(de)
        ld (usbWritePage1),a
        inc de
        djnz .loop

        ld a,3

acknowledge:        
        ld h,HIGH usbWritePage1
        ld (hl),$aa
        ld (hl),$55
        ld (hl),a
        ret

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
           
currectSlot:
        db $ff
                      
flasherEnd:
        DEPHASE

