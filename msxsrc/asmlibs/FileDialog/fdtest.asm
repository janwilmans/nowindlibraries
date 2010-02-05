			OUTPUT "fdtest.com"
			
			DEFINE	G9K_DISABLE_CURSOR
			DEFINE  G9K_DISABLE_PATTERN
			DEFINE  BITBUSTER_OPTIMIZE_SPEED
			include "FileDialog.inc" 
			include "../macros.inc"
			include "..\bdos\bdos.inc"		
			include	"..\G9K\g9klib.inc"
	
						
			org     0100h
			
		        CALL    G9k.Detect
	                RET     NZ
	                CALL    G9k.Reset
	                LD      A,G9K_MODE_B3
	                LD      BC,G9K_SCR0_8BIT*256 + G9K_SCR0_XIM512 
	                LD      DE,256 + G9K_PAL_CTRL_PAL
	                CALL    G9k.SetScreenMode
	  
			; Set default blitter settings
	                G9kWriteReg G9K_ARG,0
	                G9kWriteReg G9K_LOP,G9K_LOP_WCSC
			LD      HL,#FFFF
	                CALL    G9k.SetCmdWriteMask
	           
	           	  ; Clear screen
	                LD      HL,BOX_SETTINGS
	                LD	DE,#0000
	                CALL    G9k.DrawFilledBox 
	           	G9kCmdWait   ; Wait for G9k.DrawFilledBox to finish
	           	 
	           	LD	HL,PALETTE	
			LD	B,6*3
			LD	C,0
			CALL	G9k.WritePalette

	                LD	DE,FONT_NAME
	                CALL	LoadFont
			LD	IX,font
	 	  	CALL	G9k.SetFont      ; Set font                
	                CALL    G9k.DisplayEnable
	                 
			LD	HL,DATA_BUFFER
			CALL	FileDialog.Open			
			
			CALL	FileDialog.GetDriveTypes
		
			LD	DE,FileDialog.diskInfo
			LD	B,8*DISK_INFO
			CALL	String.PrintDebug
			
			RET
			
		
LoadFont:
; Input DE=pointer to file name

			LD	IX,font
	                CALL    G9k.OpenVff
	                RET	NZ	; Return if error loading font
	                
			LD	IY,FONT_OFFSET_TABLE
	                LD	IX,font
	                LD	HL,DATA_BUFFER
	                LD	BC,0
	                CALL    G9k.LoadFont 
	                RET	NZ	; Return if error loading font
	                
	                LD	IX,font
	                JP	G9k.Close	
	                	
FONT_NAME		DB	"DEFAULT.VFF",0			
font			VFF_OBJECT	

FONT_OFFSET_TABLE 	DS	512,0

PALETTE:		DB	0,0,0
			DB	31,31,31
			DB	1,1,1
			DB	19,20,23
			DB	9,10,11
			DB	14,16,19
			
					
BOX_SETTINGS    	DW      0,0
               		DW      512,424
                			
			include "file.asm"
			include "..\bdos\bdos.asm"		
			include "..\String\string.asm"	
			include "..\G9K\g9klib.asm"
			include	"..\bitbuster\bitbuster.asm"
			include "FileDialog.asm"
			include "..\math\math.asm"
			include "../window/window.asm"
DATA_BUFFER:		
