; Product      : Gfx9000 library
; Version      : 0.6
; Main code by : Marcel delorme
; Suport code  : BiFi
; Bitbust      : Arjan Bakker
	
		include "g9klib.inc"
		MODULE	G9k
		  
;----------------------------------------------------------------------------;
; Gfx9000 general functions                                                  ;
;----------------------------------------------------------------------------;
		
Reset:
; Function :    Resets v9990, Deletes Palette, Sprites off,stops current blit operation, puts V9990 in correct RAM config and disables display
; Input    :    None
; Output   :    None
; Notes    :    Doesn't change current adjust
; Modifies : 	A,B

                G9kReadReg G9K_DISPLAY_ADJUST + G9K_DIS_INC_READ
                PUSH    AF      ; Save adjust value

                ; Set reset state
                LD      A,G9K_SYS_CTRL_SRS
                OUT     (G9K_SYS_CTRL),A
                ; Clear reset state
                XOR     A,A
                OUT     (G9K_SYS_CTRL),A

                POP     AF
                OUT     (G9K_REG_DATA),A        	; Restore adjust value

                G9kWriteReg G9K_OPCODE,G9K_OPCODE_STOP
                G9kWriteReg G9K_CTRL,G9K_CTRL_DIS_SPD+G9K_CTRL_VRAM512

                ; Clear current palette
                G9kWriteReg G9K_PALETTE_PTR,0       ; A becomes 0
                LD      B,192
                      OUT     (G9K_PALETTE),A
                      DJNZ    $-2
                OUT (G9K_OUTPUT_CTRL),A    	   ; Set output GFX9000
                RET


SetScreenMode:
; A = Mode
; B = Bit per dot
; C = Image size
; D = Interlace
; E = Palette control register

                LD      L,A
                ADD     A,A
                ADD     A,L     ; A  = A * 3
                LD      HL,G9K_MODE_TABLE
                ADD_HL_A
                LD      A,G9K_SCREEN_MODE0
                OUT     (G9K_REG_SELECT),A
                LD      A,(HL)  ; Get fixed settings for mode reg 6
                INC     HL
                OR      A,B     ; Set bits per dot
                OR      A,C     ; Set image size
                OUT     (G9K_REG_DATA),A
                LD      A,(HL)  ; Get fixed settings for mode reg 7
                INC     HL
                DEC     D
                INC     D       ; Is d 0?
                JR      Z,$+4
                     OR    A,G9K_SCR1_EO+G9K_SCR1_IL
                OUT     (G9K_REG_DATA),A
                LD      A,(HL)
                OUT     (G9K_SYS_CTRL),A
                G9kWriteReg G9K_PALETTE_CTRL,e
                RET

G9K_MODE_TABLE
                ; Pattern mode 1      (P1)
                DB      G9K_SCR0_P1+G9K_SCR0_DTCLK4
                DB      0
                DB      G9K_SYS_CTRL_XTAL
                ; Pattern mode 2      (P2)
                DB      G9K_SCR0_P2+G9K_SCR0_DTCLK4
                DB      0
                DB      G9K_SYS_CTRL_XTAL
                ; Bitmap 256 * 212    (B1)
                DB      G9K_SCR0_BITMAP+G9K_SCR0_DTCLK4
                DB      0
                DB      G9K_SYS_CTRL_XTAL
                ; Bitmap 384 * 240    (B2)
                DB      G9K_SCR0_BITMAP+G9K_SCR0_DTCLK2
                DB      0
                DB      G9K_SYS_CTRL_MCKIN
                ; Bitmap 512 * 212    (B3)
                DB      G9K_SCR0_BITMAP+G9K_SCR0_DTCLK2
                DB      0
                DB      G9K_SYS_CTRL_XTAL
                ; Bitmap 768 * 212    (B4)
                DB      G9K_SCR0_BITMAP+G9K_SCR0_DTCLK
                DB      0
                DB      G9K_SYS_CTRL_MCKIN
                ; Bitmap 640 * 400    (B5)
                DB      G9K_SCR0_BITMAP+G9K_SCR0_DTCLK
                DB      G9K_SCR1_HSCN
                DB      G9K_SYS_CTRL_XTAL
                ; Bitmap 640 * 480    (B6)
                DB      G9K_SCR0_BITMAP+G9K_SCR0_DTCLK
                DB      G9K_SCR1_HSCN+G9K_SCR1_C25M
                DB      G9K_SYS_CTRL_XTAL
                ; Bitmap 1024 * 212   (B7) (undocumented mode)
                DB      G9K_SCR0_BITMAP+G9K_SCR0_DTCLK
                DB      0
                DB      G9K_SYS_CTRL_XTAL


SetAdjust:
; Function: Adjustment of Gfx9000 display 
;                                                   L/T   C   R/B
; Input   :      BC =>   B -> Horizontal adjustment (-7 -> 0 -> 8)
;                        C -> Vertical adjustment   (-7 -> 0 -> 8)
; Modifies:      AF, BC
;
		LD	A,B
		ADD	A,7
		CP	16
		CCF
		RET	C	; ERROR IN HORIZONTAL ADJUST
		SUB	7
		NEG
		AND	15
		LD	B,A
		LD	A,C
		ADD	A,7
		CP	16
		CCF
		RET	C	; ERROR IN VERTICAL ADJUST
		SUB	7
		NEG
		AND	15
		RLCA
		RLCA
		RLCA
		RLCA
		OR	B
		LD	B,A
		G9kWriteReg G9K_DISPLAY_ADJUST,B
		RET

SetVramWrite:
; Function :    Sets Vram write address
; Input    : 	E:HL VRAM address
; Output   : 	none
; Modifies : 	A,C

                 LD      A,G9K_WRITE_ADDR
                 OUT     (G9K_REG_SELECT),A
                 LD      C,G9K_REG_DATA
                 OUT     (C),L
                 OUT     (C),H
                 OUT     (C),E
                 RET

SetVramRead:
; Function :    Sets Vram read address
; Input    : 	E:HL VRAM address
; Output   : 	none
; Modifies : 	A,C

                 LD      A,G9K_READ_ADDR
                 OUT     (G9K_REG_SELECT),A
                 LD      C,G9K_REG_DATA
                 OUT     (C),L
                 OUT     (C),H
                 OUT     (C),E
                 RET

Detect:
; Input    :    none
; Output   :    Z=detected , NZ=not detected
; Modifies :  A,C,E,H,L,F
; Works but could be saver
                  LD      E,0
                  LD	  H,E
                  LD      L,E
                  CALL    SetVramWrite
                  LD      A,0A2h
                  OUT     (G9K_VRAM),A
                  LD      E,0
                  LD	  H,E
                  LD      L,E
                  CALL    SetVramRead
                  IN      A,(G9K_VRAM)
                  CP      A,0A2h
                  RET


DisplayEnable:
; Modifies A
                  G9kReadReg G9K_CTRL+G9K_DIS_INC_READ
                  OR      A,G9K_CTRL_DISP
                  OUT     (G9K_REG_DATA),A
                  RET

DisplayDisable:
; Modifies A
                  G9kReadReg G9K_CTRL+G9K_DIS_INC_READ
                  AND     A,255-G9K_CTRL_DISP
                  OUT     (G9K_REG_DATA),A
                  RET

SpritesEnable:
                  G9kReadReg G9K_CTRL+G9K_DIS_INC_READ
                  AND     A,255-G9K_CTRL_DIS_SPD
                  OUT     (G9K_REG_DATA),A
                  RET

SpritesDisable:
                  G9kReadReg G9K_CTRL+G9K_DIS_INC_READ
                  OR      A,G9K_CTRL_DIS_SPD
                  OUT     (G9K_REG_DATA),A
                  RET        

SetScrollX:
; Set Horizontal scroll
; In:         HL =>        Left X-position
; Uses:       AF, HL
		LD	A,G9K_SCROLL_LOW_X
		OUT	(G9K_REG_SELECT),a
		LD	A,L
		AND	A,7
		OUT     (G9K_REG_DATA),A
		LD	A,L
		SRL	H
		RRA
		SRL	H
		RRA
		SRL	H
		RRA
		OUT     (G9K_REG_DATA),A
		RET

SetScrollY:
; Function :    Sets Scroll Y 
; Input    : 	HL = Y
; Output   : 	none
; Modifies : 	A
		G9kWriteReg G9K_SCROLL_LOW_Y,L
		LD	A,(SCROLL_MODE)
		OR	A,H
		OUT	(G9K_REG_DATA),A		
		RET

		IFNDEF G9K_DISABLE_PATTERN

SetScrollXB:
; Set Horizontal scroll
; In:         HL =>        Left X-position
; Uses:       AF, BC, HL
		LD	A,G9K_SCROLL_LOW_X_B
		OUT	(G9K_REG_SELECT),a
		LD	A,L
		AND	A,7
		OUT     (G9K_REG_DATA),A
		LD	A,L
		SRL	H
		RRA
		SRL	H
		RRA
		SRL	H
		RRA
		OUT     (G9K_REG_DATA),A
		RET

		
SetScrollYB:
; Function :    Sets Scroll Y Layer B
; Input    : 	HL = Y
; Output   : 	none
; Modifies : 	A
		G9kWriteReg G9K_SCROLL_LOW_Y_B,L
		LD	A,H
		OUT	(G9K_REG_DATA),A		
		RET		
		
		ENDIF

SetScrollMode:
; Vertical Scroll mode of Gfx9000
;
; In:         AF =>   A -> 0 = Roll by image space size
;                          1 = Roll by 256 lines
;                          2 = Roll by 512 lines
; Uses:       AF, B
		AND	A,3
		RRCA
		RRCA
		LD	(SCROLL_MODE),A
		RET
		
SCROLL_MODE	DB	0
		
		

SetBackDropColor:
; Change back drop color on Gfx9000 
; In:         AF =>   A -> Back drop color
; Uses:       AF
;
		PUSH	AF
		LD	A,G9K_BACK_DROP_COLOR
		OUT	(G9K_REG_SELECT),A
		POP	AF
		AND	A,63
		OUT	(G9K_REG_DATA),A
		RET
                  
;----------------------------------------------------------------------------;
; Gfx9000 pallete functions                                                  ;
;----------------------------------------------------------------------------;
WritePalette:
 ;  HL=pointer to palette data
 ;   C=Palette pointer offset
 ;   B=Number of bytes to write
 ;  Modifies : AF,BC,HL
		G9kWriteReg G9K_PALETTE_PTR,C  
		LD	C,G9K_PALETTE     
		OTIR
		RET

ReadPalette:
 ;  HL=pointer to buffer wherin palette data is stored
 ;   C=Palette pointer offset
 ;   B=Number of bytes to read
 ;
		G9kWriteReg G9K_PALETTE_PTR,C    
		LD	C,G9K_PALETTE      
		INIR
		RET

;----------------------------------------------------------------------------;
; Gfx9000 blitter functions                                                  ;
;----------------------------------------------------------------------------;

DrawFilledBox:
; HL = pointer to parameters (format: left,top,width,height)
; DE = Color
		LD      A,G9K_DS_X
		OUT     (G9K_REG_SELECT),A
		LD      BC,8*256+G9K_REG_DATA
		G9kCmdWait
		OTIR
		LD      A,G9K_FC
		OUT     (G9K_REG_SELECT),A
		OUT     (C),E
		OUT     (C),D   ; Set color
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LMMV
		RET
		
DrawLine:	
; HL = pointer to parameters
; DE = Color	
		LD      A,G9K_DS_X
		OUT     (G9K_REG_SELECT),A
		LD      BC,8*256+G9K_REG_DATA
		G9kCmdWait
		OTIR
		LD      A,G9K_FC
		OUT     (G9K_REG_SELECT),A
		OUT     (C),E
		OUT     (C),D   ; Set color
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LINE	
		RET	
DrawBox:		
; HL = pointer to parameters (format: left,top,width,height)
		PUSH	HL
		LD      A,G9K_ARG
		OUT     (G9K_REG_SELECT),A
		LD      BC,6*256+G9K_REG_DATA
		G9kCmdWait
		XOR	A,A
		OUT	(G9K_REG_DATA),A  ; Clear argument register
		
		LD      A,G9K_DS_X
		OUT     (G9K_REG_SELECT),A
		OTIR	
		XOR	A,A
		OUT	(G9K_REG_DATA),A
		OUT	(G9K_REG_DATA),A  ; Write MI
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LINE	; Draw upper line
		POP	HL
				
		
		PUSH	HL
		LD      A,G9K_ARG
		OUT     (G9K_REG_SELECT),A
		LD	B,4	
		G9kCmdWait
		LD	A,G9K_ARG_MAJ
		OUT	(G9K_REG_DATA),A  

		LD      A,G9K_DS_X
		OUT     (G9K_REG_SELECT),A
		OTIR
		
		INC	HL
		INC	HL
		OUTI
		OUTI
		; MI stays the same
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LINE	; Draw left line
		POP	HL
		
		
		PUSH	HL
		LD      A,G9K_ARG
		OUT     (G9K_REG_SELECT),A	
		G9kCmdWait
		
		XOR	A,A
		OUT	(G9K_REG_DATA),A  ; Clear argument register
		
		LD	A,G9K_DS_X
		OUT	(G9K_REG_SELECT),A  
		
		OUTI
		OUTI	; Write left 
					
		LD	C,(HL)
		INC	HL
		LD	B,(HL)
		INC	HL	; top
		
		LD	E,(HL)
		INC	HL
		LD	D,(HL)
		INC	HL	; Width
				
		LD	A,(HL)
		INC	HL
		LD	H,(HL)  ; Height
		LD	L,A	
		ADD	HL,BC	; Calc top+height
		LD	C,G9K_REG_DATA
	
		OUT     (C),L
		OUT     (C),H   ; Write Y
		
		OUT     (C),E
		OUT     (C),D   ; Write MJ
		
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LINE	; Draw bottom line
		POP	HL   
		
		LD      A,G9K_ARG
		OUT     (G9K_REG_SELECT),A		
		;DE = width
		LD	A,(HL)
		INC	HL
		PUSH	HL
		LD	H,(HL)
		LD	L,A   ; Left
		ADD	HL,DE ; Left+width
		G9kCmdWait
	
		LD	A,G9K_ARG_MAJ
		OUT	(G9K_REG_DATA),A  
		
		LD      A,G9K_DS_X
		OUT     (G9K_REG_SELECT),A
		
		OUT     (C),L
		OUT     (C),H   ; Write X
		POP	HL
		INC	HL
		OUTI
		OUTI	        ; Write Y
		INC	HL
		INC	HL	
		OUTI
		OUTI		; Height (MJ)	
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LINE	; Draw right line
		RET	
		
		
SetupCopyRamToXY:
; Input   : HL = pointer to parameters (format: left,top,width,height)
; Modifies: A,BC,HL
		LD      A,G9K_DS_X
		OUT     (G9K_REG_SELECT),A
		LD      BC,8*256+G9K_REG_DATA
		G9kCmdWait
		OTIR
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LMMC
		RET

CopyRamToXY:
; HL = Pointer to data
; BC = Bytes to copy
		LD      A,B
		LD      B,C
		INC     B
		DEC     B
		JR      Z,$+3
                      INC    A
                LD      C,G9K_CMD_DATA
                      OTIR
                      DEC     A
                      JP   NZ,$-3
                RET

SetupCopyXYToRam:
; HL = pointer to parameters (format: left,top,width,height)
		LD      A,G9K_SC_X
		OUT     (G9K_REG_SELECT),A
		LD      BC,4*256+G9K_REG_DATA
		G9kCmdWait
		OTIR
		LD      B,4
		LD      A,G9K_NX
		OUT     (G9K_REG_SELECT),A
		OTIR
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LMCM
		RET

CopyXYToRam:
; HL = Pointer to data
; BC = Bytes to copy
		LD      A,B
		LD      B,C
		INC     B
		DEC     B
		JR      Z,$+3
      		  INC    A
		LD      C,G9K_CMD_DATA
		  INIR
		  DEC     A
       		  JP   NZ,$-3
   		RET

CopyXYToXY:
; HL = Pointer to data  (format: SourceX,SourceY,DestX,DestY,Width,height)
		
		LD      A,G9K_SC_X
		OUT     (G9K_REG_SELECT),A
		LD      BC,12*256+G9K_REG_DATA
		G9kCmdWait
		OTIR
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LMMM
		RET

CopyXYToRegisterXY
; HL = Pointer to G9B_BOX struct
; IX = Dest X
; IY = Dest Y
		
		LD      A,G9K_SC_X
		OUT     (G9K_REG_SELECT),A
		LD      C,G9K_REG_DATA
		G9kCmdWait
		OUTI
		OUTI	; SourceX
		OUTI
		OUTI	; SourceY
		LD	A,IXL
		OUT	(G9K_REG_DATA),A
		LD	A,IXH
		OUT	(G9K_REG_DATA),A; DestX
		LD	A,IYL
		OUT	(G9K_REG_DATA),A
		LD	A,IYH
		OUT	(G9K_REG_DATA),A; DestY
		OUTI
		OUTI	; Width 
		OUTI
		OUTI	; Height
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_LMMM
		RET

CopyVramToXY:
; HL = Pointer to data  (format: SourceAddress,DestX,DestY,Width,height)

		LD      A,G9K_SC_X
		OUT     (G9K_REG_SELECT),A
		LD      BC,11*256+G9K_REG_DATA
		G9kCmdWait
		OUTI
		OUT     (G9K_REG_DATA),A    ; dummy write
		OTIR
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_BMXL
		RET

CopyXYToVram:
; HL = Pointer to data  (format: SourceX,SourceY,DestAddress,Width,height)
; Modifies AF,HL,BC
		LD      A,G9K_SC_X
		OUT     (G9K_REG_SELECT),A
		LD      BC,5*256+G9K_REG_DATA
		G9kCmdWait
		OTIR
		OUT     (G9K_REG_DATA),A    ; dummy write
		LD      B,6
		OTIR
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_BMLX
		RET


SetPoint:
; HL = X
; DE = Y
; Under Construction
		LD      A,G9K_SC_X
		OUT     (G9K_REG_SELECT),A
		LD      C,G9K_REG_DATA
		OUT     (C),L
		OUT     (C),H
		OUT     (C),E
		OUT     (C),D
		
		G9kCmdWait
		RET


SetCmdWriteMask:
; HL = mask value
		G9kWriteReg G9K_WRITE_MASK,L
		LD      A,H
		OUT     (G9K_REG_DATA),A
		RET
SetCmdColor:
; HL = color value
		G9kWriteReg G9K_FC,L
		LD      A,H
		OUT     (G9K_REG_DATA),A
		RET
		
SetCmdBackColor:		
; HL = color value
		G9kWriteReg G9K_BC,L
		LD      A,H
		OUT     (G9K_REG_DATA),A
		RET
		

CopyRamToVram:
; Function :    Data transfer from CPU to VRAM 
; Input    :    HL = pointer to buffer, BC=number of bytes to send
; Output   : 	none
; Modifies :    A,B,C,HL
		LD      A,B
                LD      B,C
                LD      C,G9K_VRAM
                INC     B
                DEC     B
                JR      Z,.a
                INC     A
.a
                OTIR
                DEC     A
                JP      NZ,.a            
		RET
		

;----------------------------------------------------------------------------;
; Gfx9000 font functions                                                     ;
;----------------------------------------------------------------------------;
		IFNDEF G9K_DISABLE_VFF
OpenVff:
; Input 	  A = ram or vram.  0 = load data in vram, 1 = load font data in ram
;		 DE = Pointer to Font file name
;                IX = Pointer to VFF object
                
                LD	(IX+VFF_OBJECT.ramFont),A
		PUSH	IX
		XOR	A,A	
		CALL	File.FileOpen
		POP	IX
		RET	NZ		; Error opening
		
		PUSH	IX
		LD	HL,3		; ID
		LD	DE,VFF_DATA_ID
		CALL	File.FileRead     ; Load Id
		POP	IX
		RET	NZ		; Error reading

		PUSH	IX		
		PUSH	BC		; Save file handle
		LD	B,3
		LD	HL,VFF_DATA_ID
		LD	DE,VFF_ID
		CALL	String.StringCompare
		LD	A,_NOVFF
		POP	BC
		POP	IX
		RET	NZ		; Not a VFF file
		LD	(IX),B		; Save file handle
		LD 	DE,IX
		INC	DE
		INC	DE
		LD	HL,21		; Load header
		JP	File.FileRead
		
VFF_ID      	DB      "VFF"
VFF_DATA_ID	DS	3,0
		
LoadFont:
; Function	: Loads VFF font data 
; Input         : IX = Pointer to VFF object
;  	        : IY = Pointer to font offset table, if it's zero it's not loaded
; 	        : HL = Pointer to buffer for loading, or if it's a ram font the destination address
;               : BC = Vram low word font data offset. If it's a ram font the pointer for buffer loading is used as font data offset  
; Output        : Z = succeded NZ= not succeded, A = error number
		
		LD	A,(IX+VFF_OBJECT.ramFont)	
		OR	A,A
		JR	Z,.noRamFont
		LD	BC,HL	
.noRamFont		
		PUSH	IX
		PUSH	HL
		LD	(IX+VFF_OBJECT.vramOffset),BC     ; Store Vram font offset
		LD	HL,512                            ; Font offset table size
		LD	B,(IX)			          ; File hanlde
		LD	DE,IY				  ; Pointer to offset table
		LD	(IX+VFF_OBJECT.ptrOffsetTable),DE ; Store pointer to font character offset table
		CALL	File.FileRead    		  ; Load font offset table
		POP	DE          			  ; Pointer to buffer
		POP	IX
		RET	NZ	    			  ; Return if loading failed
		
		LD	HL,(IX+VFF_OBJECT.dataSize)
		PUSH	HL
		PUSH	DE           ; Pointer to buffer
		CALL	File.FileRead
		POP	DE
		POP	HL
		RET	NZ	     ; Loading failed
		
		; If it is ram font then copy to vram is skipped
		LD	A,(IX+VFF_OBJECT.ramFont)
		DEC	A
		RET	Z
		
		PUSH	DE           ; Pointer to buffer
		PUSH	HL           ; data size    
                LD	HL,(IX+VFF_OBJECT.vramOffset)
                LD	E,7	     ; High byte font address always 7
		CALL    SetVramWrite
		POP	BC           ; data size
		POP	HL	     ; Pointer to buffer
		CALL    CopyRamToVram
		XOR	A,A
		RET		

SetFont:
; Input IX = Pointer to VFF object
		PUSH	AF
		PUSH	HL
		PUSH	DE
		
		LD 	A,(IX + VFF_OBJECT.width)
		LD 	(currentFont.width),A
		LD 	A,(IX + VFF_OBJECT.height)
		LD 	(currentFont.height),A

		; Copy pointer to font offset table in print functions
		LD	HL,(IX + VFF_OBJECT.ptrOffsetTable)
		IFNDEF G9K_DISABLE_VFF_VRAM
		LD	(PrintStringVram.fontTablePtr+1),HL
		LD	(PutCharVram.fontTablePtr+1),HL
		ENDIF
		
		IFNDEF G9K_DISABLE_VFF_RAM
		LD	(PrintStringRam.fontTablePtr+1),HL	
		LD	(PutCharRam.fontTablePtr+1),HL
		ENDIF
		
		INC	HL
		INC	HL
		LD	A,(HL)	; offset to second char is also the character size
		LD	(currentFont.charSize),A
			
		LD	A,(IX+VFF_OBJECT.vramOffset+0)
		IFNDEF G9K_DISABLE_VFF_VRAM
		LD	(PrintStringVram.fontVramOffset+1),A
		LD	(PutCharVram.fontVramOffset+1),A
		ENDIF
		
		IFNDEF G9K_DISABLE_VFF_RAM
		LD	(PrintStringRam.fontRamOffset+1),A	
		LD	(PutCharRam.fontRamOffset+1),A
		ENDIF
		
		LD	A,(IX+VFF_OBJECT.vramOffset+1)
		IFNDEF G9K_DISABLE_VFF_VRAM
		LD	(PrintStringVram.fontVramOffset+2),A
		LD	(PutCharVram.fontVramOffset+2),A
		ENDIF	
		
		IFNDEF G9K_DISABLE_VFF_RAM
		LD	(PrintStringRam.fontRamOffset+2),A
		LD	(PutCharRam.fontRamOffset+2),A
		ENDIF
		
		LD	A,(IX+VFF_OBJECT.ramFont)
		IFNDEF G9K_DISABLE_VFF_VRAM
		LD	HL,PrintStringVram
		LD	DE,PutCharVram
		ENDIF
		
		OR	A,A
		JR	Z,.end
		
		IFNDEF G9K_DISABLE_VFF_RAM
		LD	HL,PrintStringRam
		LD	DE,PutCharRam
		ENDIF
.end:				
		LD	(PrintString+1),HL
		LD	(PutChar+1),DE
		
		POP	DE
		POP	HL
		POP	AF
		RET
		
PrintString:	
		JP 	0
PutChar:	
		JP 	0	
	
		IFNDEF G9K_DISABLE_VFF_VRAM
PrintStringVram:	
; Function	: Print ASCIIZ String
; Input 	: IX = X
;  		: IY = Y
;		; DE = Pointer to txt
; Output        : DE = Pointer to byte after term char
;               : IX = modified to x after printed string
; Modifies      ; AF,BC,DE,HL,IX
                LD	(currentFont.x),IX
                LD	(currentFont.y),IY
                    
.loop
		LD	A,G9K_SC_X
		OUT	(G9K_REG_SELECT),A
		LD	A,(DE)
		INC	DE
		OR	A,A	; End of ASCIIZ string
		RET	Z 	
		  
.fontTablePtr	LD	BC,0	;VFF.FONT_TABLE
		LD	L,A
		LD	H,0
		ADD	HL,HL
		ADD	HL,BC

		LD	A,(currentFont.width)	; Next Char
		LD	C,A
		LD	B,0
		ADD	IX,BC		; Precalculation
		
		LD	C,(HL)
		INC	HL
		LD	B,(HL)  ; vram offset of character  
.fontVramOffset LD	HL,0    ; vram font base address offset low word
                ADD	HL,BC   
                
                LD	C,G9K_REG_DATA
        	G9kCmdWait

		OUT	(C),L   ; Vram address Low byte  
		LD	A,0
		OUT	(C),A   ; Looks stuppid but else it doens't work on turbo r             
		OUT	(C),H	; Vram addres Mid byte   
		LD	HL,currentFont.vramAddress
		OUTI            ; Vram high address
		OUTI            ; X low byte
		OUTI            ; X high byte
		OUTI            ; Y low byte
		OUTI            ; Y high byte
		OUTI            ; Width
		LD	A,0
		OUT	(C),A   ; Width High byte always 0
		OUTI            ; Height
		LD	A,0     ; Height High byte always 0
		OUT	(C),A   ; Looks stuppid but else it doens't work in turbo r  
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_CMMM
		LD	(currentFont.x),IX
		JP	.loop

PutCharVram:	
; Function	: Print a character
; Input 	: E  = Character
; Output        ; IX = modified to x after printed character
; Modifies      ; AF,BC,DE,HL,IX
                    		
                            	  
.fontTablePtr	LD	BC,0	
		LD	L,A
		LD	H,0
		ADD	HL,HL
		ADD	HL,BC
		
		LD	A,G9K_SC_X
		OUT	(G9K_REG_SELECT),A 

		LD	A,(currentFont.width)	; Next Char
		LD	C,A
		LD	B,0
		ADD	IX,BC		; Precalculation
		
		LD	C,(HL)
		INC	HL
		LD	B,(HL)  ; vram offset of character  
.fontVramOffset LD	HL,0    ; vram font base address offset low word
                ADD	HL,BC   
                
                LD	C,G9K_REG_DATA
        	G9kCmdWait

		OUT	(C),L    ; Vram address Low byte  
		LD	A,0
		OUT	(C),A   ; Looks stuppid but else it doens't work on turbo r             
		OUT	(C),H	; Vram addres Mid byte   
		LD	HL,currentFont.vramAddress
		OUTI            ; Vram high address
		OUTI            ; X low byte
		OUTI            ; X high byte
		OUTI            ; Y low byte
		OUTI            ; Y high byte
		OUTI            ; Width
		LD	A,0
		OUT	(C),A   ; Width High byte always 0
		OUTI            ; Height
		LD	A,0     ; Height High byte always 0
		OUT	(C),A   ; Looks stuppid but else it doens't work in turbo r  
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_CMMM
		LD	(currentFont.x),IX
		RET
		ENDIF
		
		IFNDEF G9K_DISABLE_VFF_RAM
PrintStringRam:
; Function	: Print ASCIIZ String
; Input 	: IX = X
;  		: IY = Y
;		; DE = Pointer to txt
; Output        : DE = Pointer to byte after term char
;               : IX = modified to x after printed string
; Modifies      ; AF,BC,DE,HL,IX

                LD	(currentFont.x),IX
                LD	(currentFont.y),IY

.loop
		LD	A,G9K_DS_X
		OUT	(G9K_REG_SELECT),A
		LD	A,(DE)
		INC	DE
		OR	A,A	; End of ASCIIZ string
		RET	Z 	
		 
		; Calculate the font data pointer address in ram
.fontTablePtr	LD	BC,0
		LD	L,A
		LD	H,0
		ADD	HL,HL
		ADD	HL,BC

		; Precalculate next character offset
		; This is done here to minimize wasted time on waiting for the v9990 to finish
		LD	A,(currentFont.width)	; Next Char
		LD	C,A
		LD	B,0
		ADD	IX,BC		; Precalculation

		; Get the address of the character data in ram
		LD	C,(HL)
		INC	HL
		LD	B,(HL)  ; ram offset of character  
.fontRamOffset  LD	HL,0    ; ram font base address offset low word
                ADD	HL,BC   
                PUSH	HL	
                LD	C,G9K_REG_DATA
        	G9kCmdWait
		LD	HL,currentFont.x
		OUTI            ; X low byte
		OUTI            ; X high byte
		OUTI            ; Y low byte
		OUTI            ; Y high byte
		OUTI            ; Width
		LD	A,0
		OUT	(C),A   ; Width High byte always 0
		OUTI            ; Height
		LD	A,0     ; Height High byte always 0
		OUT	(C),A   ; Looks stuppid but else it doens't work in turbo r  
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_CMMC
		; Store addres of next X position
		LD	(currentFont.x),IX
		
		; Wait for v9990 to recieve data
		G9kWaitComReady
		
		; Send character data to vdp
		POP	HL	; Pointer to font data in ram
		LD	A,(currentFont.charSize)
		LD	B,A
		LD	C,G9K_CMD_DATA
		OTIR
		JP	.loop
		
PutCharRam:
; Function	: Print a character
; Input 	: A  = Character
; Output        ; IX = modified to x after printed character
; Modifies      ; AF,BC,DE,HL,IX		
		; Calculate the font data pointer address in ram
.fontTablePtr	LD	BC,0
		LD	L,A
		LD	H,0
		ADD	HL,HL
		ADD	HL,BC

		LD	A,G9K_DS_X
		OUT	(G9K_REG_SELECT),A
		 
		; Precalculate next character offset
		; This is done here to minimize wasted time on waiting for the v9990 to finish
		LD	A,(currentFont.width)	; Next Char
		LD	C,A
		LD	B,0
		ADD	IX,BC		; Precalculation

		; Get the address of the character data in ram
		LD	C,(HL)
		INC	HL
		LD	B,(HL)  ; ram offset of character  
.fontRamOffset  LD	HL,0    ; ram font base address offset low word
                ADD	HL,BC   
                PUSH	HL	
                LD	C,G9K_REG_DATA
        	G9kCmdWait
		LD	HL,currentFont.x
		OUTI            ; X low byte
		OUTI            ; X high byte
		OUTI            ; Y low byte
		OUTI            ; Y high byte
		OUTI            ; Width
		LD	A,0
		OUT	(C),A   ; Width High byte always 0
		OUTI            ; Height
		LD	A,0     ; Height High byte always 0
		OUT	(C),A   ; Looks stuppid but else it doens't work in turbo r  
		G9kWriteReg G9K_OPCODE,G9K_OPCODE_CMMC
		; Store addres of next X position
		LD	(currentFont.x),IX
		
		; Wait for v9990 to recieve data
		G9kWaitComReady
		
		; Send character data to vdp
		POP	HL	; Pointer to font data in ram
		LD	A,(currentFont.charSize)
		LD	B,A
		LD	C,G9K_CMD_DATA
		OTIR
		RET
		ENDIF
		
Locate:		
; Set X and Y position for putchar
; Value in IX is also needed for the putchar so don't overwrite it
		LD	(currentFont.x),IX
                LD	(currentFont.y),IY	
		RET
		
		
currentFont
.vramAddress	DB	7		; High Byte
.x		DW	0
.y		DW	0
.width		DB	8		; Font Size
.height		DB	8		;	
.charSize	DB	0		; Char size in bytes.

		ENDIF
	
;----------------------------------------------------------------------------;
; Gfx9000 bitmap functions                                                   ;
;----------------------------------------------------------------------------;
		IFNDEF G9K_DISABLE_G9B	
			
OpenG9B:
; Input  : DE = Pointer to file name
; Output : HL = Pointer to G9B header data, NZ = error , Error code in A
; Modifies: 		
		PUSH	HL
		XOR	A,A	
		CALL	File.FileOpen
		POP	HL
		RET	NZ		; Error opening
		
		PUSH	HL
		LD	HL,5		; ID + HEADER LENGTH
		LD	DE,G9B_DATA_ID
		CALL	File.FileRead
		POP	HL
		RET	NZ		; Error reading
		
		PUSH	HL		
		PUSH	BC		; Save file handle
		LD	B,3
		LD	HL,G9B_DATA_ID
		LD	DE,G9B_ID
		CALL	String.StringCompare
		LD	A,_NOG9B
		POP	BC
		POP	HL
		RET	NZ		; Not a G9B file
		
		LD	(HL),B		; Save handle in G9B struct
		INC	HL
		INC	HL
		EX	DE,HL
		LD	HL,(G9B_HEADER_SIZE)
		JP	File.FileRead

ReadG9B:
; IX = Pointer to G9B object struct
; DE = Pointer to load buffer
; BC = Size of load buffer
; HL = Destination X
; IY = Destination Y
;  A = Palette offset in bytes. Only valid with palette modes 	
		
		PUSH	AF	; Palette pointer
		LD	(G9B_COPY_PARMS),HL	
		LD	(G9B_COPY_PARMS+2),IY	
		
		LD	A,(IX+G9B_OBJECT.width+0)
		LD	(G9B_COPY_PARMS+4),A
		LD	A,(IX+G9B_OBJECT.width+1)		
		LD	(G9B_COPY_PARMS+5),A		; Copy X
		
		LD	A,(IX+G9B_OBJECT.height+0)
		LD	(G9B_COPY_PARMS+6),A
		LD	A,(IX+G9B_OBJECT.height+1)		
		LD	(G9B_COPY_PARMS+7),A		; Copy Y	
		
		LD	HL,G9B_COPY_PARMS
		POP	AF    ; Palette pointer
		PUSH	BC
		PUSH	DE
		PUSH 	AF     ; Palette pointer
		CALL	SetupCopyRamToXY	
		POP	BC
		LD	C,B    ; C = palette pointer
		LD	A,(IX+G9B_OBJECT.nrColors) 
		OR	A,A		
		CALL	NZ,G9BLoadPalette
		POP	DE		
		POP	BC
		RET	NZ	      ; Error Loading palette
		
		LD	HL,CopyRamToXY
		LD	(ReadToG9K.copyMode+1),HL ; Set copy mode
		
		IFNDEF G9K_DISABLE_BITBUST
		  LD	  A,(IX + G9B_OBJECT.compression)
		  AND	  A,07FH
		  CP	  A,G9B_COMPRESSION_BITBUST
		  JP	  Z,G9kBitBustXY
		ENDIF
		
ReadG9B.Load		
		LD	A, (IX+G9B_OBJECT.dataSize+2)
		LD	HL,(IX+G9B_OBJECT.dataSize)
ReadG9B.0	
		PUSH	HL
		POP	IY	       ; Save low word, because this is the last block 
		OR	A,A
		SBC	HL,BC
		SBC	A,0	       ; Calculate block size
		JR	C,ReadG9B.1    ; Jumps when last block is reached
		PUSH	AF 	       ; Size still to read high byte 
		PUSH	HL     	       ; Size still to read low word	
		PUSH	BC	       ; buffer size	
		LD	HL,BC	       ; Block size is buffer size     
		LD	B,(IX + G9B_OBJECT.fileHandle) ; Get file handle
		CALL	ReadToG9K
		POP	BC	       ; buffer size
		POP	HL             ; Size still to read low word 
		POP	AF	       ; Size still to read high byte  
		JP 	ReadG9B.0
		
ReadG9B.1	
		PUSH	IY
		POP	HL
		LD	B,(IX)	; Get file handle	
		JP	ReadToG9K
		


ReadG9BLinear:
; IX   = Pointer to G9B header data
; DE   = Pointer to load buffer
; BC   = Size of load buffer
; IYL:HL = Vram destination address
;    A = Palette offset in bytes. Only valid with in palette modes 	
		
		PUSH	AF
		PUSH	BC
		PUSH	DE ; Pointer to buffer
		LD	E,IYL
		CALL	SetVramWrite
		POP	DE
		POP	BC
		POP	AF
		
		PUSH	DE
		PUSH	BC
		LD	C,A    ; C = palette pointer
		LD	A,(IX+G9B_OBJECT.nrColors)
		OR	A,A		
		CALL	NZ,G9BLoadPalette
		POP	BC
		POP	DE		
		RET	NZ	      ; Error Loading palette
		
		LD	HL,CopyRamToVram
		LD	(ReadToG9K.copyMode+1),HL
		
		IFNDEF G9K_DISABLE_BITBUST
		  LD	  A,(IX + G9B_OBJECT.compression)
		  AND	  A,07FH
		  CP	  A,G9B_COMPRESSION_BITBUST
		  JP	  Z,G9kBitBustLinear
		ENDIF
		JP	ReadG9B.Load	
		
ReadToG9K:	
; Input B = FileHandle	
;      DE = Pointer to buffer
;      HL = Bytes to read	 	
		
		PUSH	DE
		PUSH	IX
		
		PUSH	DE  ; Pointer to buffer
		PUSH	HL  ; Bytes to read
		CALL	File.FileRead
		POP	BC  ; Bytes to read
		POP	HL  ; Pointer to buffer
		
.copyMode	CALL	CopyRamToXY

		POP	IX
		POP	DE
		RET	
		
G9BLoadPalette:
; A = Number of colors
; DE = Pointer to buffer
;  C = Palette pointer
		
		LD	B,A
		ADD	A,A
		ADD	A,B	; Number of colors * 3 = size on disk
		LD	H,0
		LD	L,A
		PUSH	HL	; Bytes to read
		PUSH	DE      ; Pointer to buffer
		LD	B,(IX + G9B_OBJECT.fileHandle)
		CALL	File.FileRead ; Saves BC
		POP	HL      ; Pointer to buffer 
		POP	DE      ; Bytes to read
		RET	NZ	; Error reading palette	
		LD	B,E
		JP	WritePalette
		
		
G9B_ID		DB	"G9B"	
G9B_DATA_ID	DS	3,0
G9B_HEADER_SIZE DW	0	
G9B_COPY_PARMS	DW	0
		DW	0
		DW	0
		DW	0

		IFNDEF G9K_DISABLE_BITBUST

G9kBitBustXY:
		LD	HL,CopyRamToXY	
		JR	G9kBitBust
			
G9kBitBustLinear:
		LD	HL,CopyRamToVram
		
G9kBitBust:		
		LD	(.copyFunction+1),HL
		
		PUSH	DE
		LD	B,(IX + G9B_OBJECT.fileHandle) ; Get file handle
		LD	DE,.BLOCKS
		LD	HL,1
		CALL	File.FileRead
		POP	DE
		RET	NZ	; Read error		
.loop		
		PUSH	DE
		LD	DE,.BLOCK_SIZE
		LD	HL,2
		CALL	File.FileRead
		POP	DE	; pointer to buffer
		RET	NZ	; Read error
		
		PUSH	DE      ; pointer to buffer
		LD	HL,(.BLOCK_SIZE)
		CALL	File.FileRead	
		POP	DE      ; pointer to buffer
		RET	NZ       ; Read error
				
		PUSH	BC
		LD	HL,(.BLOCK_SIZE) ; bytes
		ADD	HL,DE    ; Calculate Address to depack to
		PUSH	DE       ; pointer to buffer 
		
		PUSH	HL	 ; Pointer to decompress to 	
		EX	DE,HL    ; HL =  Address of packed data
		CALL	bitbuster.depack_raw
		; DE = Address of first byte after decompressed data
		POP	HL       ; Pointer to decompressed data
				
		PUSH	HL
		EX	DE,HL
		OR	A,A		
		SBC	HL,DE    ; Calculate bytes to send to the vdp
		LD	B,H
		LD	C,L	 ; Bytes to send 
		POP	HL	 ; Pointer to decompressed data
		
.copyFunction	CALL	CopyRamToVram
		POP	DE       ; pointer to buffer 
		POP	BC
		
		LD	A,(.BLOCKS)
		DEC	A
		LD	(.BLOCKS),A
		JP	NZ,.loop
		RET
	
		
.BLOCKS		DB	0
.BLOCK_SIZE	DW	0

		ENDIF

		ENDIF

Close:
; HL = Pointer to G9B/VFF header object
; Modifies: B
		LD	B,(IX)
		PUSH	IX
		CALL	File.FileClose
		POP	IX
		LD	(IX),0 ; clear handle
		RET
		
;----------------------------------------------------------------------------;
; Gfx9000 Mouse cursor functions                                             ;
;----------------------------------------------------------------------------;

		IFNDEF G9K_DISABLE_CURSOR

SetCursorXY:
; Both cursors are set at the same,because the data is 1 bit. So both cursors
; are needed to make a good cursor
; input    : HL=X
;          : DE=Y
; Modifies : A,C

		; Set V9990 address = #7FE00
		XOR	A,A
		OUT	(G9K_REG_SELECT),A
		OUT	(G9K_REG_DATA),A
		LD	A,0FEh
		OUT	(G9K_REG_DATA),A
		LD	A,7
		OUT	(G9K_REG_DATA),A 
		
		; "First" cursor
		LD	C,G9K_VRAM
		OUT	(C),E        ; Y cordinate low byte
		OUT	(C),E        ; Dummy		       
		OUT	(C),D        ; Y cordinate high byte
		OUT	(C),D        ; Dummy
		
                OUT     (C),L        ; X cordinate low byte
                OUT     (C),L        ; Dummy
                LD      A,(CURSOR0_ATTRIB)
                OR	A,H	     ; Set color atributes
                OUT     (G9K_VRAM),A ; X cordinate High byte and color
                OUT     (G9K_VRAM),A ; Dummy	
                      
                ; "Second" cursor
                OUT     (C),E        ; Y cordinate low byte
                OUT     (C),E        ; Dummy
                OUT     (C),D        ; Y cordinate high byte 
                OUT     (C),D        ; Dummy
                OUT     (C),L        ; X cordinate low byte
                OUT     (C),L        ; Dummy
                LD      A,(CURSOR1_ATTRIB)
                OR      A,H
                OUT     (G9K_VRAM),A ; X cordinate High byte and color
		RET

SetCursorPattern:
		

		RET

SetCursorAttrib:
; Input A = Cursor Number ( 0 or 1)		
		LD    	HL,CURSOR0_ATTRIB
		OR	A,A
		JR	Z,$+3
		 INC	HL 
		LD	(HL),A
		RET
		
CURSOR0_ATTRIB   DB  	0
CURSOR1_ATTRIB   DB  	0

		ENDIF

;----------------------------------------------------------------------------;
; Gfx9000 Pattern mode functions                                             ;
;----------------------------------------------------------------------------;

		IFNDEF G9K_DISABLE_PATTERN
SetPatternData:
; DE = Ptr to pattern data
; HL = Pattern number
;  A = Layer number (0=A,1=B)
; Modifies: A,BC,HL
		PUSH	DE
		ADD	A,A
		LD	(PAT.LAYER_SEL_SC),A
		LD	(PAT.LAYER_SEL_DS),A		
		ADD	HL,HL
		ADD	HL,HL
		ADD	HL,HL
		LD	A,L
		LD	(PAT.X),A 
		LD	A,H
		ADD	A,A
		ADD	A,A
		ADD	A,A
		LD	(PAT.Y),A 
		LD	HL,PAT.X		
		CALL 	SetupCopyRamToXY
		POP	HL
		LD	BC,8*8
		JP	CopyRamToXY
		  
		 

GetPatternData:
; DE = Ptr to buffer to write pattern data in
; HL = Pattern number
;  A = Layer number (0=A,1=B)
; Modifies: A,BC,HL
		PUSH	DE
		ADD	A,A
		LD	(PAT.LAYER_SEL_SC),A
		LD	(PAT.LAYER_SEL_DS),A		
		ADD	HL,HL
		ADD	HL,HL
		ADD	HL,HL
		LD	A,L
		LD	(PAT.X),A 
		LD	A,H
		ADD	A,A
		ADD	A,A
		ADD	A,A
		LD	(PAT.Y),A 
		LD	HL,PAT.X		
		CALL 	SetupCopyXYToRam
		POP	HL
		LD	BC,8*8
		JP	CopyXYToRam
		  
		 
PAT.X			DB	0
PAT.LAYER_SEL_SC	DB	0
PAT.Y			DB	0,0
PAT.WIDTH		DB	8
PAT.LAYER_SEL_DS	DB	0
PAT.HEIGHT		DB	8,0
	
SetPattern:
; HL = Pattern number in generator table (on screen patterns 0 to A=8159,B=7679)
; DE = Pattern number in name table (on screen patterns 0 to 4095)
;  A = Layer number (0=A,1=B)			
		PUSH	DE
		ADD	HL,HL
		LD	DE,0C000h
		OR	A,A
		JR	Z,$+5
		  LD	  DE,0E000h
		ADD	HL,DE
		LD	E,7		
		CALL	SetVramWrite
		POP	DE
		LD	A,E
		OUT	(G9K_VRAM),A
		LD	A,D
		OUT	(G9K_VRAM),A
		RET     
		
GetPattern:
; Input  DE = Pattern number in name table (on screen patterns 0 to 4095)
; Output HL = Pattern number in generator table (on screen patterns 0 to A=8159,B=7679)
;  A = Layer number (0=A,1=B)		
		PUSH	DE
		ADD	HL,HL
		LD	DE,0C000h
		OR	A,A
		JR	Z,$+5
		  LD	  DE,0E000h
		ADD	HL,DE
		LD	E,7		
		CALL	SetVramRead
		POP	DE
		IN	A,(G9K_VRAM)
		LD	A,E
		IN	A,(G9K_VRAM)
		LD	A,D
		RET         	            
		ENDIF


;----------------------------------------------------------------------------;
; Gfx9000 Pattern mode sprite functions                                      ;
;----------------------------------------------------------------------------;

		IFNDEF G9K_DISABLE_SPRITES
SetSprite:
; B = Sprite Number (0 to 124)
		; Set V9990 address = base 03FE00h + sprite number*4
		XOR	A,A
		OUT	(G9K_REG_SELECT),A
		LD	A,B
		LD	B,0
		ADD	A,A
		ADD	A,A
		OUT	(G9K_REG_DATA),A  ; Low address
		RL	B
		LD	A,0FEh
		ADD	A,B               
		OUT	(G9K_REG_DATA),A  ; Mid
		LD	A,03h 
		OUT	(G9K_REG_DATA),A  ; High 
		
		RET
             	
             	
             	ENDIF
             	
         
