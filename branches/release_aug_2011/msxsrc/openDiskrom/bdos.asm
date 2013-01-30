; bdos.asm

; CONST??? (used by MSXDOS.SYS???)
        ds $4034 - $, $c9
        jp assert
        
; CONIN
        ds $4078 - $, $c9
        jp assert
        
; CONOUT
        ds $408f - $, $c9
        jp assert

; BDOS $0c (get version number)
        ds $41ef - $, $c9
bdosFunction_0ch:        
        jp assert

; BDOS $13 (delete)
        ds $436c - $, $c9
bdosFunction_13h:
        jp assert

; BDOS $17 (rename)  
        ds $4392 - $, $c9
bdosFunction_17h:
        jp assert

; BDOS $0f (open)
        ds $4462 - $, $c9
bdosFunction_0fh:
        jp assert

; BDOS $10 (close)
        ds $456f - $, $c9
bdosFunction_10h:
        jp assert
          
; BDOS $16 (create)
        ds $461d - $, $c9
bdosFunction_16h:
        jp assert

; BDOS $2f (read sector)
        ds $46ba - $, $c9
bdosFunction_2fh:
        jp assert
        
; BDOS $30 (write sector)
        ds $4720 - $, $c9
bdosFunction_30h:        
        jp assert
        
; BDOS $14 (sequential read)
        ds $4775 - $, $c9
bdosFunction_14h:        
        jp assert
        
; BDOS $15 (sequential write)
        ds $477d - $, $c9
bdosFunction_15h:        
        jp assert
        
; BDOS $21 (random read)
        ds $4788 - $, $c9
bdosFunction_21h:
        jp assert
        
; BDOS $22 (random write)
        ds $4793 - $, $c9
bdosFunction_22h:
        jp assert
        
; BDOS $27 (random block read)
        ds $47b2 - $, $c9
bdosFunction_27h:        
        jp assert
        
; BDOS $26 (random block write)
        ds $47be - $, $c9
bdosFunction_26h:
        jp assert
        
; BDOS $28 (random write with zero fill)
        ds $47d1 - $, $c9
bdosFunction_28h:
        jp assert
        

; free space of 2021 bytes!


; BDOS $11 (find first)
        ds $4fb8 - $, $c9
bdosFunction_11h:
        jp assert
                
; BDOS $12 (find next)
        ds $5006 - $, $c9
bdosFunction_12h:
        jp assert
        
; BDOS $23 (get file size)
        ds $501e - $, $c9
bdosFunction_23h:
        jp assert
        
; BDOS $18 (get login vector)
        ds $504e - $, $c9
bdosFunction_18h:
        jp assert
        
; BDOS $1a (set disk transfer address)
        ds $5058 - $, $c9
bdosFunction_1ah:
        jp assert
        
; BDOS $1b (get allocation information)
        ds $505d - $, $c9
bdosFunction_1bh:
        jp assert
        
; BDOS $0d (disk reset)
        ds $509f - $, $c9
bdosFunction_0dh:
        jp assert

; flush buffers (used by MSXDOS.SYS)
        ds $50a9 - $, $c9
        jp assert
              
; BDOS $19 (get current drive)
        ds $50c4 - $, $c9
bdosFunction_19h:
        jp assert
        
; BDOS $24 (set random record)
        ds $50c8 - $, $c9
bdosFunction_24h:
        jp assert
        
; BDOS $0e (select disk)
        ds $50d5 - $, $c9
bdosFunction_0eh:
        jp assert
        
; BDOS $0a (buffered line input)
        ds $50e0 - $, $c9
bdosFunction_0ah:
        jp assert
        
; print newline (used in MSXDOS.SYS???)
        ds $5183 - $, $c9
        jp assert
        

; free space of 471 bytes!


; print abort string (using in MSXDOS.SYS???)
        ds $535d - $, $c9
        jp assert
        
; BDOS $02 (console output)
        ds $53a7 - $, $c9
bdosFunction_02h:
        jp assert
        
; BDOS $0b (console status)
        ds $543c - $, $c9
bdosFunction_0bh:
        jp assert
        
; BDOS $01 (console input)
        ds $5445 - $, $c9
bdosFunction_01h:
        jp assert
        
; BDOS $08 (console input without echo)
        ds $544e - $, $c9
bdosFunction_08h:
        jp assert

; BDOS $06 (direct console I/O)
        ds $5454 - $, $c9
bdosFunction_06h:
        jp assert

; BDOS $07 (direct console input)
        ds $5462 - $, $c9
bdosFunction_07h:
        jp assert

; BDOS $05 (printer output)
        ds $5465 - $, $c9
bdosFunction_05h:
        jp assert

; BDOS $03 (aux input)
        ds $546e - $, $c9
bdosFunction_03h:
        jp assert

; BDOS $04 (aux output)
        ds $5474 - $, $c9
bdosFunction_04h:
        jp assert

; BDOS $2a (get date)
        ds $553c - $, $c9
bdosFunction_2ah:
        jp assert

; BDOS $2b (set date)
        ds $5552 - $, $c9
bdosFunction_2bh:
        jp assert

; BDOS $2c (get time)
        ds $55db - $, $c9
bdosFunction_2ch:
        jp assert

; BDOS $2d (set time)
        ds $55e6 - $, $c9
bdosFunction_2dh:
        jp assert

; BDOS $2e (set/reset verify flag)
        ds $55ff - $, $c9
bdosFunction_2eh:
        jp assert

; these routines can be place in unused areas 

bdosFunction_1ch:
bdosFunction_1dh:
bdosFunction_1eh:
bdosFunction_1fh:
bdosFunction_20h:
bdosFunction_25h:
bdosFunction_29h:
        xor a
        ld b,a
        ret

bdosFunction_00h:
        jp assert
        
bdosFunction_09h:
        jp assert       ; (jumps to $f1c9, change in bdosfunctionTable)

bdosFunctionTable:
        dw bdosFunction_00h, bdosFunction_01h, bdosFunction_02h, bdosFunction_03h
        dw bdosFunction_04h, bdosFunction_05h, bdosFunction_06h, bdosFunction_07h
        dw bdosFunction_08h, bdosFunction_09h, bdosFunction_0ah, bdosFunction_0bh
        dw bdosFunction_0ch, bdosFunction_0dh, bdosFunction_0eh, bdosFunction_0fh
        dw bdosFunction_10h, bdosFunction_11h, bdosFunction_12h, bdosFunction_13h
        dw bdosFunction_14h, bdosFunction_15h, bdosFunction_16h, bdosFunction_17h
        dw bdosFunction_18h, bdosFunction_19h, bdosFunction_1ah, bdosFunction_1bh
        dw bdosFunction_1ch, bdosFunction_1dh, bdosFunction_1eh, bdosFunction_1fh
        dw bdosFunction_20h, bdosFunction_21h, bdosFunction_22h, bdosFunction_23h
        dw bdosFunction_24h, bdosFunction_25h, bdosFunction_26h, bdosFunction_27h
        dw bdosFunction_28h, bdosFunction_29h, bdosFunction_2ah, bdosFunction_2bh
        dw bdosFunction_2ch, bdosFunction_2dh, bdosFunction_2eh, bdosFunction_2fh
        dw bdosFunction_30h



; note BDOS $09 jumps to $F1C9

; TODO: welke nog meer standaard!
