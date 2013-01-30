			OUTPUT "g9bview.com"
			
			DEFINE	G9K_DISABLE_CURSOR
			DEFINE  G9K_DISABLE_PATTERN
			DEFINE  G9K_DISABLE_VFF_VRAM ; Only use ram fonts
			DEFINE  BITBUSTER_OPTIMIZE_SPEED
					
			include	"g9klib.inc"
			include "bdos.inc"
			include "macros.inc"


; Info window size
WINDOW_TOP	EQU	100
WINDOW_LEFT	EQU	10
WINDOW_WIDTH	EQU	19*8
WINDOW_HEIGHT	EQU	3*8


; Keyboard Row 8
KEY_SPACE_BAR	EQU	0		
KEY_HOME	EQU	1
KEY_INS		EQU     2
KEY_DEL		EQU	3
KEY_LEFT	EQU	4
KEY_UP		EQU	5
KEY_DOWN	EQU	6
KEY_RIGHT	EQU	7
			
			STRUCT MODE_TABLE
width	  	        WORD
height			WORD
mode			BYTE			
	                ENDS 
	                
EXPTBL0         EQU     0FCC1h
CALSLT          EQU     1Ch
KILBUF          EQU     156h

			macro	BIOS adres	
			ld      iy,(EXPTBL0-1)       ;BIOS slot in iyh
			ld      ix,adres             ;address of BIOS routine
			call    CALSLT               ;interslot call
			endm

					
			org     0100h
G9BViewMain:
			CALL    G9k.Detect
			JP      NZ,NoGfx9kDetected
			CALL    G9k.Reset
			LD	A,(CMD_LENGTH)
			OR	A,A
			JP	Z,PrintHelp
			
			G9kWriteReg G9K_ARG,0
			G9kWriteReg G9K_LOP,G9K_LOP_WCSC
			LD      HL,#FFFF
			CALL    G9k.SetCmdWriteMask
			CALL    G9k.DisplayEnable
			
			LD	DE,CMD_LINE
			CALL	FindFirst
			JP	NZ,.ErrorOpening
			
			CALL	LoadFont
			JP	NZ,ErrorFont
			; Calculate free ram pointer
			LD	HL,(font.dataSize)
			LD	DE,DATA_BUFFER  ; Pointer to buffer 
			ADD	HL,DE
			LD	(freeRam),HL
				
.viewLoop			
			LD	HL,G9BObject
			LD	DE,fib
			CALL	G9k.OpenG9B			
			JP	NZ,.ErrorOpening
			
			LD	HL,0
			CALL    G9k.SetScrollY
			LD	HL,0
			CALL    G9k.SetScrollX
			
			LD	IX,G9BObject
			CALL	G9BViewSetScreenMode
			CALL	G9BClearScreen
			CALL	G9BCenterPicture ; Returns HL= X pos, IY = Y pos
			G9kCmdWait
			
			LD	DE,(freeRam)
			LD	IX,G9BObject   ; Pointer to G9B object
	                LD	BC,35000       ; Buffer size
	                LD	A,0	       ; Palette pointer
			CALL	G9k.ReadG9B
			JR	NZ,.ErrorLoading
.close			
			LD	IX,G9BObject  ; Pointer to G9B object	
			CALL	G9k.Close
			;CALL	DuplicatePalette
			
			CALL	CalcBackDataSize
			G9kCmdWait
			
			CALL	SaveBackGrnd
			CALL	G9BViewFileInfo
			
			CALL	CalcMaxScroll			
			CALL	KeyScan
			
			CALL	RestoreBackGrnd
			
			CALL	FindNext
			CP	A,ERROR.NOFIL
			JR	Z,.exit
			;CALL	WaitForKey
			JR	.viewLoop
			
.exit			
			BIOS	KILBUF
			RET		
		
.ErrorLoading
			CALL	 PrintError	
			JR	 .close
.ErrorOpening
			CP	A,_NOG9B
			JR	Z,.NotAG9bFile
			JP	PrintError
.NotAG9bFile
			LD	DE,NO_G9B_TXT
			BdosCall  _STROUT
			JR	 .close
						
freeRam			DW	0			
NO_G9B_TXT		DB	  "File is not a G9B file",CR,LF,LE			
			
PrintError:
			LD	  DE,Bdos.ERROR_STRING
			CALL	  String.PrintAsciiz
			LD	  DE,NEW_LINE
			BdosCall  _STROUT
			RET
			
ErrorFont:
			LD	  DE,.FONT_ERR_TXT
			CALL	  String.PrintAsciiz
			LD	  DE,Bdos.ERROR_STRING
			CALL	  String.PrintAsciiz			
			LD	  DE,NEW_LINE
			BdosCall  _STROUT
			RET
			
.FONT_ERR_TXT:		DB	"*** Error loading font - ",LF,CR
			
/*
WaitForKey:
			LD	 DE,PRESS_TXT
			BdosCall _STROUT
.keyScan			
			LD	C,8
			CALL    GetDirectKey
			BIT	KEY_SPACE_BAR,A
			JP	NZ,.keyScan
			
			LD	  DE,NEW_LINE
			BdosCall  _STROUT
			RET
			
			
PRESS_TXT		DB	"Press space to continue ",CR,LE
*/
G9BViewSetScreenMode:
			LD	A,(IX+G9B_OBJECT.bitDepth)
			SRA	A
			LD	HL,G9B_VIEW_BIT_DEPTH
			ADD_HL_A
			LD	B,(HL)	; Color depth
			INC	HL
			LD	C,(HL)  ; Image size	
			;Width 8bit mode it usefull to switch to image size 1024 if width>height
			LD	A,B
			CP	A,G9K_SCR0_8BIT
			JR	NZ,.no8Bit
			LD      HL,(IX+G9B_OBJECT.width)
			LD	DE,(IX+G9B_OBJECT.height)
			OR	A,A
			SBC	HL,DE
			JR	C,.no8Bit
			LD	C,G9K_SCR0_XIM1024		
.no8Bit		
			LD	DE,(IX+G9B_OBJECT.width)
			LD	A,(IX+G9B_OBJECT.bitDepth)
			CALL    G9BGetGfxMode 
			LD	DE,(IX+G9B_OBJECT.height)
			OR	A,A
			SBC	HL,DE
			LD	D,0
			JR	NC,.nonInterlaced
			LD	HL,(currentG9kHeight)	
			ADD	HL,HL;  height * 2
			LD	(currentG9kHeight),HL			
			LD	D,1
.nonInterlaced:			
			LD      E,(IX+G9B_OBJECT.colorType)		
			JP      G9k.SetScreenMode		
			

			
G9B_VIEW_BIT_DEPTH	DB	G9K_SCR0_2BIT,G9K_SCR0_XIM1024		
			DB	G9K_SCR0_4BIT,G9K_SCR0_XIM1024		
			DB	G9K_SCR0_8BIT,G9K_SCR0_XIM512           
			DB	0,0                              
			DB	G9K_SCR0_16BIT,G9K_SCR0_XIM512               

currentG9kWidth		DW	0
currentG9kHeight	DW 	0
  

G9BGetGfxMode:
; In    DE = width of G9B file
;        A = color depth
; Output A = gfx mode
;       HL = height from gfx mode
;       DE  = width from gfx mode
			PUSH	BC
			; check if bithdepth is higher or equal then 8 bit. In that case b4 and b7 are invalid 
			CP	A,8
			LD	B,4
			JR	C,.lowerThen8bit	
			LD	B,2
			
.lowerThen8bit			
			LD	IY,.modeTable
.loop			
			LD	HL,(IY + MODE_TABLE.width)
			OR	A,A
			SBC	HL,DE
			JR	NC,.end
			
			EX	DE,HL
			LD	DE,MODE_TABLE ; size of mode table struct
			ADD	IY,DE
			EX	DE,HL	
			DJNZ	.loop 
.end
			LD	A, (IY + MODE_TABLE.mode)
			LD	DE,(IY + MODE_TABLE.width)
			LD	(currentG9kWidth),DE
			LD	HL,(IY + MODE_TABLE.height)
			LD	(currentG9kHeight),HL
			POP	BC
			RET

.modeTable              MODE_TABLE 256, 212,G9K_MODE_B1
			MODE_TABLE 384, 240,G9K_MODE_B2
			MODE_TABLE 512, 212,G9K_MODE_B3
			MODE_TABLE 768, 240,G9K_MODE_B4
			MODE_TABLE 1024,212,G9K_MODE_B7

G9BCenterPicture:
; Input IX = Pointer to G9B object
; Output HL = X
;        IY = Y 

.width	
			LD	DE,(currentG9kWidth)
			LD	HL,(IX+G9B_OBJECT.width)
			OR	A,A
			SBC	HL,DE
			; If carry is not set then there is no width centering needed
			LD	HL,0
			JR	NC,.height	 
			
			LD	HL,(IX+G9B_OBJECT.width)
			EX	DE,HL
			OR	A,A
			SBC	HL,DE
			SRA	HL		; G9kWidth - G9bWidth /2			
.height			
			PUSH	HL	
			LD	DE,(currentG9kHeight)
			LD	HL,(IX+G9B_OBJECT.height)
			OR	A,A
			SBC	HL,DE
			; If carry is not set then there is no height centering needed
			LD	IY,0
			JR	NC,.end	 
			
			LD	HL,(IX+G9B_OBJECT.height)
			EX	DE,HL
			OR	A,A
			SBC	HL,DE
			SRA	HL		; G9kHeight- G9bheight /2	
			LD	IY,HL		
.end
			POP	HL	
			RET		
G9BClearScreen:
			LD      HL,clearScreen
			LD	DE,0
			JP      G9k.DrawFilledBox  

clearScreen		G9K_BOX	0,0,512,512	

/*
DuplicatePalette:
; Copy palette for b6 and b7 
			LD	B,16*3
			LD	C,0
			LD	HL,(freeRam)
			PUSH	HL
			CALL	G9k.ReadPalette
			POP	HL
			LD	B,16
			LD	C,4*32
			CALL	G9k.WritePalette
			RET
*/

CalcMaxScroll:
; Input IX = Pointer to g9b object		
			
			; Check if Y scrolling is needed 
			LD	HL,0
			LD	(maxX),HL
			LD	(maxY),HL
			LD  	HL,(currentG9kHeight)
			LD  	DE,(IX+G9B_OBJECT.height)
			PUSH	HL
			SBC 	HL,DE
			POP	HL	 ;  display height - g9b object height
			JR      NC,.calcMaxX
			
			EX	DE,HL
			OR  	A,A
			SBC 	HL,DE    ; Maximum scroll value
			LD	(maxY),HL
.calcMaxX			
			LD  	HL,(currentG9kWidth)
			LD  	DE,(IX+G9B_OBJECT.width)
			PUSH	HL
			SBC 	HL,DE
			POP	HL	 ;  display width - g9b object width
			;JR      NC,.noXscroll
			RET	NC
			EX	DE,HL
			OR  	A,A
			SBC 	HL,DE    ; Maximum scroll value
			LD	(maxX),HL
			RET
							
KeyScan:			
			LD	DE,0      ; Current X scroll offset
			LD	IY,0      ; Current Y scroll offset

.keyScan			
			LD	C,8
			CALL    GetDirectKey
			BIT	KEY_SPACE_BAR,A
			RET	Z
						
			BIT	KEY_LEFT,A
			CALL	Z,.left
			
			BIT	KEY_RIGHT,A
			CALL	Z,.right

			BIT	KEY_UP,A
			CALL	Z,.up
			
			BIT	KEY_DOWN,A
			CALL	Z,.down
			
			JP	.keyScan


			
; Scroll Y 							
.up
			PUSH	AF	
					
			PUSH	DE
			LD	HL,(maxY)
			OR	A,A
			EX	DE,HL			
			SBC	HL,DE	
			POP	DE
			
			JR	NC,.noScrollY
			INC	DE
			
			PUSH	IY
			PUSH	DE
			CALL	RestoreBackGrnd
			LD	HL,WINDOW_TOP
			ADD	HL,DE
			LD	(vramCopy.top),HL
			CALL	SaveBackGrnd
			CALL	G9BViewFileInfo
			POP	DE
			POP	IY			
			POP	AF
			JR      .SetScrollY
.noScrollY		
			POP	AF
			RET			
			

			
.down			
			PUSH	AF
			LD	A,D
			OR	A,E
			JR	Z,.noScrollY				
			DEC	DE
			
			PUSH	IY
			PUSH	DE
			CALL	RestoreBackGrnd
			LD	HL,WINDOW_TOP
			ADD	HL,DE
			LD	(vramCopy.top),HL
			CALL	SaveBackGrnd
			CALL	G9BViewFileInfo
			POP	DE
			POP	IY
			POP	AF
.SetScrollY
			LD	HL,DE
			PUSH	AF	
			G9kWaitVsync
			CALL    G9k.SetScrollY
			POP	AF
			RET	

; Scroll X 				
.left
			PUSH	AF
			LD	BC,(maxX) 
			LD	HL,IY
			OR	A,A
			SBC	HL,BC
			
			JR	NC,.noScrollX
			INC	IY

			PUSH	IY
			PUSH	DE
			CALL	RestoreBackGrnd
			LD	BC,WINDOW_LEFT
			ADD	IY,BC
			LD	(vramCopy.left),IY
			CALL	SaveBackGrnd
			CALL	G9BViewFileInfo
			POP	DE
			POP	IY
			POP	AF
			JR      .SetScrollX
.noScrollX		
			POP	AF
			RET	
			
.right
			PUSH	AF
			LD	A,IYH
			OR	A,IYL
			JR	Z,.noScrollX	
			DEC	IY
			
			PUSH	IY
			PUSH	DE
			CALL	RestoreBackGrnd
			LD	BC,WINDOW_LEFT
			ADD	IY,BC
			LD	(vramCopy.left),IY
			CALL	SaveBackGrnd
			CALL	G9BViewFileInfo
			POP	DE
			POP	IY
			POP	AF
.SetScrollX
			LD	HL,IY
			PUSH	AF	
			G9kWaitVsync
			CALL    G9k.SetScrollX
			POP	AF
			RET	
			
maxY			DW	0			
maxX			DW	0						


;------------------------------------------------
;--- File info window
;------------------------------------------------
G9BViewFileInfo:
; Input  IX = Pointer to G9B_OBJECT
			PUSH	IX   
			LD      HL,0
	                CALL 	G9k.SetCmdBackColor
	                	             	           
        	        LD      HL,0FFFFh
                	CALL    G9k.SetCmdColor
			
			;File   Name
		        LD	DE,.FILE_TXT
	                LD 	IY,(vramCopy.top)
        	        LD	IX,(vramCopy.left)        	    
        	        CALL	G9k.PrintString 

			LD	DE,1*8			
	 	        ADD	IX,DE
		        LD	DE,fib.fileName
        	        CALL	G9k.PrintString 

        	        ; Resolution width
        	        LD	DE,1*8          
        	        ADD	IY,DE		; Next line
		        LD	DE,.WIDTH_TXT
	                LD 	IX,(vramCopy.left) 
        	        CALL	G9k.PrintString 
        	        
        	        LD	HL,(G9BObject.width)
        	        CALL	UnsignedInt16ToStr
			LD	DE,1*8	
        	        ADD	IX,DE 		; Advance 1 char
        	        LD	DE,UnsignedInt16ToStr.STRING       	    
        	        CALL	G9k.PrintString  
	
	       	        ; Resolution height
        	        LD	DE,1*8
        	        ADD	IY,DE		; Next line
		        LD	DE,.HEIGHT_TXT
	                LD 	IX,(vramCopy.left) 
        	        CALL	G9k.PrintString 
        	        
        	        LD	HL,(G9BObject.height)
        	        CALL	UnsignedInt16ToStr
        	        ;
        	        ;LD	IX,WINDOW_LEFT+6*8 
        	        LD	DE,UnsignedInt16ToStr.STRING       	    
        	        CALL	G9k.PrintString  
			
			POP	IX
			
			RET
			
.FILE_TXT		DB	"File",0
.WIDTH_TXT		DB	"Width",0
.HEIGHT_TXT		DB	"Height",0
.BIT_PER_PIXEL_TXT	DB	"Bits per Pixel",0
.COLORS_TXT		DB	"Colors",0
.COMPRESSION_TXT	DB	"Compressed",0
.YES_TXT		DB	"Yes",0
.NO_TXT			DB	"No",0
.YJK			DB	"YJK",0
.YUV			DB	"YUV",0

SaveBackGrnd:		

			LD	HL,vramCopy
			CALL	G9k.SetupCopyXYToRam
			G9kWaitComReady
			LD	BC,(backGrndDataSize)
			LD	HL,(freeRam)
			JP 	G9k.CopyXYToRam	
					
RestoreBackGrnd:
			
			LD	HL,vramCopy
			CALL	G9k.SetupCopyRamToXY
			G9kWaitComReady	
			LD	BC,(backGrndDataSize)
			LD	HL,(freeRam)
			JP	G9k.CopyRamToXY
			

CalcBackDataSize:
; IX Pointer to g9b object
			LD	HL,WINDOW_WIDTH * WINDOW_HEIGHT /2 ; Data size of 4 bit
			LD	A,(IX + G9B_OBJECT.bitDepth)	
			CP	A,4
			JR	Z,.setSize
			ADD	HL,HL
			CP	A,8
			JR	Z,.setSize
			ADD	HL,HL  ; 16 bit
.setSize
			LD	(backGrndDataSize),HL					
			RET
			
backGrndDataSize	DW	0

vramCopy		G9K_BOX	WINDOW_LEFT,WINDOW_TOP,WINDOW_WIDTH,WINDOW_HEIGHT					
			
;------------------------------------------------
;--- Load font
;------------------------------------------------


LoadFont:
			LD	DE,fontName
			LD	IX,font
		        LD	A,1     ; Font in ram
	                CALL    G9k.OpenVff
	                RET	NZ	; Return if error loading font
	                	                
			LD	IY,FONT_OFFSET_TABLE
	                LD	IX,font
	                LD	HL,DATA_BUFFER
	                LD	BC,8000h
	                CALL    G9k.LoadFont 
	                RET	NZ	; Return if error loading font
	      	      	  	                
	                LD	IX,font            
	                CALL	G9k.Close
	                
	                LD	IX,font
		        CALL	G9k.SetFont      ; Set font
	                XOR	A,A
	                RET	   	             			              	                

font			VFF_OBJECT
FONT_OFFSET_TABLE	DS	512,0
         		       		
			include "file.asm"
			include "g9klib.asm"
			include "string.asm"
			include "bdos.asm"
			include "bitbuster.asm"
		

NEW_LINE		DB	CR,LF,LE
G9BObject		G9B_OBJECT 


FindFirst:
			LD	 B,0
			LD	 IX,fib
			BdosCall _FFIRST
			jp	 nz,Bdos.ExplainError
			ret

FindNext:
			LD	 IX,fib
			BdosCall _FNEXT
			RET
			
fib			FIB

;------------------------------------------------
;--- Direct Key scan for Key input
;------------------------------------------------

; IN : C = Row number
; Out: A = Keys
GetDirectKey:
			PUSH	BC
			DI
			IN	A,(#AA) 
			LD	B,A
			AND	A,#F0
			ADD	A,C
			NOP
			OUT	(#AA),A
			IN	A,(#A9)
			LD	C,A
			LD	A,B
			EI
			OUT	(#AA),A
			LD	A,C
			POP	BC
			RET
			
				
UnsignedInt16ToStr:
; Input HL
; Output .STRING
			PUSH	HL
			PUSH	DE
			PUSH	BC
			LD	DE,.STRING+1
			CALL	.Convert
			XOR	A,A
			LD	(DE),A
			LD	HL,.STRING
			LD	(HL),' '
			POP	BC
			POP	DE
			POP	HL
			RET

.Convert
			XOR	A
			LD	B,16
			LD	C,A
.a	
			ADD	HL,HL
			RL	C
			LD	A,C
			SUB	A,10
			JR	C,.b
			LD	C,A
			INC	HL
.b
			DJNZ	.a
			LD	A,C
			ADD	A,'0'
			PUSH	AF
			LD	A,H
			OR	L
			CALL	NZ,.Convert
			POP	AF
			LD	(DE),A
			INC	DE
			RET

.STRING			DS	10
		

DATA_BUFFER
; Functions and data from here only used during init


fontName		DB    "DIGITAL.VFF",0,"$"

PrintHelp:		
			LD	DE,G9B_VIEW_TXT
			BdosCall _STROUT
			RET

G9B_VIEW_TXT		DB "G9BViewer V0.43",CR,LF,LF
			DB	"Usage:",CR,LF,LF
			DB	"G9BVIEW [D:][PATH]FILENAME.EXT",LF,LF,CR,LE
			
NoGfx9kDetected:
			LD	DE,NO_GFX9K_TEXT
			BdosCall _STROUT
			RET	
			
NO_GFX9K_TEXT		DB	"*** No Gfx9000 detected",LF,CR,LE			