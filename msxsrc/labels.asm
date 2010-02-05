; BIOS routines
RDSLT           equ $000c
WRSLT           equ $0014
CALSLT          equ $001c
ENASLT          equ $0024
CHGMOD          equ $005f
SNSMAT          equ $0141
EXTROM          equ $015f
SDFSCR          equ $0185               ; restore screen parameters from clockchip (in SUBROM)

; BIOS variables (really?)
LASTDRV         equ $f33f               ; stores CTRL-key status during boot
RAMAD1          equ $f342
XFER            equ $f36e

VALTYP          equ $f663
DAC             equ $f7f6
PTRFIL          equ $f864
FILNAM          equ $f866

EXPTBL          equ $fcc1
SLTTBL          equ $fcc5
SLTATR          equ $fcc9
SLTWRK          equ $fd09
PROCNM          equ $fd89
DEVICE          equ $fd99
EXTBIO          equ $ffca

; Diskrom routines
GETSLT          equ $402d
         
; Hooks
H.RUNC          equ $fecb               ; Intercepted by diskrom

; Nowind hardware addresses
usbwr           equ $4000               ; 0x4000..0x5fff
usbrd           equ $2000               ; 0x2000..0x3fff
usb2            equ $8000               ; 0x8000..0x9fff (read and write)
mapper          equ $6001               ; 0x6001..0x7fff (odd numbers only)

; Host commands
C_DSKIO         equ $80
C_DSKCHG        equ $81
C_GETDPB        equ $82
C_CHOICE        equ $83
C_DSKFMT        equ $84
C_DRIVES        equ $85
C_INIENV        equ $86
C_SETDATE       equ $87
C_DEVICEOPEN    equ $88
C_DEVICECLOSE   equ $89
C_DEVICERNDIO   equ $8a
C_DEVICEWRITE   equ $8b
C_DEVICEREAD    equ $8c
C_DEVICEEOF     equ $8d
C_AUXIN         equ $8e
C_AUXOUT        equ $8f
C_MESSAGE       equ $90
C_CHANGEIMAGE   equ $91
C_BOOTARGS      equ $92
C_CMDREQUEST    equ $93

; BDOS commands 0x0F- 0x37 can just use their original command code in register C

; PATCH       
        macro PATCH address, word
        code ! address
        dw word
        endmacro

; DEBUGMESSAGE
        macro DEBUGMESSAGE string
        ifdef DEBUG
        ld d,d
        jr .skip
        db string
.skip:  
        endif
        endmacro

; MESSAGE
        macro MESSAGE string
        call sendMessage
        db string
.skip2: nop
        endmacro

; DEBUGDUMPREGISTERS
        macro DEBUGDUMPREGISTERS
        ifdef DEBUG
        db $ed,7
        endif
        
        ifdef USBDEBUG
        assert ($ < $8000)
        call sendCpuInfo
        endif
        endmacro

; MAKEDPB macro
        macro MAKEDPB media, sectorSize, sectorsPerCluster, maxEnt, maxSector, fatSiz, fatCount
.firfat equ 1
.firdir equ .firfat+(fatCount*fatSiz)
.firrec equ .firdir+(maxEnt/(sectorSize/32))
        if sectorSize = 512
.shft   equ 4
        elseif sectorSize = 256
.shft   equ 3
        endif
        
        db media                                        ; media descriptor
        dw sectorSize                                   ; sector size
        db (sectorSize/32)-1, .shft                     ; dirmsk
        db sectorsPerCluster-1                          ; clusmsk 
        db sectorsPerCluster                            ; clusshft (TODO: only correct for 1 and 2 sec/clus)         
        dw .firfat
        db fatCount, maxEnt
        dw .firrec
        dw (maxSector-.firrec)/sectorsPerCluster+1      ; maxclus
        db fatSiz
        dw .firdir
        endmacro


; ROMHEADER macro
        macro romheader r   
.addr := $4000        
        repeat r
        code ! .addr

        org $4000
        db "AB"
        dw .init
        ds 12,0

        call .redir                     ; DSKIO
        call .redir                     ; DSKCHG
        call .redir                     ; GETDPB
        call .redir                     ; CHOICE
        call .redir                     ; DSKFMT
        ds 3,0                          ; DRVOFF

        code ! .addr + $3fe7
        org $7fe7
        
.init:  ld hl,romInit
        push hl
        jr .enableBank0        

.redir: ex (sp),hl
        dec hl
        dec hl
        dec hl
        ex (sp),hl
        jr .enableBank0

        ld (mapper),a                   ; copyFromBank
        ldir
.enableBank0:
        push af                         ; enable bank 0 (no registers changed)
        xor a
        ld (mapper),a
        pop af
        ret
        
.@addr := .addr + $4000
        endrepeat
        endmacro

; MACRO debugdisasm
        macro DEBUGDISASM
        db $ed, $0b
        endmacro
        
; MACRO debugdisasmoff
        macro DEBUGDISASMOFF
        db $ed, $0c
        endmacro        

; MACRO breakpoint
        macro BREAKPOINT
        ld b,b
        jr $+2
        endmacro

        
        macro PRINTVDPTEXT string
        
        push hl
        ld hl,.text 
        call printVdpText2
        pop hl
        jr .skip
.text   db string
        db 0
.skip:
        endmacro
