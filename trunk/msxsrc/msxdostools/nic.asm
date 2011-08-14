FDD_IMAGE     EQU 0
HDD_IMAGE     EQU 1 
  
              ; Nowind Image Changer, inspired by Marcel Delorme
              OUTPUT "nic.com"
              include "..\asmlibs\bdos\bdos.inc"
              include "..\asmlibs\bios\bios.inc"
                
              ORG   COM_START_ADDR
Main:             
              LD    DE,START_TEXT
              CALL  String.PrintAsciiz
              
              ;LD   D,"N"
              ;LD   E,0       ; method: get nowind interface slot
              ;XOR  A         ; find first nowind interface
              ;CALL EXTBIO

              ;LD   D,"N"
              ;LD   E,1       ; method: get number of interfaces
              ;XOR  A         ; set a to zero first
              ;CALL EXTBIO    ; returns number of interfaces in A

              ;LD   D,"N"
              ;LD   E,1       ; method: activate other interface
              ;XOR  A         ; number of interface to activate
              ;CALL EXTBIO

              
              LD  A,(CMD_LENGTH)
              OR  A,A
              JP  Z,PrintHelp
              
              LD  B,A 
              LD  HL,CMD_LINE
.scanLine             
              LD  A,(HL)
              CP  A,' '
              JP  Z,.next 
              CP  A,'/'
              JP  Z,HandleOption                  
              ; Handle characters as file name
              LD  (FILE_NAME_PTR),HL
              LD  B,1 ; End search
.next             
              INC HL
              DJNZ .scanLine  
              
              LD  HL,(FILE_NAME_PTR)
              LD  A,L
              OR  A,H
              RET Z ; No file name specified
              
              ; LD  D,H
              ; LD  E,L
              ; CALL  String.PrintAsciiz
              CALL SetImage           
Exit:
              RET

PrintHelp:
              LD    DE,HELP_TEXT
              CALL  String.PrintAsciiz
              RET

HandleOption:
; hl = Pointer to current pos in command line
; b  = Characters remaining in cmd line
                INC   HL
                LD    A,(HL)
                DEC   B
                AND   A,UPPER_CASE_MASK
                CP    A,"H"
                JP    Z,HddImage
                CP    A,"L"
                JP    Z,ListImages
                                
                LD    DE,INVALID_OPTION
                CALL  String.PrintAsciiz
                LD    B,1 ; Make sure program exits
                JP    Main.next

HddImage:
                LD  A,HDD_IMAGE
                LD  (IMAGE_TYPE),A
                JP  Main.next
              
              
ListImages: 
                JP  Main.next                           

SetImage:
            
                ; Save current slot         
                LD    A,(SLOT_ID_RAM_1)
                LD    (CUR_SLOT_PAG1),A 
                
                ; Select slot for NOWIND USB 
              
                XOR  A					; find the first nowind interface, and reset carry flag
                LD   DE,$4E00 			; D= 'N' E= function 0
                CALL  EXTBIO			; Nowind-EXTBIO call returns nowind slotcode in A
                jr    nc,not_found
                
                LD    H,0x40
                CALL  ENASLT
      
                LD    A,(IMAGE_TYPE)
                CALL  sendRegisters
                LD    (HL),$91                  ; C_CHANGEIMAGE 
                LD    HL,(FILE_NAME_PTR)
                 ; TODO: add length check to prevent sending bull
.loop:        
                LD    A,(HL)
                INC   HL
                LD    (0x04000),A
                OR    A,A
                JP    NZ,.loop
                
                ; Restore slot
                LD    A,(CUR_SLOT_PAG1)
                LD    H,0x40                    
                CALL  ENASLT  
                RET
not_found:
				LD	DE,NFOUND_TEXT
				CALL  String.PrintAsciiz
				RET

sendRegisters:
                push af
                ld a,h
                ld h,$40                        ; TODO: User USBRW
                ld (hl),$af                     ; header
                ld (hl),$05
                ld (hl),c
                ld (hl),b
                ld (hl),e
                ld (hl),d
                ld (hl),l
                ld (hl),a                       ; register h
                pop de
                ld (hl),e                       ; register f
                ld (hl),d                       ; register a
                ret

                
CUR_SLOT_PAG1:  DB  0

              
FILE_NAME_PTR:  DW  0
IMAGE_TYPE:     DB  FDD_IMAGE

START_TEXT:     DB "NOWIND Image Changer - NOWIND (c) 2009",CR,LF,0
NFOUND_TEXT:    DB "No Nowind Interface found!",CR,LF,0
HELP_TEXT:      DB CR,LF
                DB "NIC {OPTIONS} PATH\\FILENAME",CR,LF  
                DB " Options",CR,LF
                DB " /H - Image is a HDD Image",CR,LF
                DB " /L - List Images",CR,LF
                DB 0    
                  
INVALID_OPTION: DB " *** Invalid option!!!",CR,LF,0
FILE_NAME       DS 64,0
                
    
   include "..\asmlibs\string\string.asm"

    
