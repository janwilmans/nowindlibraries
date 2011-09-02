; labels.asm doesn't generate any code 
; it provides names for several memory addresses and some macros

; BIOS
CALSLT          equ $001c
ENASLT          equ $0024
CHPUT           equ $00A2

IDBYTE_2D       equ $002d               ; MSX version number, 0=MSX1, 1=MSX2, 2=MSX2+, 3=MSX Turbo R
INITXT          equ $006C
SNSMAT          equ $0141
CHGMOD          equ $005f
CHGCLR          equ $0062
EXTROM          equ $015f
SDFSCR          equ $0185               ; restore screen parameters from clockchip (in SUBROM)

; variables 
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
usbWritePage1   equ $4000               ; 0x4000..0x5fff
usbReadPage0    equ $2000               ; 0x2000..0x3fff
usbWritePage2   equ $8000               ; 0x8000..0x9fff (read and write)
usbReadPage2    equ $8000               ; 0x8000..0x9fff (read and write)
mapper          equ $6001               ; 0x6001..0x7fff (odd numbers only)

; Host commands
C_DSKIO         equ $80
C_DSKCHG        equ $81
C_GETDPB        equ $82
C_CHOICE        equ $83
C_DSKFMT        equ $84
C_DRIVES        equ $85
C_INIENV        equ $86
C_GETDATE       equ $87
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
C_GETDOSVERSION equ $92
C_CMDREQUEST    equ $93
C_BLOCKREAD     equ $94
C_BLOCKWRITE    equ $95
C_CPUINFO       equ $96
C_COMMAND       equ $97
C_STDOUT        equ $98

API_NOWMAP      equ 0

; BDOS commands 0x0F - 0x37 can just use their original command code in register C

; PATCH       
        macro PATCH address, word
currentFilePosition := $
        code ! address
        dw word
        code @ currentFilePosition
        endmacro

; MAKEDPB macro
        macro MAKEDPB media, sectorsPerCluster, maxEnt, maxSector, fatSiz, fatCount

        define sectorSize 512
        define dirMask ((sectorSize/32)-1)
        define dirShift 4                               ; number of 1-bits in dirMask (sectorsize is always 512 in MSX)
        define firstFat 1
        define firstDir (firstFat + (fatCount*fatSiz))
        define firstRec (firstDir + (maxEnt/(sectorSize/32)))
        
        db media                                        ; media descriptor
        dw sectorSize                                   ; sector size
        db (sectorSize/32)-1, dirShift                  ; dirmsk
        db sectorsPerCluster-1                          ; clusmsk 
        db sectorsPerCluster                            ; clusshft (TODO: only correct for 1 and 2 sec/clus)         
        dw firstFat
        db fatCount, maxEnt
        dw firstRec
        dw ((maxSector-firstRec)/sectorsPerCluster)+1   ; maxclus
        db fatSiz
        dw firstDir
        endmacro

; BANKSWITCHING macro
        macro BANKSWITCHING bankNumber

        ds $8000-(bankswitchEnd - bankInit)-$, $ff

bankInit := $
        ld hl,nowindInit
        push hl
        xor a
        jr switchBank        

copyFromBank := $ 
        ldir
        ret
        
callInBank := $
        push hl
        ld h,bankNumber                 ; store current bank
        ex (sp),hl
        ld (mapper),a                   ; enable new bank
        call jumpIX
        pop af
switchBank := $
        ld (mapper),a                   ; restore bank
        ret

jumpIX := $
        jp (ix)

bankswitchEnd := $


        endmacro                        


; ROMDISK macro
        macro INCLUDE_ROMDISK_360KB dskimage
        
bankNumber := 8
offset := 0
       
; 22 banks with sector data
        repeat 22
        
        MSXROMHEADER        
        ds $4100 - $, $ff
        incbin dskimage, offset+512, 16384-512
        ds $8000-(bankswitchEnd - bankInit)-$, $ff
        BANKSWITCHING bankNumber
bankNumber := bankNumber + 1
offset := offset + 16384        
        
        endrepeat

; 1 bank with remaining 8192 bytes       
        MSXROMHEADER
        ds $4100 - $, $ff
        incbin dskimage, offset, 8192
        ds $8000-(bankswitchEnd - bankInit)-$, $ff
        BANKSWITCHING bankNumber        
bankNumber := bankNumber + 1

; 1 bank with sectors 00, 32, 64, ...
        MSXROMHEADER        
offset := 0
        ds $4100 - $, $ff
        repeat 23
        incbin dskimage, offset, 512
offset := offset + 16384
        endrepeat
        ds $8000-(bankswitchEnd - bankInit)-$, $ff
        BANKSWITCHING bankNumber        
        
        endmacro
        
; MSXROMHEADER
        macro MSXROMHEADER
        org $4000
        db "AB"
        dw bankInit
        ds 12,0
        endmacro
