        
vramDump:
        di
        call enableNowindPage0

        ld a,2
        out ($99),a
        ld a,$80+15             ; status register pointer at S#2
        out ($99),a

evenFrame:
        ; vram address 0x0000
        
        ;xor a
        ;out ($99),a
        ;ld a,$80+2
        ;out ($99),a
        
        ld a,%00000011                 ; pattern generator table
        out ($99),a
        ld a,$80+4
        out ($99),a

        xor a                   ; color table high
        out ($99),a
        ld a,$80+10
        out ($99),a
        
        ld a,%00000100          ; vram access at 0x10000
        call setVramAccessPointer
        call tranferframe
        call waitForRetrace
        call changeColors

oddFrame:
        ; vram address 0x10000

        ;ld a,%01000000
        ;out ($99),a
        ;ld a,$80+2
        ;out ($99),a

        ld a,%00100011          ; pattern generator table
        out ($99),a
        ld a,$80+4
        out ($99),a

        ld a,00000100           ; color table high
        out ($99),a
        ld a,$80+10
        out ($99),a

        xor a
        call setVramAccessPointer
        call tranferframe
        call waitForRetrace
        call changeColors
        jp evenFrame

tranferframe:
        call sendRegisters
        ld (hl),255
                
        ld hl,usbReadPage0
        call getHeader
        
        ld d,112                        ; 224x64 is 1 screen 2 page of data ($3800)
write_more:        
        ld hl,usbReadPage0
        ld bc,$0098
        repeat 128
        outi
        endrepeat

        dec d
        ld a,d
        or a
        jp nz,write_more
        ret             

changeColors:
        xor a                   ; set color register pointer to zero
        out ($99),a
        ld a,$80+16
        out ($99),a
        
        ld hl,usbReadPage0
        ld bc,$009A             ; write to color register

        repeat 32
        outi
        endrepeat               
        ret

        
setVramAccessPointer:
        out ($99),a
        ld a,$80+14
        out ($99),a
        xor a
        out ($99),a
        ld a,%01000000          ; vram write
        out ($99),a
        ret

waitForRetrace:
        in a,($99)
        bit 6,a
        jr nz,waitForRetrace       
.lp2:        
        in a,($99)
        bit 6,a
        jr z,.lp2       
        ret
                
        ; just let it go... 
