FDD_IMAGE     EQU 0
HDD_IMAGE     EQU 1 
  
              ; Nowind Test Application
              OUTPUT "nta.com"
              include "..\asmlibs\bdos\bdos.inc"
              include "..\asmlibs\bios\bios.inc"
              include "..\asmlibs\Keyboard\keyboard.inc"
                
              ORG   COM_START_ADDR
Main:             
              LD    DE,START_TEXT
              CALL  String.PrintAsciiz
              
              
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
              ; Handle other characters as error
              
              JP  PrintHelp
              ;LD (FILE_NAME_PTR),HL
              ;LD B,1 ; End search
.next             
              INC HL
              DJNZ .scanLine  
              
              ; no option? print help
PrintHelp:
              LD    DE,HELP_TEXT
              CALL  String.PrintAsciiz

Exit:
              RET
              
HandleOption:
; HL = Pointer to current pos in command line
;  B = Characters remaining in cmd line
                INC   HL
                LD    A,(HL)
                DEC   B
                AND   A,UPPER_CASE_MASK
                CP    A,"R"
                JP    Z,ReadTest
                CP    A,"W"
                JP    Z,WriteTest
                                
                LD    DE,INVALID_OPTION
                CALL  String.PrintAsciiz
                LD    B,1 ; Make sure program exits
                JP    Main.next

WriteTest:
ReadTest:
                LD    C,_SETDTA
                LD    DE,BUFFER   
                CALL  BDOS        ; set disk transfer address
.loop
                LD    C,_RDABS
                LD    DE,0
                LD    L,0         ;(0=A: etc.)
                LD    H,1         
                CALL  BDOS        ; read 1 sector
                
                LD    C,7
                CALL  Keyboard.GetDirectKey
                
                AND   KEY_ESC
                jr    nz,.loop
                RET
              
FILE_NAME_PTR:  DW  0
IMAGE_TYPE:     DB  FDD_IMAGE

START_TEXT:     DB "NOWIND Test Application - NOWIND (c) 2009",CR,LF,0
HELP_TEXT:      DB CR,LF
                DB "NTA {OPTIONS}",CR,LF  
                DB " Options",CR,LF
                DB " /R - read test",CR,LF
                DB " /W - write test",CR,LF
                DB 0    
                  
INVALID_OPTION: DB " *** Invalid option!!!",CR,LF,0
FILE_NAME:      DS 64,0
BUFFER:         DS 1024               
    
include "..\asmlibs\string\string.asm"
include "..\asmlibs\Keyboard\keyboard.asm"

    