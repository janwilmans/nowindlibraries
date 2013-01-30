                OUTPUT p1test.com
                include	"g9klib.inc"
                include "bdos.inc"
		include "macros.inc"
		
                org     0100h
                
                CALL    G9k.Detect
                RET     NZ
                CALL    G9k.Reset
                LD      A,G9K_MODE_P1
                LD      BC,G9K_SCR0_4BIT*256 + G9K_SCR0_XIM512 
                LD      DE,4 ; Set pallete for layer b on second pallete
                CALL    G9k.SetScreenMode
   		
   		CALL    G9k.DisplayEnable
		
		; Set default blitter settings
                G9kWriteReg G9K_ARG,0
                G9kWriteReg G9K_LOP,G9K_LOP_WCSC
		LD      HL,#FFFF
                CALL    G9k.SetCmdWriteMask
      		
      		LD_EHL  G9K_SCRA_PAT_NAME_TABLE
      		CALL	G9k.SetVramWrite
      		CALL	SetPatterns
      	
      		LD_EHL  G9K_SCRB_PAT_NAME_TABLE
      		CALL	G9k.SetVramWrite
      		CALL	SetPatterns

		; load pattern data layer A
  	        LD	A,0	       ; Palette pointer
 		LD	IY,0
 		LD	HL,0
      		LD	DE,G9B_FILE_A	
      		CALL	LoadPicture          
 
		; load pattern data layer A

  	        LD	A,16*4	       ; Palette pointer
 		LD	IY,4
 		LD	HL,0
      		LD	DE,G9B_FILE_B	
      		CALL	LoadPicture          
 
                
                RET

SetPatterns:
     		LD	HL,0
      		LD	E,L
		LD	D,27
		LD	C,G9K_VRAM
NextLine:
      		LD	B,32
.loop:		
      		OUT	(C),L
      		OUT	(C),H
      		INC	HL	
		DJNZ	.loop   
		 		
; fill patterns 32-63 with zeros		 		
      		LD	B,32
SkipLine:      		
      		OUT	(C),E
      		OUT	(C),E	
		DJNZ	SkipLine   
      		
      		DEC	D
      		JP	NZ,NextLine
      		RET

LoadPicture                
; Open a G9B file
		PUSH	AF
		PUSH	IY
		PUSH	HL
                LD	HL,g9bObject
                CALL	G9k.OpenG9B
                POP	HL
                POP	IY
                POP	AF
                                             
                LD	IX,g9bObject   ; Pointer to G9B object
                LD	DE,DATA_BUFFER ; Pointer to buffer 
                LD	BC,40000       ; Buffer size
         
                CALL	G9k.ReadG9BLinear 
                
                LD	IX,g9bObject
                JP	G9k.Close
                
G9B_FILE_A:	DB	"devildog.g9b",0
G9B_FILE_B:	DB	"RELIGIE.G9B",0


g9bObject	G9B_OBJECT
                
                include "g9klib.asm"
		include "file.asm"
		include "string.asm"
		include "bitbuster.asm"	
		include "bdos.asm"
		
DATA_BUFFER		