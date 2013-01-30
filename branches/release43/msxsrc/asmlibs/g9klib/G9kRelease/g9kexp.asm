; Define The start address of the g9k lib here
G9K_START_ADDRESS	equ	01000h 

			;DEFINE  ENABLE_JUMP_TABLE
			
; Note: remove bitbust export labels from the exp file when you assemble this file
	
	  		; Disable specific cursor functions
	        	;DEFINE	 G9K_DISABLE_CURSOR
	        	
	        	; Disable vff support
			;DEFINE  G9K_DISABLE_VFF
			
		        ; Disable routines for vram fonts
			;DEFINE  G9K_DISABLE_VFF_VRAM 

	 		; Disable routines for ram fonts
			;DEFINE  G9K_DISABLE_VFF_RAM 

			
			; Disable specific pattern mode functions
			;DEFINE  G9K_DISABLE_PATTERN
			
			; Optimize bitbuster code, bigger but faster
			DEFINE  BITBUSTER_OPTIMIZE_SPEED
			
			; disable the direct export of internal labels in de g9klib.exp file
			DEFINE  G9K_DISABLE_DIRECT_EXPORT
	         	
	         	
	         	OUTPUT g9klib.lib
	         	
	                include	"g9klib.inc"
	                include "bdos.inc"
			include "macros.inc"			
		
			org	G9K_START_ADDRESS
			IFDEF   ENABLE_JUMP_TABLE
;----------------------------------------------------------------------------;
; General Functions overview                                                 ;
;----------------------------------------------------------------------------;
		
							
			EXPORT  G9kReset				
G9kReset:		JP      G9k.Reset    	    ; Reset and initialize the Gfx9000	

			EXPORT  G9kSetScreenMode
G9kSetScreenMode:	JP      G9k.SetScreenMode   ; Set screen mode

			EXPORT  G9kSetVramWrite
G9kSetVramWrite:	JP      G9k.SetVramWrite    ; Set vram write address

			EXPORT	G9kSetVramRead
G9kSetVramRead:		JP      G9k.SetVramRead     ; Set vram read address

			EXPORT  G9kDetect
G9kDetect:		JP      G9k.Detect          ; Detect presence of the Gfx9000

			EXPORT  G9kDisplayEnable
G9kDisplayEnable:	JP      G9k.DisplayEnable   ; Enable display

			EXPORT  G9kDisplayDisable
G9kDisplayDisable:	JP      G9k.DisplayDisable  ; Disable display

			EXPORT  G9kSpritesEnable
G9kSpritesEnable:	JP      G9k.SpritesEnable   ; Enable sprites/mouse cursor

			EXPORT  G9kSpritesDisable
G9kSpritesDisable:	JP      G9k.SpritesDisable  ; Disable sprites/mouse cursor

			EXPORT  G9kWritePalette
G9kWritePalette:	JP      G9k.WritePalette    ; Write palette data to the Gfx9000

			EXPORT  G9kReadPalette
G9kReadPalette:		JP      G9k.ReadPalette     ; Read palette data from the Gfx9000

			EXPORT  G9kSetAdjust
G9kSetAdjust:		JP      G9k.SetAdjust       ; Adjust Gfx9000 display 

			EXPORT  G9kSetBackDropColor
G9kSetBackDropColor:	JP      G9k.SetBackDropColor; Set backdrop color

			EXPORT  G9kSetScrollX
G9kSetScrollX:		JP      G9k.SetScrollX      ; Set scroll X Layer A

			EXPORT  G9kSetScrollY
G9kSetScrollY:		JP      G9k.SetScrollY      ; Set scroll Y Layer A

			IFNDEF  G9K_DISABLE_PATTERN
			EXPORT  G9kSetScrollXB
G9kSetScrollXB:		JP      G9k.SetScrollXB	    ; Set scroll X Layer B

			EXPORT  G9kSetScrollYB
G9kSetScrollYB:		JP      G9k.SetScrollYB     ; Set scroll Y Layer B
			ENDIF

			EXPORT  G9kSetScrollMode
G9kSetScrollMode:	JP      G9k.SetScrollMode   ; Set scroll mode

			EXPORT  G9kClose
G9kClose:		JP      G9k.Close           ; Closes a G9B or VFF file	
	

;----------------------------------------------------------------------------;
; Blitter Function overview                                                  ;
;----------------------------------------------------------------------------;
			EXPORT  G9kDrawFilledBox
G9kDrawFilledBox:       JP      G9k.DrawFilledBox     ; Draw filled box

			EXPORT  G9kDrawBox
G9kDrawBox:		JP 	G9k.DrawBox           ; Draw box

			EXPORT  G9kDrawLine
G9kDrawLine:		JP 	G9k.DrawLine	      ; Draw line (simple)

			EXPORT  G9kSetupCopyRamToXY
G9kSetupCopyRamToXY: 	JP 	G9k.SetupCopyRamToXY  ; Setup parameters for Ram to XY copy

			EXPORT  G9kCopyRamToXY
G9kCopyRamToXY:	 	JP 	G9k.CopyRamToXY       ; Copy data from Ram to XY

			EXPORT  G9kSetupCopyXYToRam
G9kSetupCopyXYToRam: 	JP 	G9k.SetupCopyXYToRam  ; Setup parameters for XY to Ram copy

			EXPORT  G9kCopyXYToRam
G9kCopyXYToRam:		JP 	G9k.CopyXYToRam       ; Copy data from XY to Ram

			EXPORT  G9kCopyXYToXY
G9kCopyXYToXY:		JP 	G9k.CopyXYToXY        ; Copy XY to XY

			EXPORT  G9kCopyXYToRegisterXY
G9kCopyXYToRegisterXY:	JP 	G9k.CopyXYToRegisterXY; Copy XY(struct) to XY (registers)

			EXPORT  G9kCopyVramToXY
G9kCopyVramToXY:	JP 	G9k.CopyVramToXY      ; Copy Linear vram address to XY

			EXPORT  G9kCopyXYToVram
G9kCopyXYToVram:	JP 	G9k.CopyXYToVram      ; Copy XY to Linear vram address

			EXPORT  G9kSetCmdWriteMask
G9kSetCmdWriteMask:	JP 	G9k.SetCmdWriteMask   ; Set blitter command write mask

			EXPORT  G9kSetCmdColor
G9kSetCmdColor:		JP 	G9k.SetCmdColor       ; Set blitter command color

			EXPORT  G9kSetCmdBackColor
G9kSetCmdBackColor:	JP 	G9k.SetCmdBackColor   ; Set command back ground color

			EXPORT  G9kCopyRamToVram
G9kCopyRamToVram:	JP 	G9k.CopyRamToVram     ; Copy data from ram to Linear vram address

;----------------------------------------------------------------------------;
; Font Function overview                                                     ;
; ---------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_VFF to disable inclution of vff functions
			IFNDEF G9K_DISABLE_VFF
			
			EXPORT   G9kOpenVff
G9kOpenVff:		JP	 G9k.OpenVff            ; Open a VFF file
			
			EXPORT   G9kLoadFont
G9kLoadFont:		JP	 G9k.LoadFont	      ; Loads a VFF(V9990 font format) file from disk

			EXPORT   G9kSetFont
G9kSetFont:		JP	 G9k.SetFont            ; Set a font as default

			EXPORT   G9kPrintString
G9kPrintString:		JP	 G9k.PrintString        ; Print a zero terminated string 
			
			EXPORT   G9kPutChar
G9kPutChar:		JP 	 G9k.PutChar	      ; Print a character
	
			EXPORT   G9kLocate	      ; Set X and Y coordinates for putchar
G9kLocate:		JP	 G9k.Locate
			
			ENDIF
;----------------------------------------------------------------------------;
; Gfx9000 bitmap functions                                                   ;
;----------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_G9B to disable inclution of G9B functions
			IFNDEF G9K_DISABLE_G9B
			
			EXPORT   G9kOpenG9B
G9kOpenG9B:		JP	 G9k.OpenG9B            ; Open a G9B file

			EXPORT   G9kReadG9B
G9kReadG9B:		JP	 G9k.ReadG9B            ; Read data from disk to Gfx9000 VRAM X,Y

			EXPORT   G9kReadG9BLinear
G9kReadG9BLinear:	JP	 G9k.ReadG9BLinear      ; Read data from disk to Gfx9000 Linear VRAM Address
		
			ENDIF
		
		
;----------------------------------------------------------------------------;
; Gfx9000 pattern functions                                                  ;
;----------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_PATTERN to disable inclution of pattern functions
			IFNDEF G9K_DISABLE_PATTERN
			
			EXPORT   G9kSetPatternData
G9kSetPatternData:	JP 	 G9k.SetPatternData     ; Set pattern data

			EXPORT   G9kGetPatternData
G9kGetPatternData:	JP	 G9k.GetPatternData     ; Get partern data

			EXPORT   G9kSetPattern
G9kSetPattern:		JP	 G9k.SetPattern         ; Set pattern

			EXPORT   G9kGetPattern
G9kGetPattern:		JP	 G9k.GetPattern         ; Get pattern
						
			include "g9klib.asm"
			include "file.asm"
			include "string.asm"
			include "bitbuster.asm"	
			include "bdos.asm"	
			
			ENDIF


			ELSE
						
			include "g9klib.asm"
			include "file.asm"
			include "string.asm"
			include "bitbuster.asm"	
			include "bdos.asm"	
			
;----------------------------------------------------------------------------;
; General Functions overview                                                 ;
;----------------------------------------------------------------------------;


			EXPORT  G9kReset				
G9kReset:		equ      G9k.Reset    	    ; Reset and initialize the Gfx9000	

			EXPORT  G9kSetScreenMode
G9kSetScreenMode:	equ      G9k.SetScreenMode   ; Set screen mode

			EXPORT  G9kSetVramWrite
G9kSetVramWrite:	equ      G9k.SetVramWrite    ; Set vram write address

			EXPORT	G9kSetVramRead
G9kSetVramRead:		equ      G9k.SetVramRead     ; Set vram read address

			EXPORT  G9kDetect
G9kDetect:		equ      G9k.Detect          ; Detect presence of the Gfx9000

			EXPORT  G9kDisplayEnable
G9kDisplayEnable:	equ      G9k.DisplayEnable   ; Enable display

			EXPORT  G9kDisplayDisable
G9kDisplayDisable:	equ      G9k.DisplayDisable  ; Disable display

			EXPORT  G9kSpritesEnable
G9kSpritesEnable:	equ      G9k.SpritesEnable   ; Enable sprites/mouse cursor

			EXPORT  G9kSpritesDisable
G9kSpritesDisable:	equ      G9k.SpritesDisable  ; Disable sprites/mouse cursor

			EXPORT  G9kWritePalette
G9kWritePalette:	equ      G9k.WritePalette    ; Write palette data to the Gfx9000

			EXPORT  G9kReadPalette
G9kReadPalette:		equ      G9k.ReadPalette     ; Read palette data from the Gfx9000

			EXPORT  G9kSetAdjust
G9kSetAdjust:		equ      G9k.SetAdjust       ; Adjust Gfx9000 display 

			EXPORT  G9kSetBackDropColor
G9kSetBackDropColor:	equ      G9k.SetBackDropColor; Set backdrop color

			EXPORT  G9kSetScrollX
G9kSetScrollX:		equ      G9k.SetScrollX      ; Set scroll X Layer A

			EXPORT  G9kSetScrollY
G9kSetScrollY:		equ      G9k.SetScrollY      ; Set scroll Y Layer A

			IFNDEF  G9K_DISABLE_PATTERN
			EXPORT  G9kSetScrollXB
G9kSetScrollXB:		equ      G9k.SetScrollXB	    ; Set scroll X Layer B

			EXPORT  G9kSetScrollYB
G9kSetScrollYB:		equ      G9k.SetScrollYB     ; Set scroll Y Layer B
			ENDIF

			EXPORT  G9kSetScrollMode
G9kSetScrollMode:	equ      G9k.SetScrollMode   ; Set scroll mode

			EXPORT  G9kClose
G9kClose:		equ      G9k.Close           ; Closes a G9B or VFF file	
	

;----------------------------------------------------------------------------;
; Blitter Function overview                                                  ;
;----------------------------------------------------------------------------;
			EXPORT  G9kDrawFilledBox
G9kDrawFilledBox:       equ      G9k.DrawFilledBox     ; Draw filled box

			EXPORT  G9kDrawBox
G9kDrawBox:		equ 	G9k.DrawBox           ; Draw box

			EXPORT  G9kDrawLine
G9kDrawLine:		equ 	G9k.DrawLine	      ; Draw line (simple)

			EXPORT  G9kSetupCopyRamToXY
G9kSetupCopyRamToXY: 	equ 	G9k.SetupCopyRamToXY  ; Setup parameters for Ram to XY copy

			EXPORT  G9kCopyRamToXY
G9kCopyRamToXY:	 	equ 	G9k.CopyRamToXY       ; Copy data from Ram to XY

			EXPORT  G9kSetupCopyXYToRam
G9kSetupCopyXYToRam: 	equ 	G9k.SetupCopyXYToRam  ; Setup parameters for XY to Ram copy

			EXPORT  G9kCopyXYToRam
G9kCopyXYToRam:		equ 	G9k.CopyXYToRam       ; Copy data from XY to Ram

			EXPORT  G9kCopyXYToXY
G9kCopyXYToXY:		equ 	G9k.CopyXYToXY        ; Copy XY to XY

			EXPORT  G9kCopyXYToRegisterXY
G9kCopyXYToRegisterXY:	equ 	G9k.CopyXYToRegisterXY; Copy XY(struct) to XY (registers)

			EXPORT  G9kCopyVramToXY
G9kCopyVramToXY:	equ 	G9k.CopyVramToXY      ; Copy Linear vram address to XY

			EXPORT  G9kCopyXYToVram
G9kCopyXYToVram:	equ 	G9k.CopyXYToVram      ; Copy XY to Linear vram address

			EXPORT  G9kSetCmdWriteMask
G9kSetCmdWriteMask:	equ 	G9k.SetCmdWriteMask   ; Set blitter command write mask

			EXPORT  G9kSetCmdColor
G9kSetCmdColor:		equ 	G9k.SetCmdColor       ; Set blitter command color

			EXPORT  G9kSetCmdBackColor
G9kSetCmdBackColor:	equ 	G9k.SetCmdBackColor   ; Set command back ground color

			EXPORT  G9kCopyRamToVram
G9kCopyRamToVram:	equ 	G9k.CopyRamToVram     ; Copy data from ram to Linear vram address

;----------------------------------------------------------------------------;
; Font Function overview                                                     ;
; ---------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_VFF to disable inclution of vff functions
			IFNDEF G9K_DISABLE_VFF
			
			EXPORT   G9kOpenVff
G9kOpenVff:		equ	 G9k.OpenVff            ; Open a VFF file
			
			EXPORT   G9kLoadFont
G9kLoadFont:		equ	 G9k.LoadFont	      ; Loads a VFF(V9990 font format) file from disk

			EXPORT   G9kSetFont
G9kSetFont:		equ	 G9k.SetFont            ; Set a font as default

			EXPORT   G9kPrintString
G9kPrintString:		equ	 G9k.PrintString        ; Print a zero terminated string 
			
			EXPORT   G9kPutChar
G9kPutChar:		equ 	 G9k.PutChar	      ; Print a character
	
			EXPORT   G9kLocate	      ; Set X and Y coordinates for putchar
G9kLocate:		equ	 G9k.Locate
			
			ENDIF
;----------------------------------------------------------------------------;
; Gfx9000 bitmap functions                                                   ;
;----------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_G9B to disable inclution of G9B functions
			IFNDEF G9K_DISABLE_G9B
			
			EXPORT   G9kOpenG9B
G9kOpenG9B:		equ	 G9k.OpenG9B            ; Open a G9B file

			EXPORT   G9kReadG9B
G9kReadG9B:		equ	 G9k.ReadG9B            ; Read data from disk to Gfx9000 VRAM X,Y

			EXPORT   G9kReadG9BLinear
G9kReadG9BLinear:	equ	 G9k.ReadG9BLinear      ; Read data from disk to Gfx9000 Linear VRAM Address
		
			ENDIF
		
;----------------------------------------------------------------------------;
; Gfx9000 pattern functions                                                  ;
;----------------------------------------------------------------------------;
; DEFINE G9K_DISABLE_PATTERN to disable inclution of pattern functions
			IFNDEF G9K_DISABLE_PATTERN
			
			EXPORT   G9kSetPatternData
G9kSetPatternData:	equ 	 G9k.SetPatternData     ; Set pattern data

			EXPORT   G9kGetPatternData
G9kGetPatternData:	equ	 G9k.GetPatternData     ; Get partern data

			EXPORT   G9kSetPattern
G9kSetPattern:		equ	 G9k.SetPattern         ; Set pattern

			EXPORT   G9kGetPattern
G9kGetPattern:		equ	 G9k.GetPattern         ; Get pattern
			ENDIF
			
			ENDIF
	
		
		
		