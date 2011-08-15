; Gfx9000 library test file

                OUTPUT g9ktest.com
                include	"../g9klib.inc"
                include "../../bdos/bdos.inc"
		include "../../macros.inc"
		
                org     0100h
Test:
                CALL    G9k.Detect
                RET     NZ
                CALL    G9k.Reset
                LD      A,G9K_MODE_B3
                LD      BC,G9K_SCR0_16BIT*256 + G9K_SCR0_XIM512 
                LD      DE,0
                CALL    G9k.SetScreenMode
  
		; Set default blitter settings
                G9kWriteReg G9K_ARG,0
                G9kWriteReg G9K_LOP,G9K_LOP_WCSC
		LD      HL,#FFFF
                CALL    G9k.SetCmdWriteMask
           
                ; Clear screen
                LD      HL,BOX_SETTINGS
                LD	DE,0
                CALL    G9k.DrawFilledBox  
                
                ; Wait for G9kDrawFilledBox to finish
                G9kCmdWait 
                    
                ; Load vff file, if command line is empty ANTIQUE.VFF is loaded 
                LD	HL,DATA_BUFFER                     
                LD	A,(CMD_LENGTH)
                OR	A,A
                LD	DE,FONT_NAME0
                JR 	Z,$+5                     
                    LD	  DE,#82	; Command line
             
                ; load font file
            	CALL	LoadFonts
            	RET	NZ		; Return if error loading font
	        
	        LD	IX,font0
	        CALL	G9k.SetFont      ; Set font
	        
		CALL	LoadPicture
    
                
                CALL    G9k.DisplayEnable
		LD      HL,7 * G9K_RED + 0 * G9K_GREEN + 0 * G9K_BLUE 
                CALL    G9k.SetCmdWriteMask
                LD	DE,7 * G9K_RED + 0 * G9K_GREEN + 0* G9K_BLUE 
                LD	HL,BOX_SETTINGS2
		CALL    G9k.DrawFilledBox
		; Wait for G9k.DrawFilledBox to finish
		G9kCmdWait
		
		LD      HL,#FFFF
                CALL    G9k.SetCmdWriteMask
		
		LD      HL,0 * G9K_RED + 0 * G9K_GREEN + 0 * G9K_BLUE 
                CALL 	G9k.SetCmdBackColor
                LD      HL,31 * G9K_RED + 31 * G9K_GREEN + 31 * G9K_BLUE 
                CALL    G9k.SetCmdColor
                G9kWriteReg G9K_LOP,G9K_LOP_WCSC+G9K_LOP_TP
                LD 	IX,150
                LD	IY,50
                LD	DE,WELCOME_TXT
                CALL	G9k.PrintString
                G9kCmdWait
                ; Print loaded font
                LD 	IX,110
                LD	IY,50
                LD	BC,(font1.height)
                ADD	IY,BC
          
                LD	A,(#80)
                OR	A,A
                LD	DE,FONT_NAME0
                JR 	Z,$+5                     
                    LD	  DE,#82	; Command line
                CALL	G9k.PrintString
                
                LD	IX,font1
	        CALL	G9k.SetFont      ; Set font
	        
	        LD	DE,FONT_NAME1
	        LD 	IX,110
                LD	IY,100
                CALL	G9k.PrintString

                LD	IX,font2
	        CALL	G9k.SetFont      ; Set font
	        
                LD	DE,.fontInRam	
                LD 	IX,110
                LD	IY,140
                CALL	G9k.PrintString 
                RET
                
.fontInRam	DB	"Font data in ram",0            
                        
LoadFonts:
; Input DE=pointer to file name

		LD	IX,font0
	        XOR	A,A	; Font in vram
                CALL    G9k.OpenVff
                RET	NZ	; Return if error loading font
                
		LD	IY,FONT_OFFSET_TABLE
                LD	IX,font0
                LD	HL,DATA_BUFFER
                LD	BC,0
                CALL    G9k.LoadFont 
                RET	NZ	; Return if error loading font
                
                LD	IX,font0
                CALL	G9k.Close
                
                
                LD	DE,FONT_NAME1
                LD	IX,font1
                XOR	A,A	; Font in vram
                CALL    G9k.OpenVff
                RET	NZ	; Return if error loading font
              
                LD	BC,(font0.dataSize)               
		LD	IY,FONT_OFFSET_TABLE ; Point to same offset table as font0
                LD	IX,font1
                LD	HL,DATA_BUFFER
                CALL    G9k.LoadFont 
                RET	NZ	; Return if error loading font
                
                LD	IX,font1
                CALL	G9k.Close

                LD	DE,FONT_NAME2
                LD	IX,font2
                LD	A,1	; Font in ram
                CALL    G9k.OpenVff
                RET	NZ	; Return if error loading font
                           	         
		LD	IY,FONT_OFFSET_TABLE2 
                LD	IX,font2
                LD	HL,FONT_DATA2
                CALL    G9k.LoadFont 
                RET	NZ	; Return if error loading font 
		
	        LD	IX,font2
                JP	G9k.Close
		
LoadPicture:               
; Open a G9B file
                LD	DE,G9B_FILE
                LD	HL,g9bObject
                CALL	G9k.OpenG9B
                                             
                LD	IX,g9bObject   ; Pointer to G9B object
                LD	DE,DATA_BUFFER ; Pointer to buffer 
                LD	BC,30000       ; Buffer size
                LD	HL,0	       ; X
                LD	IY,0           ; Y
                LD	A,0	       ; Palette pointer
                CALL	G9k.ReadG9B 
                
                LD	IX,g9bObject
                JP	G9k.Close		

BOX_SETTINGS
                DW      0,0
                DW      512,212
BOX_SETTINGS2			
		DW      100,50
                DW      312,100


                include "../g9klib.asm"
		include "../file.asm"
		include "../../string/string.asm"
		include "../../bitbuster/bitbuster.asm"	
		include "../../bdos/bdos.asm"



WELCOME_TXT	  DB	"Gfx9000 Library v0.004",0
FONT_NAME0	  DB	"ANTIQUE.VFF",0
FONT_NAME1	  DB	"COMPUTER.VFF",0
FONT_NAME2        DB    "CP111.VFF",0

G9B_FILE	  DB	"FIRE.G9B",0  ; 
g9bObject	  G9B_OBJECT
font0		  VFF_OBJECT	; Font data in vram
font1		  VFF_OBJECT	; Font data in vram
font2 		  VFF_OBJECT	; Font data in ram


		
FONT_OFFSET_TABLE  DS	512,0
FONT_OFFSET_TABLE2 DS	512,0
FONT_DATA2	   DS   5000,0

DATA_BUFFER	 ; Load routines need a buffer. This is declared here

