 		ifndef FileDialog_ASM
		define FileDialog_ASM


		include "FileDialog.inc"

		MODULE FileDialog



;----------------------------------------------------------------------------;
; Get entries from disk functions                                            ;
;----------------------------------------------------------------------------;


GetDirectories:
; Input DE = Pointer to file entry table
;       BC = Max files
; Output A = error
;       IX = entries found
;       DE = Pointer to end of file entry table
;       BC = Max files - entries found

		LD	IX,0		; Number of entries 0
		
		PUSH	IX
		PUSH	DE
		PUSH	BC
		LD	IX,fib	
 		LD	DE,.DIR_NAME
 		LD	B,FILE_DIRECTORY
		CALL	Bdos.FindFirst
		POP	BC
		POP	DE
		POP	IX
		RET	NZ		; Exit on error
		
		LD	HL,fib.fileName
		LD	A,'.'
		CP	A,(HL)
		JR	Z,.Next ; Skip entry if it's a point, only the first entry can be .
		
.CopyName	
		LD	HL,fib.fileAttrib
		BIT	4,(HL)		; FILE_DIRECTORY
		JR	Z,.Next
		
		PUSH	BC
		LD	A,ENTRY_DIR
		LD	(DE),A
		INC	DE
		
		LD	HL,fib.fileName
		LD	BC,13
		LDIR
		
		LD	HL,fib.lastModTime ; Copy date and time
		LD	BC,4
		LDIR
		
		INC	DE
		INC	DE
		INC	DE
		INC	DE
		
		
		INC	IX		   ; Increase entries
		POP	BC
			
.Next		
		PUSH    DE
		PUSH	IX
		LD	IX,fib
		CALL	Bdos.FindNext
		POP	IX
		POP	DE
		JR	NZ,.End
		
		DEC	BC
		LD	A,B
		OR	A,C
		JP	NZ,.CopyName
.End		
		XOR 	A,A	; Clear flags
		RET
		
.DIR_NAME	DB	"*.*",0


		 
GetFiles:
; Input DE = Pointer to file entry table
;       HL = Pointer to wild card
;       BC = Max files
; Output A = error
;       IX = entries found
;       DE = Pointer to end of file entry table
;       BC = Max files - entries found
	        LD	IX,0		; Number of entries 0
	        
	        PUSH	IX
		PUSH	DE
		PUSH	BC
		LD	IX,fib	
 		EX	DE,HL
 		LD	B,0
		CALL	Bdos.FindFirst
		POP	BC
		POP	DE
		POP	IX
		RET	NZ		; Exit on error
		


.CopyName		
		PUSH	BC
		
		LD	A,ENTRY_FILE
		LD	(DE),A
		INC	DE
		
		LD	HL,fib.fileName
		
		; Leave first char upercased
		LDI	A,(HL)
		LDI	(DE),A
		
		LD	B,12
.lowerCase:
		LDI	A,(HL)
		CP	A,"A"
		JR	C,.skip
		CP	A,"Z"+1
		JR	NC,.skip
		XOR	A,32
.skip:		
		LDI	(DE),A
		DJNZ	.lowerCase
		
		
		
		LD	HL,fib.lastModTime ; Copy date and time
		LD	BC,4
		LDIR
		
		LD	HL,fib.fileSize ; Copy file size
		LD	BC,4
		LDIR		
		
		
		INC	IX	; Increase entries
		POP	BC

.Next		
		PUSH    DE
		PUSH	IX
		LD	IX,fib
		CALL	Bdos.FindNext
		POP	IX
		POP	DE
		JR	NZ,.End
		
				
		DEC	BC
		LD	A,B
		OR	A,C
		JP	NZ,.CopyName
.End		
		XOR 	A,A	; Clear flags
		RET
		 	 



GetEntries
; DE = Pointer to file entry table
; BC = Max files
; HL = Pointer to wildcard	
			
		PUSH	HL
		LD	HL,fileInfo.entryOffset
		LD	(HL),0	       ; Reset entry offset
		CALL	GetDirectories ; Return IX=entries found,DE=Pointer to end of file entry table ,BC = Max files - entries found
		LD	(fileInfo.nrDirs),IX
		POP	HL
		PUSH	IX
		CALL	GetFiles     ; Only get files if GetDirectories return width success
		LD	(fileInfo.nrFiles),IX
		POP	DE
		ADD	IX,DE
		LD	(fileInfo.totalEntries),IX
		
		RET

;----------------------------------------------------------------------------;
; View files functions                                                       ;
;----------------------------------------------------------------------------;

ViewFilesExt:
;  HL = Pointer to file entry table
;  IX = Pointer to file view struct
;  BC = Nr entries
		
		PUSH 	HL
		PUSH 	BC

		LD	HL,IX
		;LD	DE,(IX+FILE_VIEW.backColor)
		LD	DE,FONT_COLOR_BACK_GND
		PUSH 	DE ; Save back color
		DI
		CALL 	G9k.DrawFilledBox	; Clear file view
		EI
		LD 	E,(IX+FILE_VIEW.rows)
		LD      A,E
		LD 	(.rows),A
		LD 	D,(IX+FILE_VIEW.columns)
				
		LD	BC,(IX+FILE_VIEW.top)
		LD      (.top),BC
		LD	IY,BC
		
		LD	BC,(IX+FILE_VIEW.left)
		LD	IX,BC				
		POP     HL ; Back color
		G9kCmdWait  ; Wait for G9k.DrawFilledBox
		DI
               	CALL    G9k.SetCmdBackColor  
               ;	LD	HL,(IX+FILE_VIEW.fontColor)  
                LD	HL,FONT_COLOR
               	CALL    G9k.SetCmdColor
               	G9kWriteReg G9K_LOP,G9K_LOP_WCSC+G9K_LOP_TP
               	EI
		POP	 BC
		POP	 HL
		LD	 A,B
		OR	 A,C 
		RET	 Z  	    ; No files to view
		
.next		
		PUSH	 BC
		LD	 A,(HL)	    ; FILE_ENTRY.type	
		PUSH	 HL
		INC	 HL		
		OR	 A,A	    ; ENTRY_FILE
		DI
		CALL	 PrintEntry
		EI
.calcNewPos	
		LD	 A,(G9k.PrintString.HEIGHT)
		LD	 C,A
		LD	 B,0
		ADD	 IY,BC       ; Next row
		DEC	 E           ; Dec nr rows
		JR	 NZ,.decEntries   
		
		DEC	 D
		JR	 Z,.exit     ; No more columns	
			
		LD	 A,(.rows)
		LD	 E,A         ; Reset number of rows
		LD	 BC,8*15     ; Should be dynamic 
		ADD	 IX,BC	
		LD	 IY,(.top)
.decEntries		
		POP	 HL
		LD 	 BC,FILE_ENTRY
		ADD	 HL,BC	
		POP	 BC
		DJPNZ_BC .next	
		G9kWriteReg G9K_LOP,G9K_LOP_WCSC
		RET
.exit	
		POP	HL
		POP	BC
		G9kWriteReg G9K_LOP,G9K_LOP_WCSC
		RET
			
.rows		DB	0
.top		DW	0

PrintEntry:
		JR	 Z,PrintFileEntry
		JR	 PrintDirEntry	

		
PrintFileEntry:
; HL = Pointer to entry name
; IX = X
; IY = Y	
		PUSH	DE
		PUSH	IX
		LD	BC,(G9k.PrintString.WIDTH)
		ADD	IX,BC
		ADD	IX,BC ; Skip two chars
		EX	DE,HL
		DI
		CALL	G9k.PrintString
		EI
		POP	IX
		POP	DE
		RET
		
		
PrintDirEntry:
; HL = Pointer to entry name
; IX = X
; IY = Y	
		PUSH	DE
		PUSH	IX
		PUSH	HL
		DI
		LD	HL,iconMap
		CALL	G9k.CopyXYToRegisterXY
		LD	DE,16
		ADD	IX,DE
		POP	DE
		CALL	G9k.PrintString
		EI
		POP	IX
		POP	DE
		RET			
		
fileInfo	FILE_INFO

iconMap		G9K_BOX 64,848+32,16,14	
	
GetFullPathName:
		BdosCall _CURDRV
		ADD	A,"A"
		LD	(FULL_PATH_NAME.CUR_DRIVE),A
	
		LD	B,0
		LD	HL,FULL_PATH_NAME.PATH_NAME
		LD	(HL),0
		EX	DE,HL
		BdosCall _GETCD
		
		LD	HL,FULL_PATH_NAME.PATH_NAME	; Maximum chars for path name
		LD	BC,40 ; Max chars to dislay
		XOR	A,A
		CPIR		; Search end off path
		RET	PO
		DEC	HL
		LD	B,C	
.ClearPath:
		LD	(HL)," "
		INC	HL
		DJNZ	.ClearPath
		LD	(HL),0		
		RET	
		
FULL_PATH_NAME		
.CUR_DRIVE	DB	"A:\\"
.PATH_NAME	DS	64,0



GetEntryNumber:
; HL =  X
; DE =  Y
; Result  HL = entry number
;          C = Column number
;          B = Row number

		PUSH	DE
		LD	DE,COLUMN_SIZE
		CALL	Math.HLDivDE	; Calculate column
		LD	C,L	        ; Save Column number
		; HL = Column number	
		LD	DE,ROWS	
		PUSH	BC
		CALL	Math.HLMulDE    ; Calculate column * number of rows
		POP	BC 
		POP	DE
		
		PUSH	HL	    
		EX	DE,HL
		LD	DE,ROW_SIZE
		PUSH	BC
		CALL	Math.HLDivDE	; Calculate Row
		POP	BC
		LD	B,L		; save row number
		POP	DE
		ADD	HL,DE		; Calculate column * number of rows + row
		; HL = Entry number
		RET

GetEntry:
; Input 
;     IX = Pointer to file enry table
;     HL =  X
;     DE =  Y
; Return:
;    IX = Pointer to selected entry
;     C = Column number
;     B = Row number
;     Carry set then it's a valid entry
		CALL	GetEntryNumber
		LD	DE,(fileInfo.entryOffset)
		ADD	HL,DE	     ; entry number + entry offset
		PUSH	HL
		LD	DE,(fileInfo.totalEntries)     
		OR	A,A
		SBC	HL,DE	     ; Entry number - total number of entries
		POP	HL		
		RET	NC	     ; Invalid enry
			
		PUSH	BC	
		LD	DE,FILE_ENTRY
		CALL	Math.HLMulDE ; Calculate offset in entry table
		EX	DE,HL
		ADD	IX,DE	    
		SCF	   	     ; Return Valid entry
		POP	BC
		RET

CalcPosSelectedFile:
; Input
;     C = Column number
;     B = Row number
; Returns
;   IX  = pos x
;   IY  = pos y

		LD	L,C
		LD	H,0
		LD	DE,COLUMN_SIZE
		PUSH	BC
		CALL	Math.HLMulDE	; Calculate X
		LD	DE,(fileView.left)
		ADD	HL,DE
		PUSH	HL
		POP	IX
		POP	BC
		
		LD	L,B
		LD	H,0
		LD	DE,ROW_SIZE
		PUSH	BC
		CALL	Math.HLMulDE	; Calculate Y
		LD	DE,(fileView.top)
		ADD	HL,DE
		PUSH	HL
		POP	IY
		POP	BC
		RET
		
CalculateViewOffset:
; Input :
;          HL = Pointer to file entry buffer
; Returns:
;         BC = Nummber of entries left
;         HL = Pointer to start op entry table to view

		PUSH	HL
   		LD	HL,(fileInfo.entryOffset)
                PUSH	HL
               	LD	DE,FILE_ENTRY
		CALL	Math.HLMulDE ; Calculate offset in entry table
                EX	DE,HL	     ; DE = offset in entry table
                
                LD	HL,(fileInfo.totalEntries)
                POP 	BC       ; (fileInfo.entryOffset)
                OR	A,A
                SBC	HL,BC  
                LD	B,H
                LD	C,L      ; BC = totalEntries - entryOffset 
                POP	HL
                ADD	HL,DE	 ; Add offset to base address entry table
		RET


;----------------------------------------------------------------------------;
; Draw dialog window functions                                               ;
;----------------------------------------------------------------------------;

DrawWindow:
; IX = left
; IY = top
; HL = width
; BC = height
                DI
		CALL	Window.DrawWindow
		CALL	DrawTitleBar
		CALL	DrawScrollBar
		CALL	DrawLoadSaveButton
		EI
		LD	DE,52
		ADD	IY,DE ; Top + 52
		LD	DE,8
		ADD	IX,DE ; Left + 4
		LD	HL,COLUMN_SIZE*COLUMNS+4
		LD	BC,14*14+4
		DI
		CALL	Window.DrawInnerLine
		EI
		CALL	DrawDriveButtons
		CALL	ClearFileEditBox
		RET

ClearFileEditBox:
		DI
		PUSH	AF
		PUSH	BC
		PUSH	DE
		PUSH	HL
		LD	DE,FONT_COLOR_BACK_GND
		LD	HL,.CLEAR_EDIT
		CALL	G9k.DrawFilledBox
		G9kCmdWait
		LD      HL,FONT_COLOR
               	CALL    G9k.SetCmdColor
		EI	
		POP	HL
		POP	DE
		POP	BC
		POP	AF
		RET
		
.CLEAR_EDIT	G9K_BOX		FV_FILE_EDIT_LEFT,FV_FILE_EDIT_TOP,12*8,14
            	

PrintPathName:
		
		CALL	GetFullPathName
		LD      HL,FONT_COLOR_BACK_GND
		G9kCmdWait
		DI
               	CALL 	G9k.SetCmdBackColor
                LD      HL,FONT_COLOR
               	CALL    G9k.SetCmdColor
               	EI
	
		LD	DE,FileDialog.FULL_PATH_NAME
		LD	IX,FILE_VIEW_LEFT+24
                LD	IY,FILE_VIEW_TOP+6
                DI
		CALL	G9k.PrintString
                EI
                RET
                
DrawTitleBar:
; Make generic!!! and place it in windows.asm

		PUSH	IX
		PUSH	IY
		PUSH	HL
		PUSH	BC
		
		LD	IX,FILE_VIEW_LEFT+3
		LD	IY,FILE_VIEW_TOP+3
		LD	HL,.titleLeft
		CALL	G9k.CopyXYToRegisterXY

		LD	IX,FILE_VIEW_LEFT+FILE_VIEW_WIDTH-3-22
		LD	HL,.titleRight
		CALL	G9k.CopyXYToRegisterXY
		
		LD	IY,FILE_VIEW_TOP+3
		LD	(.param.top),IY
		LD	HL,.param
		LD	DE,0x3E3E
		G9kCmdWait
		G9kWriteReg G9K_ARG,0
		CALL	G9k.DrawLine
		
		INC	IY
		INC	IY
		LD	(.param.top),IY
		LD	HL,.param
		LD	DE,0x3C3C
		CALL	G9k.DrawLine
		
		LD	DE,15
		ADD	IY,DE
		LD	(.param.top),IY
		LD	HL,.param
		LD	DE,0x3E3E
		CALL	G9k.DrawLine
		INC	IY
		LD	(.param.top),IY
		LD	HL,.param
		LD	DE,0x0606
		CALL	G9k.DrawLine
		INC	IY
		LD	(.param.top),IY
		LD	HL,.param
		LD	DE,0x3E3E  
		CALL	G9k.DrawLine
		
		G9kCmdWait
		G9kWriteReg G9K_LOP,G9K_LOP_WCSC|G9K_LOP_TP
		
		LD	IX,FV_CLOSE_LEFT
		LD	IY,FV_CLOSE_TOP+6
		LD	HL,.closeButton
		CALL	G9k.CopyXYToRegisterXY
			
		POP	BC
		POP	HL
		POP	IY
		POP	IX
		
		G9kCmdWait
		G9kWriteReg G9K_LOP,G9K_LOP_WCSC
					
		RET  
		  
.param		G9K_BOX FILE_VIEW_LEFT+22,0,FILE_VIEW_WIDTH-2-22-23,0
		            
.titleRight	G9K_BOX 0,848+64,23,20              
.titleLeft	G9K_BOX 26,848+64,21,20   
.closeButton	G9K_BOX 0,848+48,5,6

CalculatePixelsPerColumn:
	; Calculate max number of colmns
		PUSH	AF
		PUSH	BC
		PUSH	DE
		PUSH	HL
		
	        ; Calculate rows
		LD	HL,(fileInfo.totalEntries)
		LD	DE,ROWS
		CALL	Math.HLDivDE
		; Is remainder 0?
		LD	A,E
		OR	A,D
		JR	Z,.a
		INC	HL	; Total collumns
.a		
		; Less then max columns?
		LD	DE,COLUMNS
		EX	DE,HL
		CALL	Math.CPHLDE
		JR	NC,.less	
		; sub nummer of columns with 2
		DEC	DE
		DEC	DE
		jr	.calcPixels  
.less
		LD	DE,1
		; Calculate pixels per column               
.calcPixels               
                LD	HL,FV_SCROLL_BAR_WIDTH
                CALL	Math.HLDivDE
                LD	A,E
		OR	A,D
		JR	Z,.b
		INC	HL	; Total collumns
.b		
                LD	(fileInfo.pixelsPerColumn),HL
                    
                POP	HL
                POP	DE
                POP	BC
                POP	AF
                RET
UpdateDir:
; HL = Pointer to file entry buffer
		PUSH	HL
		LD	DE,0
		LD 	(fileInfo.pSelectedFile),DE
		
		;LD	A,H
		;OR	A,L
		;JR	Z,.noCopyFileName
		
		;LD	DE,EditFileName.text
		;LD	BC,13
		;LDIR
		;
		
		;CALL	CopyFileName
		
;.noCopyFileName:		
		
                DI
		CALL	PrintPathName
		EI
                LD      DE,(fileInfo.entryTable) ; Pointer to file entry table
		LD      HL,WILD_CARD 
		LD      BC,16384/FILE_ENTRY
                CALL	GetEntries
                LD	BC,(fileInfo.totalEntries)
                POP	HL
                CALL	CalculatePixelsPerColumn
                LD	IX,fileView
                JP	ViewFilesExt

fileView	FILE_VIEW FILE_VIEW_LEFT+10,FILE_VIEW_TOP+54,COLUMN_SIZE*COLUMNS,ROWS*14,COLUMNS,ROWS,FONT_COLOR_BACK_GND,FONT_COLOR	
WILD_CARD:	DB	"*.BLX",0	



ChangeDir:
; DE = Pointer to directory name
		BdosCall  _CHDIR
		LD	HL,(fileInfo.entryTable)
		JP	UpdateDir


;----------------------------------------------------------------------------;
; Scroll bar functions	                                                     ;
;----------------------------------------------------------------------------;
DrawScrollBar:
		PUSH	IX
		PUSH	IY
		PUSH	HL
		PUSH	BC
	
		LD	IX,FV_PREV_COL_LEFT
		LD	IY,FV_PREV_COL_TOP
		LD	HL,.left
		CALL	G9k.CopyXYToRegisterXY
		
		LD	IX,FV_NEXT_COL_LEFT
		LD	IY,FV_NEXT_COL_TOP
		LD	HL,.right
		CALL	G9k.CopyXYToRegisterXY
		
		POP	BC
		POP	HL
		POP	IY
		POP	IX
		RET
		
.leftActive:	G9K_BOX 5,848+48,14,14
.rightActive:   G9K_BOX 33,848+48,14,14
.left:		G9K_BOX 19,848+48,14,14
.right:	        G9K_BOX 47,848+48,14,14

ScrollBar:
		; Calculate selected column
                LD	HL,(CursorData.x)
		LD	BC,-FV_SCROLL_BAR_LEFT
                ADD	HL,BC
		LD	DE,(fileInfo.pixelsPerColumn)
		CALL	Math.HLDivDE
		; HL = collumn number
		; Calculate entry number
		LD	DE,ROWS
	        CALL	Math.HLMulDE
	        LD	DE,(fileInfo.entryOffset)
	        CALL	Math.CPHLDE
	        JP  	Z,Window.GetEvent ; Open.loop	; clicked on the same entry 
	        
	        LD	(fileInfo.entryOffset),HL
	        
		LD	HL,(fileInfo.entryTable)
		CALL	CalculateViewOffset
		LD	IX,fileView
		CALL	ViewFilesExt
		JP  	Window.GetEvent; Open.loop
		

;----------------------------------------------------------------------------;
; Change column functions                                                    ;
;----------------------------------------------------------------------------;

NextColumn:	
		LD	HL,(fileInfo.totalEntries)
		LD	DE,(fileInfo.entryOffset)
		OR	A,A
		SBC	HL,DE	
		LD	DE,ROWS*COLUMNS+1
		CALL	Math.CPHLDE			
		JP  	C,Window.GetEvent ; Open.loop   ; If ((totalEnrties-entryOffset)>=rows*columns) then return
		
		LD	HL,0
		LD	(fileInfo.pSelectedFile),HL
						
		LD	HL,(fileInfo.entryOffset)
		LD	DE,ROWS
		ADD	HL,DE
		LD	(fileInfo.entryOffset),HL
		
		LD	HL,(fileInfo.entryTable)
		CALL	CalculateViewOffset
		LD	IX,fileView
		CALL	ViewFilesExt
		JP 	Window.GetEvent ; Open.loop
		
PreviousColumn:
		LD	HL,(fileInfo.entryOffset)
		LD	A,H
		OR	A,L
		JP  	Z,Window.GetEvent ;Open.loop	; first column is already visable
		
		LD	DE,0
		LD	(fileInfo.pSelectedFile),DE
		
		LD	DE,-ROWS
		ADD	HL,DE
		LD	(fileInfo.entryOffset),HL
		
		LD	HL,(fileInfo.entryTable)
		CALL	CalculateViewOffset
		LD	IX,fileView
		CALL	ViewFilesExt
		JP  	Window.GetEvent 
		
;----------------------------------------------------------------------------;
; Edit	box   	                                                             ;
;----------------------------------------------------------------------------;
EditFileName:
		
.OnEdit:
		LD	HL,(fileInfo.pSelectedFile)
		LD	A,H
		OR	A,L
		JR	Z,.noFileSelected
	
	 	; Don't reset selected file name if current file is edit box
		LD	DE,.text
		CALL	Math.CPHLDE
		JR	Z,.noFileSelected
				
		PUSH	HL
		PUSH	IX
		PUSH	IY
		LD	IX,(fileInfo.selectedFileX)
		LD	IY,(fileInfo.selectedFileY)
		CALL	PrintFileEntry
		POP	IY
		POP	IX
		POP	HL
		

.noFileSelected:		
		LD	DE,.text
		; Set selected file on edit file box
		LD	(fileInfo.pSelectedFile),DE
		
		LD	A,H
		OR	A,L
		JR	Z,.nonSelected	
			
		CALL	CopyFileName
		
		JR      .calcPos
.nonSelected:

		PUSH	DE
		LD	HL,.text
		LD	DE,.text+1
		LD	(HL),0
		LD	BC,12
		LDIR			; Clear string
	        POP	DE
	        
.calcPos:	
		
		CALL	ClearFileEditBox
		LD	DE,.text
            	LD	IX,FV_FILE_EDIT_LEFT
            	LD	IY,FV_FILE_EDIT_TOP
            	DI
            	CALL	G9k.PrintString
		EI
		
		CALL	.CalcCursorPos
		LD	B,8 ; max 8 chars. 
		CALL	.editMenu		
		JP  	Window.GetEvent


.CalcCursorPos:	
; OUPUT DE = Pointer pos in string
;        A = Char number
		LD	DE,.text
		CALL	String.StringLenght ; B = string lenght

		LD	HL,(CursorData.x)
		LD	DE,-FV_FILE_EDIT_LEFT
                ADD	HL,DE
		SRL	HL
		SRL	HL
		SRL	HL
		LD	A,L
		
		; Check if cursor is on the string else put cursor at end of string
		CP	A,B
		JR	C,.notInstring
		LD	A,B
.notInstring:		
		PUSH	AF
		LD	HL,.text
		ADD_HL_A         ; Calculate postion in string
		POP	AF
		EX	DE,HL
		
		RET		
		
.text:		DS	14,0


.editMenu:	
; Input DE = pointer to edit box
;        B = Max char
;        A = current char position
	
		PUSH	AF
		PUSH	BC
		PUSH	DE
		BIOS    KILBUF
		POP	DE
		POP	BC
		POP	AF
		
		LD      C,A
.editLoop:
		PUSH	BC
		PUSH	DE	
		LD	E,0FFh  
		BdosCall  _DIRIO  ; read char from  standard input
		POP	DE
		POP	BC
		OR	A,A
		JR	Z,.checkMouse
		CP	A,CR
		RET	Z	 ; Return on enter pressed
		
		PUSH	DE
		LD	E,A
		LD	A,C
		CP	A,B
		LD	A,E
		POP	DE    
		JR	NC,.checkMouse
		
		PUSH	BC
		PUSH	DE
		LD	E,A
		LD	D,1;   //SUPPRESS_UPPER_CASING
		BdosCall _CHKCHR
		BIT	NOT_A_VALID_CHAR,D
		LD	A,E
		POP	DE
		POP	BC
		JR	NZ,.checkMouse
		
		LD	(DE),A
		INC	DE
		INC	C   ; Inc current char	
		
		PUSH	BC
		PUSH	DE
		LD	DE,.text
            	LD	IX,FV_FILE_EDIT_LEFT
            	LD	IY,FV_FILE_EDIT_TOP
            	DI
            	CALL	G9k.PrintString
		EI
		POP	DE
		POP	BC
	
.checkMouse:
		LD	A,(CursorData.A)
		OR	A,A
		RET	NZ
		JR	.editLoop


CopyFileName:
; HL = pointer to file name
		LD	DE,EditFileName.text
		LD	BC,8
		LDIR
		
		LD	HL,EditFileName.text
		LD	A,"."
		LD	BC,8
.next		
		CPI
		JR	Z,.found
		JP	PE,.next  ; check if end of search	
.found		
		DEC	HL
		LD	(HL),0
		
		LD	A,B
		OR	A,C
		JR	Z,.noClear
		LD	DE,HL
		INC	DE
		LDIR
		
.noClear:		
		LD	HL,EditFileName.text
		LD	(fileInfo.pSelectedFile),HL
		RET
		
;----------------------------------------------------------------------------;
; Load Save functions	                                                     ;
;----------------------------------------------------------------------------;
DrawLoadSaveButton:
		PUSH	IX
		PUSH	IY
		PUSH	HL
		PUSH	BC
		
		LD	A,(fileInfo.mode)
		LD	HL,.loadButton
		OR	A,A
		JR	Z,$+5
		LD	HL,.saveButton				
		LD	IX,FV_LS_BUTTON_LEFT
		LD	IY,FV_LS_BUTTON_TOP
		CALL	G9k.CopyXYToRegisterXY
		
		POP	BC
		POP	HL
		POP	IY
		POP	IX

		RET	
			
.loadButton:	G9K_BOX 64,848,64,15
.saveButton:	G9K_BOX 128,848,64,15		
		


;----------------------------------------------------------------------------;
; Change drive rotuines                                                      ;
;----------------------------------------------------------------------------;
	
ChangeDrive:
		LD	 E,(IX + WINDOW.data)
		BdosCall _SELDSK
	
		LD	 HL,(fileInfo.entryTable)
		CALL	 UpdateDir
		JP  	 Window.GetEvent ; Open.loop	

InitDriveButtons:
		LD	IX,DButtons
		LD	IY,envData.diskInfo
		LD	B,8
		LD	C,0	;Drive Number
.loop:		
		LD	A,(IY + DISK_INFO.deviceType)
		OR	A,A
		JR	Z,.next		; No drive connect jump to next
		LD	HL,ChangeDrive
		LD	(IX + WINDOW.onMouseClick),HL
		LD	(IX + WINDOW.data),C
		LD	(IX + WINDOW.data+1),A ; Store device type
		
		LD	DE,WINDOW
		ADD	IX,DE
.next:		
		LD	DE,DISK_INFO
		ADD	IY,DE	
		INC	C	; Next drive			
		DJNZ	.loop
		RET
		
DrawDriveButtons:
		
		G9kCmdWait
		LD	HL,0
               	CALL    G9k.SetCmdBackColor
                LD	HL,FONT_COLOR
               	CALL    G9k.SetCmdColor
		G9kWriteReg G9K_LOP,G9K_LOP_WCSC+G9K_LOP_TP
		
		LD	IX,DButtons
		LD	B,8
.loop		
		LD	A,(IX+WINDOW.onMouseClick+1)
		OR	A,A
		RET	Z	; No more drives buttons to draw
		
		PUSH	BC
		; Set drive letter text
		LD	A,(IX+WINDOW.data)
		ADD	A,'A'
		LD	DE,.DRIVE_TEXT
		LD	(DE),A
		
		LD	A,(IX + WINDOW.data+1) ; Device type
		DEC	A
		ADD	A,A
		ADD	A,A
		ADD	A,A	; times 8
		LD	HL,iconHDD
		ADD_HL_A
		
		LD	BC,(IX+WINDOW.top)
		LD	IY,BC	
		
		LD	BC,(IX+WINDOW.left)
		PUSH	IX
		LD	IX,BC
		
		DI
		PUSH	HL
		CALL	G9k.PrintString ; Updates Ix coordinate
		POP	HL
		G9kCmdWait
		CALL	G9k.CopyXYToRegisterXY
		EI		
		POP	IX	
		
		LD	DE,WINDOW
		ADD	IX,DE
		POP	BC
		DJNZ	.loop
		G9kWriteReg G9K_LOP,G9K_LOP_WCSC
		RET	
		
.DRIVE_TEXT	DB	"A:",0	
	
iconHDD		G9K_BOX 32,848+32,16,16
iconFDD		G9K_BOX 16,848+32,16,16
iconCD		G9K_BOX 0, 848+32,16,16
iconRAM		G9K_BOX 48,848+32,16,16
		
;----------------------------------------------------------------------------;
; Main File Dialog                                                           ;
;----------------------------------------------------------------------------;
	
			
Open:
; HL = Pointer to entry table
; A  = Save or load dialog , 1=save,0 = load		
		LD	(fileInfo.mode),A
		LD	DE,0
		LD	(fileInfo.pSelectedFile),DE
		CALL	SetDiskErrorAutoAbort
                DI
                G9kWriteReg G9K_ARG,0                    
                EI
                
		LD	(fileInfo.entryTable),HL
		PUSH	HL
		LD	IX,FILE_VIEW_LEFT
		LD	IY,FILE_VIEW_TOP
		LD	HL,FILE_VIEW_WIDTH
		LD	BC,FILE_VIEW_HEIGHT
		CALL	DrawWindow
		POP	HL

		CALL	UpdateDir

		LD	    HL,fileOpenObjectList
		CALL        Window.SetObjectList
		JP	    Window.GetEvent

fileOpenObjectList: DB	  15
	    WINDOW   FV_CLOSE_LEFT,FV_CLOSE_LEFT+FV_CLOSE_WIDTH,FV_CLOSE_TOP,FV_CLOSE_TOP+FV_CLOSE_HEIGHT,CloseDialog,0,0,0,0,0
	    WINDOW   FV_SELECT_LEFT,FV_SELECT_LEFT+FV_SELECT_WIDTH,FV_SELECT_TOP,FV_SELECT_TOP+FV_SELECT_HEIGHT,OpenObject,0,0,0,0,0
	    WINDOW   FV_PREV_COL_LEFT,FV_PREV_COL_LEFT+FV_PREV_COL_WIDTH,FV_PREV_COL_TOP,FV_PREV_COL_TOP+FV_PREV_COL_HEIGHT,PreviousColumn,0,0,0,0,0
	    WINDOW   FV_NEXT_COL_LEFT,FV_NEXT_COL_LEFT+FV_NEXT_COL_WIDTH,FV_NEXT_COL_TOP,FV_NEXT_COL_TOP+FV_NEXT_COL_HEIGHT,NextColumn,0,0,0,0,0
DButtons:   WINDOW   FV_DRIVE_LEFT+FV_DRIVE_NEXT*0,FV_DRIVE_LEFT+FV_DRIVE_NEXT*0+FV_DRIVE_WIDTH,FV_DRIVE_TOP,FV_DRIVE_TOP+FV_DRIVE_HEIGHT,0,0,0,0,0,0
	    WINDOW   FV_DRIVE_LEFT+FV_DRIVE_NEXT*1,FV_DRIVE_LEFT+FV_DRIVE_NEXT*1+FV_DRIVE_WIDTH,FV_DRIVE_TOP,FV_DRIVE_TOP+FV_DRIVE_HEIGHT,0,0,0,0,0,0
	    WINDOW   FV_DRIVE_LEFT+FV_DRIVE_NEXT*2,FV_DRIVE_LEFT+FV_DRIVE_NEXT*2+FV_DRIVE_WIDTH,FV_DRIVE_TOP,FV_DRIVE_TOP+FV_DRIVE_HEIGHT,0,0,0,0,0,0
	    WINDOW   FV_DRIVE_LEFT+FV_DRIVE_NEXT*3,FV_DRIVE_LEFT+FV_DRIVE_NEXT*3+FV_DRIVE_WIDTH,FV_DRIVE_TOP,FV_DRIVE_TOP+FV_DRIVE_HEIGHT,0,0,0,0,0,0
	    WINDOW   FV_DRIVE_LEFT+FV_DRIVE_NEXT*4,FV_DRIVE_LEFT+FV_DRIVE_NEXT*4+FV_DRIVE_WIDTH,FV_DRIVE_TOP,FV_DRIVE_TOP+FV_DRIVE_HEIGHT,0,0,0,0,0,0
	    WINDOW   FV_DRIVE_LEFT+FV_DRIVE_NEXT*5,FV_DRIVE_LEFT+FV_DRIVE_NEXT*5+FV_DRIVE_WIDTH,FV_DRIVE_TOP,FV_DRIVE_TOP+FV_DRIVE_HEIGHT,0,0,0,0,0,0
	    WINDOW   FV_DRIVE_LEFT+FV_DRIVE_NEXT*6,FV_DRIVE_LEFT+FV_DRIVE_NEXT*6+FV_DRIVE_WIDTH,FV_DRIVE_TOP,FV_DRIVE_TOP+FV_DRIVE_HEIGHT,0,0,0,0,0,0
	    WINDOW   FV_DRIVE_LEFT+FV_DRIVE_NEXT*7,FV_DRIVE_LEFT+FV_DRIVE_NEXT*7+FV_DRIVE_WIDTH,FV_DRIVE_TOP,FV_DRIVE_TOP+FV_DRIVE_HEIGHT,0,0,0,0,0,0
    	    WINDOW   FV_LS_BUTTON_LEFT,FV_LS_BUTTON_LEFT+FV_LS_BUTTON_WIDTH,FV_LS_BUTTON_TOP,FV_LS_BUTTON_TOP+FV_LS_BUTTON_HEIGHT,OpenFile,0,0,0,0,0	
    	    WINDOW   FV_SCROLL_BAR_LEFT,FV_SCROLL_BAR_LEFT+FV_SCROLL_BAR_WIDTH,FV_SCROLL_BAR_TOP,FV_SCROLL_BAR_TOP+FV_SCROLL_BAR_HEIGHT,ScrollBar,0,0,0,0,0
    	    WINDOW   FV_FILE_EDIT_LEFT,FV_FILE_EDIT_LEFT+FV_FILE_EDIT_WIDTH,FV_FILE_EDIT_TOP,FV_FILE_EDIT_TOP+FV_FILE_EDIT_HEIGHT,EditFileName.OnEdit,0,0,0,0,0	
    
BOX_SETTINGS    	DW      0,0
               		DW      512,424

OpenObject:
		LD  	IX,(fileInfo.entryTable)
                LD	HL,(CursorData.y)
		LD	BC,-FV_SELECT_TOP		;84
                ADD	HL,BC
                EX	DE,HL

                LD	HL,(CursorData.x)
		LD	BC,-FV_SELECT_LEFT		;42
                ADD	HL,BC
	        CALL	GetEntry
		JP	NC,Window.GetEvent; Open.loop	; Return if entry is not valid

		DI
                LD      HL,HIGHLIGHT_COLOR
		G9kCmdWait
               	CALL 	G9k.SetCmdBackColor
                LD      HL,FONT_COLOR_BACK_GND
               	CALL    G9k.SetCmdColor
		EI

                PUSH	IX
		CALL	CalcPosSelectedFile
		
		POP	HL  ; Pointer to file name
		LD	A,(HL)
		INC	HL
		OR	A,A
		PUSH	AF
		PUSH	HL
		CALL	PrintEntry
		POP	DE
		POP	AF
		JP	Z,SelectFile	; Entry is file

		CALL	ChangeDir
		
           	JP  Window.GetEvent; Open.loop

SelectFile:
		G9kCmdWait
		DI
                LD      HL,FONT_COLOR_BACK_GND
               	CALL 	G9k.SetCmdBackColor
                LD      HL,FONT_COLOR
               	CALL    G9k.SetCmdColor
               	EI
               	
               	
               	; Clear old selected file, if any
		LD	HL,(fileInfo.pSelectedFile)
		PUSH	DE
		LD	DE,EditFileName.text
		CALL	Math.CPHLDE			; Check if selected file is not edit box file
		POP	DE
		JR	Z,.newEntry
		
		LD	A,H
		OR	A,L
		JR	Z,.newEntry
				
		PUSH	HL
		PUSH	IX
		PUSH	IY
		LD	IX,(fileInfo.selectedFileX)
		LD	IY,(fileInfo.selectedFileY)
		CALL	PrintFileEntry
		POP	IY
		POP	IX
		POP	HL
.newEntry:
		; Check if current entry is the same as the previous, then the current entry is unselected
		CALL	Math.CPHLDE
		JR	NZ,.diffenrentEntry
		LD	DE,0
.diffenrentEntry:		
		CALL	ClearFileEditBox
		LD	(fileInfo.pSelectedFile),DE
		LD	(fileInfo.selectedFileX),IX
		LD	(fileInfo.selectedFileY),IY
            	LD	IX,FV_FILE_EDIT_LEFT
            	LD	IY,FV_FILE_EDIT_TOP
            	DI
            	CALL	G9k.PrintString
		EI
               	JP 	Window.GetEvent; Open.loop

ClearSelectedFile:
		LD	HL,0
		LD	(fileInfo.pSelectedFile),HL
		
		RET         
         
               	
OpenFile:
		; Check if there is a selected file
		LD	DE,(fileInfo.pSelectedFile)
		LD	A,D
		OR	A,E
		JP	Z,Window.GetEvent  ; Open.loop
		LD	HL,EditFileName.text
		CALL	Math.CPHLDE
		JR	NZ,.exit
		EX	DE,HL
		CALL	String.StringLenght
		DEC	B
		INC	B  ; If string is empy then return to event loop
		JP	Z,Window.GetEvent  
		
		LD	HL,BLOX_LV_EXT
		LD	BC,5
		LDIR
		LD	DE,EditFileName.text	
.exit		
		PUSH	DE		
		CALL	SetDiskErrorNormalAbort
		POP	DE	
		RET       
		        	
BLOX_LV_EXT	DB	".BLX",0 
 
               	
CloseDialog:
		CALL	SetDiskErrorNormalAbort
		LD	DE,0
	    	RET



		ENDMODULE

		include "../lib/window/window.asm"

		endif
