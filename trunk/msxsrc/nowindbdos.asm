
BDOSNW:
; is this a virtual drive? if not then call 56D3

                                DEBUGMESSAGE "BDOS CALL wrapped"
                                DEBUGDUMPREGISTERS
                                push af
                                push bc
                                push de
                                push hl
                                
                                ld a,c
                                cp 0x0F
                                jr z,hooked_bdoscall
                                cp 0x11
                                jr z,hooked_bdoscall
                                cp 0x12
                                jr z,hooked_bdoscall
                                jr exit 

hooked_bdoscall:

                                push hl
                                push de
                                call sendRegisters
                                ld (hl),c                                       ; send the original BDOS command code
                                pop de
                                pop hl
                                
                                call sendFCB 
                                
                                jr exit
        
; just return from the interslot call after handling it
                                pop hl
                                pop de
                                pop bc
                                pop af  
                                ret

exit:
                                pop hl
                                pop de
                                pop bc
                                pop af

                                jp $56D3
                                
                                
BDOS_0FH:
                                DEBUGMESSAGE "BDOS CALL 0FH"
                                DEBUGDUMPREGISTERS
                                
                                push hl
                                push de
                                call sendRegisters
                                ld (hl),$0f                                     ; send the original BDOS command code
                                pop de
                                pop hl

                                ; if first byte of FCB is 0x00 then replace it with the default drive
                                ld a,(de)
                                or a
                                jr nz,.nodefault
                                ld a,($F247)                    ; todo: add a label DEFDRV? 
                                ld (de),a
.nodefault:

                                call sendFCB
                                
                                ; enabled slots?
                                call enableNowindPage0          ; old page 0 slot-selection is kept in IXh 
                                call getHeader
                                call receiveFCB                                         ; receive 32 bytes and write them to (DE+x)
                                call restorePage0
                                
                                ; failed
                                ld a,$ff
                                ret
                                
                                ; resume normal operation 
                                jp $42A5                        ; call was overwritten
                                
BDOS_11H:
                                DEBUGMESSAGE "BDOS CALL 11H"
                                DEBUGDUMPREGISTERS

                                push hl
                                push de
                                call sendRegisters
                                ld (hl),$11                             ; send the original BDOS command code
                                pop de
                                pop hl
                                call sendFCB

                                call enableNowindPage0          ; old page 0 slot-selection is kept in IXh 
                                call getHeader
                                call receiveFCB                                         ; receive 32 bytes and write them to (DE+x)
                                call restorePage0
                                
                                ; resume normal operation 
                                jp $42A5                        ; call was overwritten
                
BDOS_12H:
                                DEBUGMESSAGE "BDOS CALL 12H"
                                DEBUGDUMPREGISTERS
                                
                                push hl
                                push de
                                call sendRegisters
                                ld (hl),$12                                     ; send the original BDOS command code
                                pop de
                                pop hl
                                call sendFCB                            ; send 32 bytes (DE+x) to the host
                                
                                call enableNowindPage0          ; old page 0 slot-selection is kept in IXh 
                                call getHeader
                                call receiveFCB                                         ; receive 32 bytes and write them to (DE+x)
                                call restorePage0
                                
                                ; resume normal operation 
                                jp $440e                        ; call was overwritten


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

; TODO: the list of command in the redbook says: 21H: write, 22H: read, check this.

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