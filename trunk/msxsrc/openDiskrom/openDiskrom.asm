; openDiskrom.asm
DRVTBL          equ $fb21 
GETWRK          equ assert

        output "openDiskrom.rom"

        defpage 0, $4000, $4000
        defpage 1..31, $4000, $4000
       
        define DEBUG
        include "../labels.asm"
                
        page 0

        db "AB"                         ; rom header
        dw init                         ; init
        dw 0                            ; call statement
        dw 0                            ; device handler
        dw 0                            ; basic text
        ds 6,0                          ; reserved

        jp DSKIO
        jp DSKCHG
        jp GETDPB
        jp CHOICE
        jp DSKFMT
        ds 3,0                          ; DRVSTP not implemented

        call assert                     ; start Disk Basic (fixed at $4022)
        scf                             ; format disk (fixed at $4025)
        call assert
        call assert                     ; DRVSTP all (fixed at $4029)
        nop
        jp getSlotPage1                 ; GETSLT (fixed at $402d)
        ld hl,($f34b)
        ret

        include "bdos.asm"
        
        include "../common.asm"
        include "../slotRoutines.asm"
        include "../flashWriter.asm"

        include "../nowindDriver.asm"

init:   DEBUGMESSAGE "init"
        call INIHRD
        di
        ld a,(DEVICE)
        or a
        ret m                           ; abort diskrom init
        jp nz,noDiskromInit
        ; initialiseer EXTBIO (check HOKVLD eerst)
        ; 16k RAM?
        
        DEBUGMESSAGE "shift?"
        ld a,6
        call SNSMAT
        di
        rrca
        jr nc,shiftPressed
        
        DEBUGMESSAGE "neeeeh"

        ; reserveer F380+MYSIZE-F1C9 (start of diskvariables)
        ; clear F1C9 - F380
        ; AUTLIN(2) = 0 (grootste gevonden sector is 0)
        ; clear DRVINF(8) (4x aantal drives + slotcode)
        call GETSLT
        
        ; clear DRVTBL(12) (slotcode + ISR)
        
        
        ; init hooks (F24F-F2B7) met $C9
        ; init F365 (in a,($a8), ret)
        
        ld a,6                          ; store status CTRL-key
        call SNSMAT
        di
        and $02
        ld (LASTDRV),a
        
        ld a,7                          ; beep
        rst $18
               
        call GETSLT                     ; continue initialise after MSX-BASIC is initialised
        ld hl,H.RUNC
        ld de,basicInterceptor
        call installHook     
        
        ; skip next part
        
        ret
        
noDiskromInit:        
        ; zoek lege entry in DRVTBL
        
        
        
        
        call DRIVES
        ld a,h
        ld (DRVTBL),a
        call getSlotPage1
        ld (DRVTBL+1),a

        ; - skip (*) en spring naar (**)
        ;
        ; - (*)installeren van drives
        ; - tel totaal aantal drives (DRVTBL, $FB21)
        ;       als bit 7 van een van de 4 entries geset is -> No enough memory (di, halt)
        ;       8 drives of meer, dan ret
        ;       allocate MYSIZE bytes
        ; (**)
        ; - sla het address van werkgebied op in SLTWRK (zoek juiste entry)
        ; - check sectorsize van driver (SECLEN variable) en pas zonodig AUTLIN aan
        ; - zoek vrije entry in DRVTBL
        ;       alle vier bezet, No enough memory (di, halt)


        
        
        ; - tel reg_l bij al gevonden drives op en check of het niet meer dan 8 is
        ;       is het aantal meer dan 8 sla dan zoveel mogelijk op (dus 8 minua aantal al geinstallerde drives)
        ;       sla aantal drives van dit diskrom op in DRVTBL
        ;       sla slotid ook op
        ;       update DPBTBL (hier staat voor elke drive een pointer naar een DPB)
        ;       alloceer ruimte voor DPB(s) en init deze

        call INIENV
                
        ld hl,DEVICE
        inc (hl)
        ret
                
basicInterceptor:
        DEBUGMESSAGE "H.RUNC"
        
        ld hl,H.RUNC
        ld b,5
.loop:  ld (hl),$c9
        inc hl
        djnz .loop
        
        ; TODO: do more init!
        ret        
        
shiftPressed:
        ld a,7
        rst $18
abortDiskInit:
        ld a,255
        ld (DEVICE),a        
        ret

installHook:
        ld (hl),$f7                     ; rst $30
        inc hl
        ld (hl),a                       ; slot id
        inc hl
        ld (hl),e                       ; address to be jumped to
        inc hl
        ld (hl),d
        ret
        
printText:
        ;skip text, no printing routine implemented yet
        ex (sp),hl
        xor a
        ld c,255
        cpir 
        ex (sp),hl
        ret

ROMDISK_DSKIO:
ROMDISK_DSKCHG:
 
assert: 
        pop ix
        dec ix
        dec ix
        dec ix                          ; IX address from where call was made
        DEBUGDUMPREGISTERS              
        db $ed,$0a                      ; nowind assertion 
 
        ds $8000-$-1,0xff               ; fill with 0xff until 0x8000
        ret
        
; add rom headers in banks 1..31
        repeat 31
       
        page 1..31
        db "AB"                         ; rom header
        dw .init                        ; init
        dw 0                            ; call statement
        dw 0                            ; device handler
        dw 0                            ; basic text
        ds 6,0                          ; reserved

        ds $4000 - 16 - 9, 255
           
.init:  ld hl,init
        push hl
        xor a
        ld (mapper),a
        ret

        endrepeat

; diskvariabelen: F1C9 t/m F37F
;       - 23 hooks $F24F-$F2B7 
;        
; Wat doet een DOS1 rom
; rom init:
; - call INIHRD
;       (DOS2 quit als MSX1)
; - check DEVICE
;       bit 7 is 1 -> quit
;       0 doe initialisatie zelf
;       niet 0, ander diskrom heeft init al gedaan, dus alleen drives installeren (*)
; 
; disk init:
; - installeer EXTBIO routine (check eerst HOKVLD, bit 0)
; - meer dan 16k aanwezig? (BOTTOM)
;       zo niet, dan DEVICE=255, quit
; - Turbo-R check of HIMEM nog steeds F380 is (DEVICE=255 en stop)
; - check SHIFT
;       shift ingedruk, dan beep en DEVICE=255, quit
; note: Turbo-R checkt nog op '1' wordt ingedrukt, anders wordt R800 RAM cpumode ingeschakeld
;
;
; - reserveer F380+MYSIZE-F1C9 (start of diskvariables)
; - clear F1C9 - F380
; - AUTLIN(2) = 0 (grootste gevonden sector is 0)
; - clear DRVINF(8) (4x aantal drives + slotcode)
; - clear DRVTBL(12) (slotcode + ISR)
; - init hooks (F24F-F2B7) met $C9
; - init F365 (in a,($a8), ret)
; - check CTRL and store in F33F (LASTDRV)
; - beep
; - copieer interslot call op H.RUNC ($FECB)
; - skip (*) en spring naar (**)
;
; - (*)installeren van drives
; - tel totaal aantal drives (DRVTBL, $FB21)
;       als bit 7 van een van de 4 entries geset is -> No enough memory (di, halt)
;       8 drives of meer, dan ret
;       allocate MYSIZE bytes
; (**)
; - sla het address van werkgebied op in SLTWRK (zoek juiste entry)
; - check sectorsize van driver (SECLEN variable) en pas zonodig AUTLIN aan
; - zoek vrije entry in DRVTBL
;       alle vier bezet, No enough memory (di, halt)
; - CALL DRIVES
; - tel reg_l bij al gevonden drives op en check of het niet meer dan 8 is
;       is het aantal meer dan 8 sla dan zoveel mogelijk op (dus 8 minua aantal al geinstallerde drives)
;       sla aantal drives van dit diskrom op in DRVTBL
;       sla slotid ook op
;       update DPBTBL (hier staat voor elke drive een pointer naar een DPB)
;       alloceer ruimte voor DPB(s) en init deze
; - CALL INIENV
; - increment DEVICE (number of diskrominterfaces)
; - ret
 
; Andere roms krijgen nu de kans, en pas als H.RUNC wordt aangeroepen gaat we hier verder:
; clear H.RUNC (met 5x $c9)
; check of DEVICE == 0 door een andere diskrom (zo ja, ret)
; clear DEVICE
; iets met H.LOPD en H.CLEA
; F348 (EXBRSA?) diskrom slotid
; init diskrom variable (F1C9-F237) source $7397 tot aan begin driver
;       - BDOS09, XFER, warm boot, start handle in DOS mem, validate FCB
;       - een jump, devicenames, fake directory?, aantal dagen per maand 
; copy BIOS $34-$37 naar F30F
; iets datum
; iets met databuffer F241, F246
; nog iets met FCB's
; initialize F368 (alleen 8x een $c3, jump dus)
;       routines alleen bruikbaar onder DOS? (DOSON, DOSOFF, XFER... BDOS)
; allocate sectorbuffer (AUTLIN) en store address in SECBUF
; allocate datasectorbuffer (store in F34F)
; allocate dirsectorbuffer (store in F351) (o.a. voor DSKI?)
; check DRVTBL en update F347 (number of drives)
; initialise DPB???
; initialise AUX IN en AUX OUT hooks? (ld a,eof/ret)
; init jump table voor AUX device
; call 7D2F in mainrom (does only the 8250 diskrom do this?) (Restore screen parameters from RTC)
; vaaaaaaaaaaag
; initialise BASIC screenmode
; check en init clockchip
; init diskbasic hooks
; zoekram? en zet RAMAD0..3
; zoek voor BASIC TEXT extentions????
; laad bootsector
; start boot code met carry flag reset
; 




