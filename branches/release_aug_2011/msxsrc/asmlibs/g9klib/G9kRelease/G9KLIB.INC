		ifndef G9KLIB_INC
		define G9KLIB_INC
		
		ifndef G9K_DISABLE_DIRECT_EXPORT
;----------------------------------------------------------------------------;
; General Functions overview                                                 ;
;----------------------------------------------------------------------------;
		EXPORT G9k.Reset    	   ; Reset and initialize the Gfx9000
		EXPORT G9k.SetScreenMode   ; Set screen mode
		EXPORT G9k.SetVramWrite    ; Set vram write address
		EXPORT G9k.SetVramRead     ; Set vram read address
		EXPORT G9k.Detect          ; Detect presence of the Gfx9000
		EXPORT G9k.DisplayEnable   ; Enable display
		EXPORT G9k.DisplayDisable  ; Disable display
		EXPORT G9k.SpritesEnable   ; Enable sprites/mouse cursor
		EXPORT G9k.SpritesDisable  ; Disable sprites/mouse cursor
		EXPORT G9k.WritePalette    ; Write palette data to the Gfx9000
		EXPORT G9k.ReadPalette     ; Read palette data from the Gfx9000
		EXPORT G9k.SetAdjust       ; Adjust Gfx9000 display 
		EXPORT G9k.SetBackDropColor; Set backdrop color
		EXPORT G9k.SetScrollX      ; Set scroll X Layer A
		EXPORT G9k.SetScrollY      ; Set scroll Y Layer A
		IFNDEF G9K_DISABLE_PATTERN
		EXPORT G9k.SetScrollXB	   ; Set scroll X Layer B
		EXPORT G9k.SetScrollYB     ; Set scroll Y Layer B
		ENDIF
		EXPORT G9k.SetScrollMode   ; Set scroll mode
		EXPORT G9k.Close           ; Closes a G9B or VFF file

;----------------------------------------------------------------------------;
; Blitter Function overview                                                  ;
;----------------------------------------------------------------------------;
		EXPORT G9k.DrawFilledBox     ; Draw filled box
		EXPORT G9k.DrawBox           ; Draw box
		EXPORT G9k.DrawLine	     ; Draw line (simple)
		EXPORT G9k.SetupCopyRamToXY  ; Setup parameters for Ram to XY copy
		EXPORT G9k.CopyRamToXY       ; Copy data from Ram to XY
		EXPORT G9k.SetupCopyXYToRam  ; Setup parameters for XY to Ram copy
		EXPORT G9k.CopyXYToRam       ; Copy data from XY to Ram
		EXPORT G9k.CopyXYToXY        ; Copy XY to XY
		EXPORT G9k.CopyXYToRegisterXY; Copy XY(struct) to XY (registers)
		EXPORT G9k.CopyVramToXY      ; Copy Linear vram address to XY
		EXPORT G9k.CopyXYToVram      ; Copy XY to Linear vram address
		EXPORT G9k.SetCmdWriteMask   ; Set blitter command write mask
		EXPORT G9k.SetCmdColor       ; Set blitter command color
		EXPORT G9k.SetCmdBackColor   ; Set command back ground color
		EXPORT G9k.CopyRamToVram     ; Copy data from ram to Linear vram address
;----------------------------------------------------------------------------;
; Font Function overview                                                     ;
; ---------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_VFF to disable inclution of vff functions
		IFNDEF G9K_DISABLE_VFF
		EXPORT G9k.OpenVff            ; Open a VFF file
		EXPORT G9k.LoadFont	      ; Loads a VFF(V9990 font format) file from disk
		EXPORT G9k.SetFont            ; Set a font as default
		EXPORT G9k.PrintString        ; Print a zero terminated string 
		EXPORT G9k.PutChar	      ; Print a character
		EXPORT G9k.Locate	      ; Set X and Y coordinates for putchar
		ENDIF
;----------------------------------------------------------------------------;
; Gfx9000 bitmap functions                                                   ;
;----------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_G9B to disable inclution of G9B functions
		IFNDEF G9K_DISABLE_G9B
		EXPORT G9k.OpenG9B            ; Open a G9B file
		EXPORT G9k.ReadG9B            ; Read data from disk to Gfx9000 VRAM X,Y
		EXPORT G9k.ReadG9BLinear      ; Read data from disk to Gfx9000 Linear VRAM Address
		ENDIF
		
;----------------------------------------------------------------------------;
; Gfx9000 pattern functions                                                  ;
;----------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_PATTERN to disable inclution of pattern functions
		IFNDEF G9K_DISABLE_PATTERN
		EXPORT G9k.SetPatternData     ; Set pattern data
		EXPORT G9k.GetPatternData     ; Get partern data
		EXPORT G9k.SetPattern         ; Set pattern
		EXPORT G9k.GetPattern         ; Get pattern
		ENDIF
		
		
;----------------------------------------------------------------------------;
; Macro overview    	                                                     ;
;----------------------------------------------------------------------------;
; G9kCmdWait            - Wait for Blitter command completion
; G9kWriteReg           - Write Gfx9000 register
; G9kReadReg            - Read Gfx9000 register
; G9kWaitVsync          - Wait for Vertical Sync
		ENDIF

;----------------------------------------------------------------------------;
; V9990 register and port defines                                            ;
;----------------------------------------------------------------------------;

; Port defines
G9K_VRAM		EQU	60h	; R/W
G9K_PALETTE		EQU	61h	; R/W
G9K_CMD_DATA		EQU	62h	; R/W
G9K_REG_DATA		EQU	63h	; R/W
G9K_REG_SELECT		EQU	64h	; W
G9K_STATUS		EQU	65h	; R
G9K_INT_FLAG		EQU	66h	; R/W
G9K_SYS_CTRL		EQU	67h	; W
G9K_OUTPUT_CTRL         EQU     6Fh     ; R/W

; Bit defines G9K_SYS_CTRL
G9K_SYS_CTRL_SRS	EQU	2	; Power on reset state
G9K_SYS_CTRL_MCKIN	EQU	1	; Select MCKIN terminal
G9K_SYS_CTRL_XTAL	EQU	0	; Select XTAL

; Register defines
G9K_WRITE_ADDR		EQU	0	; W
G9K_READ_ADDR		EQU	3	; W
G9K_SCREEN_MODE0	EQU	6	; R/W
G9K_SCREEN_MODE1	EQU	7	; R/W
G9K_CTRL		EQU	8	; R/W
G9K_INT_ENABLE          EQU     9       ; R/W
G9K_INT_V_LINE_LO	EQU	10	; R/W	
G9K_INT_V_LINE_HI	EQU	11	; R/W
G9K_INT_H_LINE		EQU	12	; R/W	
G9K_PALETTE_CTRL	EQU	13	; W
G9K_PALETTE_PTR		EQU	14	; W
G9K_BACK_DROP_COLOR 	EQU     15      ; R/W
G9K_DISPLAY_ADJUST	EQU	16	; R/W
G9K_SCROLL_LOW_Y	EQU     17      ; R/W
G9K_SCROLL_HIGH_Y	EQU     18      ; R/W
G9K_SCROLL_LOW_X	EQU     19      ; R/W
G9K_SCROLL_HIGH_X	EQU     20      ; R/W
G9K_SCROLL_LOW_Y_B	EQU     21      ; R/W
G9K_SCROLL_HIGH_Y_B	EQU     22      ; R/W
G9K_SCROLL_LOW_X_B	EQU     23      ; R/W
G9K_SCROLL_HIGH_X_B	EQU     24      ; R/W
G9K_PAT_GEN_TABLE   	EQU     25      ; R/W
G9K_LCD_CTRL        	EQU     26      ; R/W
G9K_PRIORITY_CTRL  	EQU     27      ; R/W
G9K_SPR_PAL_CTRL	EQU	28	; W
G9K_SC_X		EQU	32	; W
G9K_SC_Y		EQU	34	; W
G9K_DS_X		EQU	36	; W
G9K_DS_Y		EQU	38	; W
G9K_NX			EQU	40	; W
G9K_NY			EQU	42	; W
G9K_ARG			EQU	44	; W
G9K_LOP			EQU	45	; W
G9K_WRITE_MASK		EQU	46	; W
G9K_FC			EQU	48	; W
G9K_BC			EQU	50	; W
G9K_OPCODE		EQU	52	; W

; Register Select options
G9K_DIS_INC_READ	EQU	64
G9K_DIS_INC_WRITE	EQU	128

; Bit defines G9K_SCREEN_MODE0 (register 6)
G9K_SCR0_STANDBY	EQU	192	; Stand by mode
G9K_SCR0_BITMAP		EQU	128	; Select Bit map mode
G9K_SCR0_P2		EQU	64	; Select P1 mode
G9K_SCR0_P1		EQU	0	; Select P1 mode
G9K_SCR0_DTCLK		EQU	32	; Master Dot clock not divided
G9K_SCR0_DTCLK2		EQU	16	; Master Dot clock divided by 2
G9K_SCR0_DTCLK4		EQU	0	; Master Dot clock divided by 4
G9K_SCR0_XIM2048	EQU	12	; Image size = 2048
G9K_SCR0_XIM1024	EQU	8	; Image size = 1024
G9K_SCR0_XIM512		EQU	4	; Image size = 512
G9K_SCR0_XIM256		EQU	0	; Image size = 256
G9K_SCR0_16BIT		EQU	3	; 16 bits/dot
G9K_SCR0_8BIT		EQU	2	; 8 bits/dot
G9K_SCR0_4BIT		EQU	1	; 4 bits/dot
G9K_SCR0_2BIT		EQU	0	; 2 bits/dot

; Bit defines G9K_SCREEN_MODE1 (register 7)
G9K_SCR1_C25M		EQU	64	; Select 640*480 mode
G9K_SCR1_SM1		EQU	32	; Selection of 263 lines during non interlace , else 262
G9K_SCR1_SM		EQU	16	; Selection of horizontal frequency 1H=fsc/227.5
G9K_SCR1_PAL		EQU	8	; Select PAL, else NTSC
G9K_SCR1_EO		EQU	4	; Select of vertical resoltion of twice the non-interlace resolution
G9K_SCR1_IL		EQU	2	; Select Interlace
G9K_SCR1_HSCN		EQU	1	; Select High scan mode

; Bit defines G9K_CTRL    (Register 8)
G9K_CTRL_DISP		EQU	128	; Display VRAM
G9K_CTRL_DIS_SPD	EQU	64	; Disable display sprite (cursor)
G9K_CTRL_YSE		EQU	32	; /YS Enable
G9K_CTRL_VWTE		EQU	16	; VRAM Serial data bus control during digitization
G9K_CTRL_VWM		EQU	8	; VRAM write control during digitization
G9K_CTRL_DMAE		EQU	4	; Enable DMAQ output
G9K_CTRL_VRAM512	EQU	2	; VRAM=512KB
G9K_CTRL_VRAM256	EQU	1	; VRAM=256KB
G9K_CTRL_VRAM128	EQU	0	; VRAM=128KB

; Bit defines G9K_INT_ENABLE (register 9)
G9K_INT_IECE	        EQU     4       ; Command end interrupt enable control
G9K_INT_IEH	        EQU     2       ; Display position interrupt enable
G9K_INT_IEV	        EQU     1       ; Int. enable during vertical retrace line interval

; Bit Defines G9K_PALETTE_CTRL  (Register 13)
G9K_PAL_CTRL_YUV	EQU	192	; YUV mode
G9K_PAL_CTRL_YJK	EQU	128	; YJK mode
G9K_PAL_CTRL_256	EQU	64	; 256 color mode
G9K_PAL_CTRL_PAL	EQU	0	; Pallete mode
G9K_PAL_CTRL_YAE	EQU	32	; Enable YUV/YJK RGB mixing mode

; Bit defines G9K_LOP           (Register 45)
G9K_LOP_TP		EQU	16
G9K_LOP_WCSC		EQU	12
G9K_LOP_WCNOTSC		EQU	3
G9K_LOP_WCANDSC		EQU	8
G9K_LOP_WCORSC		EQU	14
G9K_LOP_WCEORSC		EQU	6

; Bit defines G9K_ARG
G9K_ARG_MAJ		EQU	1
G9K_ARG_NEG		EQU	2
G9K_ARG_DIX		EQU	4
G9K_ARG_DIY		EQU	8

; Blitter Commands G9K_OPCODE    (Register 52)
G9K_OPCODE_STOP		EQU	00h	; Command being excuted is stopped 
G9K_OPCODE_LMMC		EQU	10h     ; Data is transferred from CPU to VRAM rectangle area
G9K_OPCODE_LMMV		EQU	20h     ; VRAM rectangle area is painted out
G9K_OPCODE_LMCM		EQU	30h     ; VRAM rectangle area is transferred to CPU
G9K_OPCODE_LMMM		EQU	40h     ; Rectangle area os transferred from VRAM to VRAM
G9K_OPCODE_CMMC		EQU	050h    ; CPU character data is color-developed and transferred to VRAM rectangle area
G9K_OPCODE_CMMK		EQU	060h    ; Kanji ROM data is is color-developed and transferred to VRAM rectangle area
G9K_OPCODE_CMMM		EQU	070h    ; VRAM character data is color-developed and transferred to VRAM rectangle area 
G9K_OPCODE_BMXL		EQU	080h    ; Data on VRAM linear address is transferred to VRAM rectangle area
G9K_OPCODE_BMLX		EQU	090h    ; VRAM rectangle area is transferred to VRAM linear address 
G9K_OPCODE_BMLL		EQU	0A0h    ; Data on VRAM linear address is transferred to VRAM linear address 
G9K_OPCODE_LINE		EQU	0B0h    ; Straight line is drawer on X-Y co-ordinates
G9K_OPCODE_SRCH		EQU	0C0h    ; Border color co-ordinates on X-Y are detected
G9K_OPCODE_POINT	EQU	0D0h    ; Color code on specified point on X-Y is read out
G9K_OPCODE_PSET		EQU	0E0h    ; Drawing is executed at drawing point on X-Y co-ordinates
G9K_OPCODE_ADVN		EQU	0F0h    ; Drawing point on X-Y co-ordinates is shifted

; Bit defines G9K_STATUS
G9K_STATUS_TR           EQU     128
G9K_STATUS_VR           EQU     64
G9K_STATUS_HR           EQU     32
G9K_STATUS_BD           EQU     16
G9K_STATUS_MSC          EQU     4
G9K_STATUS_EO           EQU     2
G9K_STATUS_CE           EQU     1

; Mode select defines for SetScreenMode
G9K_MODE_P1		EQU	0	; Pattern mode 0 256 212
G9K_MODE_P2		EQU	1	; Pattern mode 1 512 212
G9K_MODE_B1		EQU	2	; Bitmap mode 1 256 212
G9K_MODE_B2		EQU	3	; Bitmap mode 2 384 240
G9K_MODE_B3		EQU	4	; Bitmap mode 3 512 212
G9K_MODE_B4		EQU	5	; Bitmap mode 4 768 240
G9K_MODE_B5		EQU	6	; Bitmap mode 5 640 400 (VGA)
G9K_MODE_B6		EQU	7	; Bitmap mode 6 640 480 (VGA)
G9K_MODE_B7		EQU	8	; Bitmap mode 7 1024 212 (Undocumented v9990 mode)

; Fixed VRAM addresses
G9K_SCRA_PAT_NAME_TABLE EQU     07C000h
G9K_SCRB_PAT_NAME_TABLE EQU     07E000h
G9K_P1_SPR_ATTRIB_TABLE EQU     03FE00h
G9K_P2_SPR_ATTRIB_TABLE EQU     07BE00h

G9K_CURSOR0_ATTRIB      EQU     07FE00h
G9K_CURSOR1_ATTRIB      EQU     07FE08h

G9K_CURSOR0_PAT_DATA    EQU     07FF00h
G9K_CURSOR1_PAT_DATA    EQU     07FF80h

G9K_RED                 EQU     32
G9K_GREEN               EQU     1024
G9K_BLUE                EQU     1

;----------------------------------------------------------------------------;
; G9k Structs                                                                ;
;----------------------------------------------------------------------------;

		STRUCT G9B_OBJECT
fileHandle	WORD        ; Dos2 file handle of the openend G9B file
bitDepth	BYTE        ; 2,4,8 or 16 bit
colorType	BYTE	    ; 0=64 color palette mode,64=256 color fixed ,128=YJK and 192=YUV mode
nrColors	BYTE        ; Number of colors in palette mode
width		WORD	    ; Width
height	        WORD        ; Height
compression	BYTE        ; 0 = no compression, other value = compression used
dataSize	D24         ; 24 bit data size	
		ENDS

G9B_COMPRESSION_BITBUST EQU	1

; struct VFF object
		STRUCT VFF_OBJECT
fileHandle	WORD    	    ; Dos2 file handle of the openend VFF file	
name		BLOCK	16,0	    ; vff font name	
width		BYTE                ; width 
height		BYTE                ; height
pitch		BYTE 		    ; 0 = Fixed : 1 = Variable  (Still unsused)
dataSize	WORD		    ; Data size
ptrOffsetTable  WORD		    ; Pointer to a font offset table
vramOffset	WORD	            ; Pointer to base address of font in vram starting at #70000 
ramFont		BYTE		    ; 0 font data in vram, 1 font data in ram
		ENDS

                STRUCT G9K_BOX
left            WORD
top	        WORD    
width           WORD
height          WORD
                ENDS

                STRUCT G9K_COPY_XY_XY
sourceX         WORD
sourceY         WORD    
destX           WORD
destY           WORD
width           WORD
height          WORD
                ENDS

		STRUCT G9K_COPY_VRAM_XY
sourceAddress   D24
destX		WORD
destY		WORD
width		WORD
height		WORD		
		ENDS

		STRUCT G9K_COPY_XY_VRAM
sourceX         WORD
sourceY         WORD    		
destAddress     D24
width		WORD
height		WORD		
		ENDS

;----------------------------------------------------------------------------;
; ;G9k Error codes                                                           ;
;----------------------------------------------------------------------------;
			
; Error codes
_NOVFF          EQU     1       ;  Input file is not a VFF file
_NOG9B		EQU	2 	;  Input file is not a G9B file

;----------------------------------------------------------------------------;
; ;Gfx9000 Macro's                                                           ;
;----------------------------------------------------------------------------;

		MACRO	G9kCmdWait
; Wait for command execution completion
; Modifies A
		IN	A,(G9K_STATUS)
		RRA
		JR	C,$-3
		ENDM

		MACRO G9kWriteReg register,data
; Write gfx9000 register
; modifies: A
		LD	A,register
		OUT	(G9K_REG_SELECT),A
		LD	A,data
		OUT	(G9K_REG_DATA),A
		ENDM

		MACRO G9kReadReg register
; Read Gfx9000 register
; Result in A
		LD	A,register
		OUT	(G9K_REG_SELECT),A
		IN	A,(G9K_REG_DATA)
		ENDM



                MACRO G9kWaitVsync
; Wait for Vsync
                IN      A,(G9K_STATUS)
                AND     A,G9K_STATUS_VR
		JR	NZ,$-4

                IN      A,(G9K_STATUS)
                AND     A,G9K_STATUS_VR
		JR	Z,$-4
                ENDM
		

		MACRO G9kWaitComReady	
; Wait for Command data ready	
		IN      A,(G9K_STATUS)
		RLA
		JR	NC,$-3
		ENDM
		
		endif