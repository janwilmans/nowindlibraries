
BDOS_DTA                        equ $f23d

BDOS_OPENFILE                   equ $0f
BDOS_CLOSEFILE                  equ $10
BDOS_FINDFIRST                  equ $11
BDOS_FINDNEXT                   equ $12
BDOS_DELETEFILE                 equ $13
;BDOS_SEQUENTALREAD             equ $14
;BDOS_SEQUENTALWRITE            equ $15
BDOS_CREATEFILE                 equ $16
BDOS_RENAMEFILE                 equ $17

;BDOS_RANDOMREAD                equ $21
;BDOS_RANDOMWRITE               equ $22

BDOS_RANDOMBLOCKWRITE           equ $26
BDOS_RANDOMBLOCKREAD            equ $27
;BDOS_RANDOMWRITEWITHZEROFILL   equ $28

BDOS_ABSOLUTESECTORREAD         equ $2f
BDOS_ABSOLUTESECTORWRITE        equ $30


; patching the BDOS hook ($f37d) or its jumptable will not work as some routines are called directly internally by the BDOS itself.
; For example, H.BINL (load binary) assumes these routines are at fixed places in the diskrom. Also some programs may assume the 
; addresses are fixed and call them directly.
        
        PATCH $5d20, nowindBDOS              ; overwrite the standard BDOS hook "DW $56D3" with BDOSNW (for logging only)     

currentFilePosition2 := $        

        ;code ! $408F
        ;jp bdosInternalOutputToScreen    ; internal bdos subroutine that calls the CHPUT BIOS call
        
        ;code ! $40AB                    ; MSX BIOS call logging for call made by BDOS
        ;jp bdosInternalCallBios
        
;        code ! $5445
;        jp bdosConsoleInput             ; 0x01, CPM MSX1

;         code ! $53a7                   
;         ld a,c                          ; needed because C_OUT is called internally
;C_OUT:   jp bdosConsoleOutput            ; 0x02, CPM MSX1, no need to patch this, because it uses 'bdosInternalOutputToScreen' which is already patched

;        code ! $546e
;        jp bdosAuxInput                 ; 0x03, CPM MSX1

;        code ! $5474
;        jp bdosAuxOutput                ; 0x04, CPM MSX1

;        code ! $5465
;        jp bdosPrinterOutput            ; 0x05, CPM MSX1

;        code ! $5454
;        jp bdosDirectConsoleIO          ; 0x06, CPM MSX1

;        code ! $5462
;        jp bdosDirectConsoleInput       ; 0x07, MSX1

;        code ! $544e
;        jp bdosConsoleInputWithoutEcho  ; 0x08, MSX1

; TODO: check bdos 09h          ; 0x09, CPM MSX1, calls bdos 02h to output chars

;        code ! $50e0
;        jp bdosBufferedLineInput        ; 0x0a, CPM MSX1

;        code ! $543c
;        jp bdosConsoleStatus            ; 0x0b, CPM MSX1

;        code ! $41ef
;        jp bdosReturnVersionNumber      ; 0x0c, CPM MSX1

        code ! $509f
        jp bdosDiskReset                ; 0x0d, CPM MSX1

;        code ! $50d5
;        jp bdosSelectDisk               ; 0x0e, CPM MSX1

        code ! $4462
        jp bdosOpenFile                 ; 0x0f, CPM MSX1

        code ! $456f
        jp bdosCloseFile                ; 0x10, CPM MSX1

        code ! $4fb8
        jp bdosFindFirst                ; 0x11, CPM MSX1
        
        code ! $5006
        jp bdosFindNext                 ; 0x12, CPM MSX1

        code ! $436c
        jp bdosDeleteFile               ; 0x13, CPM MSX1

        code ! $4775
        jp bdosSequentialRead           ; 0x14, CPM MSX1
        
        code ! $477d
        jp bdosSequentialWrite          ; 0x15, CPM MSX1
        
        code ! $461d
        jp bdosCreateFile               ; 0x16, CPM MSX1
        
        code ! $4392
        jp bdosRenameFile               ; 0x17, CPM MSX1

;        code ! $504e
;        jp bdosGetLoginVector           ; 0x18, CPM MSX1

;        code ! $50c4
;        jp bdosGetCurrentDrive          ; 0x19, CPM MSX1

;        code ! $5058
;        jp bdosSetDTA                   ; 0x1a, CPM MSX1

;        code ! $505d
;        jp bdosGetAllocationInfo        ; 0x1b, MSX1

        code ! $4788
        jp bdosRandomRead               ; 0x21, CPM MSX1

        code ! $4793
        jp bdosRandomWrite              ; 0x22, CPM MSX1

        code ! $501E
        jp bdosGetFileSize              ; 0x23, CPM MSX1

        code ! $50c8
        jp bdosSetRandomRecord          ; 0x24, CPM MSX1
        
        code ! $47be
        jp bdosRandomBlockWrite         ; 0x26, MSX1

        code ! $47b2
        call bdosRandomBlockRead        ; 0x27, MSX1
        ret

        code ! $47d1
        jp bdosRandomBlockWriteZeroFill ; 0x28, CPM MSX1

;        code ! $553c
;        jp bdosGetDate                  ; 0x2a, MSX1   ; todo: implement, on msx1 the date is stored in RAM? 
; Leest de datum en zet deze in de Z80-registers. A : dag van de week, D : dag, E : maand, HL: jaar

;        code ! $5552
;        jp bdosSetDate                  ; 0x2b, MSX1

;        code ! $55db
;        jp bdosGetTime                  ; 0x2c, MSX1  ; todo: find out what this does on clockchipless MSX
; Leest de tijd en zet deze in de Z80-registers. E : honderdsten van seconden, D : seconden, L : minuten, H : Uren

;        code ! $55e6
;        jp bdosSetTime                  ; 0x2d, MSX1

;        code ! $55ff
;        jp bdosSetResetVerifyFlag       ; 0x2e, MSX1

        code ! $46ba
        jp bdosAbsoluteSectorRead       ; 0x2f, MSX1

        code ! $4720
        jp bdosAbsoluteSectorWrite      ; 0x30, MSX1

        code @ currentFilePosition2

nowindBDOS:
        DEBUGMESSAGE "nwBDOS"
        DEBUGDUMPREGISTERS
        jp $56d3                        ; normal BDOS hook address

bdosInternalCallBios:

        DEBUGMESSAGE "bdosInternalCallBios"

        ; code from original bdosInternalCallBios implementation
        push    iy
        ld      iy,(EXPTBL-1+0)
        call    CALSLT
        ei
        pop     iy
        ret

bdosInternalOutputToScreen:

        ;DEBUGMESSAGE "bdosInternalOutputToScreen"
        ;DEBUGDUMPREGISTERS
        push af
        push de
        push hl
        call sendRegisters
        ld (hl),C_STDOUT
        pop hl
        pop de
        pop af

        ; code from original bdosInternalOutputToScreen implementation
        push    ix
        ld      ix,CHPUT
        call    $40AB                   ; CHPUT BIOS call
        pop     ix
        ret

bdosConsoleOutput:

        push af
        push bc
        push de
        push hl
        DEBUGMESSAGE "Console output"    

        pop hl
        pop de
        pop bc
        pop af
        
        call $F2AF  ; code from the original bdosConsoleOutput that was overwritten by the bdos patch
        jp $53AB

bdosOpenFile:
        DEBUGMESSAGE "bdosOpenFile"
        ld a,1                          ; since bdosOpenFile is a CP/M compatible BDOS call, we must set this.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   

        push de
        call sendRegisters
        ld (hl),BDOS_OPENFILE
        pop de

        ld a,d                          ; used by blockRead (TODO: moet anders!)
        ex de,hl                        ; send FCB to host
        ld bc,37
        ldir
        
        call blockRead
        jr c,.error			; connection lost
        cp 128
        jr z,.error
        
        ld a,0
        ;DEBUGMESSAGE "bdosOpen ok"
        jp restorePage0
        
.error:        
		ld a,255
		DEBUGMESSAGE "bdosOpen error"
        jp restorePage0

;        call enableNowindPage0
;        call getHeaderInPage0
;        jr nc,.exit                     ; carry means is connection lost
;        ld a,255                        ; return 'file not found'
;.exit:
;        ; TODO: update FCB?
;        call restorePage0        
;        ret
       
bdosCloseFile:
        DEBUGMESSAGE "bdosCloseFile"
        ld a,1                          ; since bdosCloseFile is a CP/M compatible BDOS call, we must set this.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
        
        call sendRegisters
        ld (hl),BDOS_CLOSEFILE

        xor a
        ret

;todo: see  GET PREVIOUS ERROR CODE (65H)
bdosFindFirst:
        DEBUGMESSAGE "bdosFindFirst"
        ld a,1                          ; since bdosFindFirst is a CP/M compatible BDOS call, we must set this.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
        
        push de
        ld hl,(BDOS_DTA)
        call sendRegisters
        ld (hl),BDOS_FINDFIRST
        pop de
        
        push hl
        ex de,hl                        ; send FCB to host
        ld bc,37
        ldir
        pop hl

.findResult:
		ld a,h
        ; TODO: a laden met high transfer address?
        call blockRead
        jr c,.error

        and a
        jp m,.error             ; 128 means 'file not found'

        xor a
        ret z                   ; file was found! (drive number and filename copied to DTA)

.error: 
        ld a,$ff
        ret

bdosFindNext:
        //DEBUGMESSAGE "bdosFindNext"       
        ld a,1                          ; since bdosFindNext is a CP/M compatible BDOS call, we must set this.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
               
        ld hl,(BDOS_DTA)
        call sendRegisters
        ld (hl),BDOS_FINDNEXT
        jp bdosFindFirst.findResult

bdosDeleteFile:
        DEBUGMESSAGE "bdosDeleteFile"
        ld a,1                          ; since bdosDeleteFile is a CP/M compatible BDOS call, we must set this.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
        
        call sendRegisters
        ld (hl),BDOS_DELETEFILE
        ld a,255        ; delete unsuccessful
        ret

; function: BDOS 0x16 (_FMAKE), CREATE FILE [FCB] 
; in: de = pointer to unopened FCB
;
; out: a = 0xff if error 
;      a = 0 if successful
        
bdosCreateFile:
        DEBUGMESSAGE "bdosCreateFile"
        ;ld a,1                          ; since bdosCreateFile is a CP/M compatible BDOS call, we must set this.
        ;ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
        
        ;push de
        ;ld hl,(BDOS_DTA)
        ;call sendRegisters
        ;ld (hl),BDOS_CREATEFILE
        ;pop de

        ;ex de,hl                        ; send FCB to host
        ;ld bc,37
        ;ldir
        
        ;ld a,(BDOS_DTA + 1)
        ;call blockRead
        ;jr c,.error
        
        ;call receiveRegisters           ; get bdosCreateFile results   
        ;jr c,.error
        
        ;DEBUGMESSAGE "receiveRegisters done!"
        
.error:        
        ld a,255        ; create unsuccessful
        ret

; function: BDOS 0x17 (_FREN), RENAME FILE [FCB] 
; in: de = pointer to unopened FCB
;
; out: a = 0xff if error 
;      a = 0 if successful

bdosRenameFile:
        DEBUGMESSAGE "bdosRenameFile"
        ld a,1                          ; since bdosRenameFile is a CP/M compatible BDOS call, we must set this.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
        
        call sendRegisters
        ld (hl),BDOS_RENAMEFILE
        ld a,255        ; rename unsuccessful
        ret

; function: BDOS 0x26, Random Block Write
; in: de = pointer to opened FCB
;     hl = number of records to write
;
; out: a = 0xff if error 
;      a = 0 if successful

bdosRandomBlockWrite:     
        DEBUGMESSAGE "bdosRandomBlockWrite"
        call sendRegisters
        ld (hl),BDOS_RANDOMBLOCKWRITE
        ld a,255        ; write unsuccesful        
        ret
        
; function: BDOS 0x27, Random Block Read 
; in: de = pointer to opened FCB
;     hl = number of records to read
;
; out: a = 0xff if error (usually caused by end-of-file)
;      a = 0 if successful
;     hl = number of records actually read
        
bdosRandomBlockRead:
        DEBUGMESSAGE "bdosRandomBlockRead"

        xor a                           ; since bdosRandomBlockRead is not a CP/M compatible BDOS call, we must set this to zero.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   

        push de
        ld bc,(BDOS_DTA)                ; send DTA in bc
        call sendRegisters
        ld (hl),BDOS_RANDOMBLOCKREAD
        pop de

        ex de,hl                        ; send FCB to host
        ld bc,37
        ldir

        ld a,(BDOS_DTA + 1)
        call blockRead
        jr c,.error
        
        DEBUGMESSAGE "data blockRead done!"
        
        ; todo: receive updated FCB here with second blockread
        
        call receiveRegisters       ; get bdosRandomBlockRead results   
        jr c,.error
        
        DEBUGMESSAGE "receiveRegisters done!"

        ; A = 1 if an error occured (mostly EOF), otherwise A = 0
        ; HL = records received
        ; DE and IX = address of open FCB
                    
        ; do FCB update, hl must be added to the random record field [FCB+0x21] [FCB+0x22] [FCB+0x23]
        ; see: http://msxsyssrc.cvs.sourceforge.net/viewvc/msxsyssrc/disk100upd/disk.mac?revision=1.1&view=markup line: 1875
        
        push hl
        push de
        ld e, (ix+$21)
        ld d, (ix+$22)
        add hl,de

        ld (ix+$21), l
        ld (ix+$22), h
        ld (ix+$23), 0        
        pop de
        pop hl

        DEBUGMESSAGE "FCB updated"
        DEBUGDUMPREGISTERS
        ;DEBUGMESSAGE "CMD_INSTR_ON"
        ret
        
.error: 
        DEBUGMESSAGE "BDOS 0x27 error"
        ld hl,0
        ld a,1
        ret

; function: BDOS 0x14, Sequential Read, reads the next 128 bytes.
; in: de = pointer to opened FCB
;
; out: a = 0xff if error (usually caused by end-of-file)
;      a = 0 if successful
        
bdosSequentialRead:
        DEBUGMESSAGE "bdosSequentialRead"

        push de                         ; read just 1 record of 128 bytes        
        pop ix
        ld (ix+$0e), 128
        ld (ix+$0f), 0  
        ld hl,1  
        
        call bdosRandomBlockRead        ; resets $F306
        
        ld a,1                          ; since bdosSequentialRead is a CP/M compatible BDOS call, we must set this.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
        ret        

bdosAbsoluteSectorRead:
        DEBUGMESSAGE "bdosAbsoluteSectorRead"
        xor a                           ; since bdosAbsoluteSectorRead is not a CP/M compatible BDOS call, we must set this to zero.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
        
        call sendRegisters
        ld (hl),BDOS_ABSOLUTESECTORREAD
        ld a,2  ; not ready (TODO: check!)
        ret
        
bdosAbsoluteSectorWrite:
        DEBUGMESSAGE "bdosAbsoluteSectorWrite"
        xor a                           ; since bdosAbsoluteSectorWrite is not a CP/M compatible BDOS call, we must set this to zero.
        ld ($F306),a                    ; for CP/M ld l,a and ld h,b is done after return of the BDOS function.   
        
        ld a,2  ; not ready (TODO: check!)
        ret

bdosDiskReset:
bdosSequentialWrite:
bdosRandomRead:
bdosRandomWrite:
bdosGetFileSize:
bdosSetRandomRecord:
bdosRandomBlockWriteZeroFill:
        DEBUGMESSAGE "ASSERTTTT"
        DEBUGDUMPREGISTERS
        di
        halt

    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0
    ;db 0,0,0,0,0,0,0,0    
    ;        ds 128
        
; http://www.konamiman.com/msx/msx-e.html#msx2th
; http://www.angelfire.com/art2/unicorndreams/msx/RR-BASIC.html
; http://msxsyssrc.cvs.sourceforge.net/msxsyssrc/diskdrvs/

;01H Get Character from Console
;02H send one character to console
;03H get one character from auxiliary device
;04H send one character to auxiliary device
;05H send one character to printer
;06H get one character from console (no input wait, no echo back, no control code 07H get one character from console (input wait, no echo back, no control                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               code 08H get one character from console (input wait, no echo back, control code check)
;09H send string
;0AH get string
;0BH check input from console
;0CH get version number
;0DH disk reset                                                                                 (impl: flush write buffers, if any, but also call the original? (reset dma address to 0x0080, A: default en schrijf alle FAT's naar alle disks?)
;0EH select default drive                                               ; writes to DEFDRV ($F247)
;0FH Open File                                                                                  (impl: open file handle, first byte of FCB=drive, 0=default, 1=A:, 2=B:, DE=addres of FCB, return: A=0 when file was opened successfully, otherwise 0xff), FCB is filled
;10H Close File                                                                                 (impl: close file handle, DE=addres of FCB, return: A=0 when file was closed successfully, otherwise 0xff), note that if the file was only read, the user is not required to call "close"
;11H Find First                                                                                 (impl: DE=unopend FCB, return: A=0 when the file is found, otherwise 0xff, write the file's directory entry (32 bytes) to DMA + the FCB drive number (1 byte)
;12H Find Next                                                                                  (impl: no input, A=0 when a next file is found, otherwise 0xff
;13H delete file                                                                                (impl: delete file on host, make read-only host-option, DE=unopened FCB, return: A=00 when succuessfull, otherwise 0xff, wildcards allowed!)
;14H read sequential file                                               (impl: transfer to dma address, transfer 128 bytes to DMA with auto-increment to FCB records, DE=unopened FCB
;15H write sequential file                                      (impl: transfer from dma address, transfer 128 bytes from DMA with auto-increment to FCB records, DE=opened FCB
;16H create file                                                                                (impl: create file on host, DE=unopened FCB, A=0 will a file was successfully created, otherwise 0xff, FCB is filled
;17H rename file                                                                                (impl: rename file on host, weird shit in FCD, wildcards allowed
;18H get login vector
;19H get default drive name                                     ; reads from DEFDRV ($F247)
;1AH set DMA address                                                      ; checked: writes to $F23D
;1BH get disk information
;1CH-20H no function
;21H read random file                                                           (impl: transfer to dma address, random record in FCB <-- record for readout, A=0 when successful, otherwise 0xff
;22H write random file                                                  (impl: transfer 128 bytes from dma address, random record in FCB <-- record to be written to, A=0 when successful, otherwise 0xff
;23H get file size                                                                      (impl: get file size, DE=unopened FCB, write filesize/128 to first 3 bytes of FCB's random record field
;24H set random record field                            (?? Current record position, calculated from the current block and record fields of specified FCB, is set in the random record field ??
;25H no function
;26H write random block                                                 (write 1..65535 bytes to file specified by FCB, DE=opened FCB, A=0 when successful, otherwise 0xff
;27H read random block                                                  (read records of size "FCB record size", HL=amount of records, return: A=0 when successful, otherwise 0xff, HL=amount of records read
;28H write random file (00H is set to unused portion)                           (??) much like 22H, except when the file becomes larger, 00H is written to the added records coming before the specified record.
;29H no function
;2AH get date                                                                                           (impl: could be implemented later, with an option to sync the usbhost clock or store a relative value to it)
;2BH set date                                                                                           idem
;2CH get time                                                                                           idem
;2DH set time                                                                                           idem
;2EH set verify flag                                                            (E=1 means turn on verify, otherwise turn verify off) (impl: we could force verify with a usbhost-option)
;2FH read logical sector                                                (could be implemented to improve speed in basic and dos!) DE=start-secor, H=amount of sectors, L=drive 00=A:, 01=B:, impl: transfer to DMA address
;30H write logical sector                                               (could be implemented to improve speed in basic and dos!) DE=start-secor, H=amount of sectors, L=drive 00=A:, 01=B:, impl: transfer from DMA address
