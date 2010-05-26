; These routines are used for slot selection

; function: enabled the selected slot of page 1 in page 0
; in: none
; out: ixh = previous slot selection for page 0
; unchanged: d, iy
; requirements: stack available
enableNowindPage0:
        call getSlotPage0
        ld ixh,a
        call getSlotPage1
        jp enableSlotPage0       

restorePage0:
        push af
        ld a,ixh
        call enableSlotPage0
;        ei
        pop af
        ret

enableSlotPage0:
        ; HL and D remain unchanged
        ; enableSlotPage0 can be called in page 1 and 2 (no absolute jumps)

        ;DEBUGMESSAGE "enasltP0"
        ld e,a                          ; store slotcode for further use
        and 3
        ld c,a                          ; new primary slot in c  
        bit 7,e
        di
        jr nz,.expanded

        in a,($a8)
        and %11111100
        or c                            ; enable new primary slot in page 0
        out ($a8),a
        ret
        
.expanded:
        ld a,e                          ; store secondary slot in e
        and %00001100     
        rrca
        rrca
        ld e,a

        in a,($a8)
        and %11111100
        or c                            ; new primary slot in page 0
        ld b,a                          ; used to restore
        rla
        and a                           ; (reset carry)
        rla
        or c                            ; new primary slot in page 3
        rrca
        rrca
        out ($a8),a

        ld a,(-1)                       ; write secondary slot register
        cpl
        and %11111100
        or e                            ; apply new secondary slot for page 0
        ld (-1),a
        ld e,a                          ; store secondary slot register (for SLTTBL)

        ld a,b                          ; restore primary slot page 3
        out ($a8),a

        ld a,LOW SLTTBL                 ; update SLTTBL
        add c                           ; add primary slot
        ld c,a
        ld b,HIGH SLTTBL
        ld a,e                          ; restore secondary slot register
        ld (bc),a
        ret

; These routines determine the current slot and subslot of a page.

getSlotPage0:
        ;DEBUGMESSAGE "getsltP0"
        in a,($a8)
        call expanded
        rlca
        rlca
        and %00001100           ; keep subSlot
        or c                    ; add mainSlot and expanded bit
        ret
                
getSlotPage1:
;        DEBUGMESSAGE "getsltP1"
        in a,($a8)
        rrca
        rrca
        call expanded
        and %00001100           ; keep subSlot
        or c                    ; add mainSlot and expanded bit
        ret        
                
getSlotPage2:
;        DEBUGMESSAGE "getsltP2"
        in a,($a8)
        rrca
        rrca
        rrca
        rrca
        call expanded
        rrca
        rrca
        and %00001100           ; keep subSlot
        or c                    ; add mainSlot and expanded bit
        ret

expanded:        
        ld hl,EXPTBL - $0300
        ld b,3
        and b
        ld c,a
        add hl,bc
        bit 7,(hl)
        jr z,notExpanded

        set 7,c
        inc l
        inc l
        inc l
        inc l
        ld a,(hl)
        ret        

notExpanded:
        pop bc
        ret

getEntrySLTWRK:
        call getSlotPage1
        ld hl,SLTWRK
        ld c,a
        rrca
        rrca
        rrca
        and %01100000           ; main slot x 32
        ld b,a
        ld a,c
        rlca
        and %00011000           ; sub slot x 8
        add b
        add l
        ld l,a
        ret

