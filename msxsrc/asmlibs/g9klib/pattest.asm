; Pattern mode test copying data into pattern table
			
			output	"pattest.com"
			org	0100h
			
			DEFINE  BITBUSTER_OPTIMIZE_SPEED
	
			include "../macros.inc"	
			include "../bdos/bdos.inc"
			include "g9klib.inc"

; Keyboard Row 8
KEY_SPACE_BAR	EQU	0	

Main:		
			CALL	PaletteOnInt	
	                ;CALL	TestPalette	                	                
			RET

PatternBMLLTest:	                
	                CALL    G9k.Detect
	                RET     NZ
	                CALL    G9k.Reset

	                LD      A,G9K_MODE_P1
	                LD      BC,G9K_SCR0_4BIT*256 + G9K_SCR0_XIM512 
	                LD      DE,4 ; Set pallete for layer b on second pallete
	                CALL    G9k.SetScreenMode
   		
			; Set default blitter settings
	                G9kWriteReg G9K_ARG,0
	                G9kWriteReg G9K_LOP,G9K_LOP_WCSC
			LD      HL,#FF00
			CALL    G9k.SetCmdWriteMask
   		
	   		CALL    G9k.DisplayEnable
			
			LD_EHL  400000H
			call	G9k.SetVramWrite
			LD	DE,512
.clear
			XOR	A,A
			OUT	(G9K_VRAM),A
			DEC	DE
			LD	A,E
			OR	A,D
			JP	NZ,.clear			

			LD_EHL  400000H
			call	G9k.SetVramWrite
			XOR	A,A 
			LD	B,0
.loop			
			OUT	(G9K_VRAM),A
			INC	A
			DJNZ	.loop		


			LD	HL,.copy
			CALL	G9k.CopyVramToVram		
			RET	
			
.copy			G9K_COPY_VRAM_VRAM  800000H, 82000H,256   ; 
			
TestPalette:
			DI
			LD	B,64*3
			LD	C,0
			LD	HL,orgPalette
			G9kWaitVsync
			CALL	G9k.ReadPalette	; Read current palette
			
			LD	HL,orgPalette
			LD	DE,tempPalette
			LD	BC,64*3
			LDIR
			LD	B,10	; Do the fade 10 times!		
.loop			
			PUSH	BC
			LD	B,32

.fadeOut:
			PUSH	BC
			
			CALL	FadeOut
			G9kWaitVsync
			
			LD	HL,tempPalette
			LD	B,64*3
			LD	C,0
			CALL	G9k.WritePalette
			
			POP	BC
			DJNZ	.fadeOut
			
			LD	B,32
.fadeIn:			
			PUSH	BC		
			
			CALL	FadeIn

			G9kWaitVsync
			LD	HL,tempPalette
			LD	B,64*3
			LD	C,0
			CALL	G9k.WritePalette
			
			POP	BC
			DJNZ	.fadeIn
			
			POP	BC
			DJNZ	.loop
			EI
			RET
			
			
FadeOut:			
			LD	HL,tempPalette
			LD	B,64*3
.loop					
			LD	A,(HL)
			OR	A,A
			JR	Z,.next
			DEC	A
			LD	(HL),A
.next			
			INC	HL
			DJNZ	.loop
			RET		

			
FadeIn:
			LD	DE,tempPalette
			LD	HL,orgPalette
			LD	B,64*3
.loop					
			LD	A,(DE)
			CP	A,(HL)
			JR	Z,.next
			INC	A
			LD	(DE),A
.next			
			INC	HL
			INC	DE
			DJNZ	.loop
			RET		
			
				
			
orgPalette:		ds	64*3,0
tempPalette:		ds	64*3,0

NR_COLORS		equ	16
	
PaletteOnInt:
			LD	B,NR_COLORS*3
			LD	C,0
			LD	HL,orgPalette
			G9kWaitVsync
			CALL	G9k.ReadPalette	; Read current palette
			
			LD	HL,orgPalette
			LD	DE,tempPalette
			LD	BC,NR_COLORS*3
			LDIR
			CALL	DisableV99x8Int
			; Set v9990 line int on line 
			; This is time critical and my differs with cpu speeds
			LD	HL,212+14
			CALL	G9k.SetIntLine
			G9kWriteReg G9K_INT_ENABLE,G9K_INT_IEH
			
.keyScan		
	
			LD	C,8
			CALL    GetDirectKey
			BIT	KEY_SPACE_BAR,A
			JP	NZ,.keyScan

			G9kWriteReg G9K_INT_ENABLE,0
			LD	A,G9K_INT_IEH
			OUT	(G9K_INT_FLAG),A  ; Clear Int flag
			
			CALL	EnableV99x8Int
			RET	
			
			
G9kFade:
			LD	A,G9K_INT_IEH
			OUT	(G9K_INT_FLAG),A  ; Clear int flag
						
			LD	A,(fade)
			OR	A,A
			JR	NZ,FadeIn2


FadeOut2
			LD	A,(count)
			OR	A,A
			JR	Z,.setFadeIn
			
			DEC	A	
			LD	(count),a
	
			LD	A,G9K_PALETTE_PTR
			OUT	(G9K_REG_SELECT),A
			XOR	A,A
			OUT	(G9K_REG_DATA),A
			
			LD	HL,tempPalette
			LD	B,NR_COLORS*3
.loop					
			LD	A,(HL)
			OR	A,A
			JR	Z,.next
			DEC	A
			LD	(HL),A
.next			
			OUT	(G9K_PALETTE),A
			INC	HL
			DJNZ	.loop
			RET

.setFadeIn:
			LD	A,30
			LD	(count),A
			LD	(fade),A
			RET	
						
FadeIn2
			LD	A,(count)
			OR	A,A
			JR	Z,.setFadeOut
			
			DEC	A	
			LD	(count),a

			LD	A,G9K_PALETTE_PTR
			OUT	(G9K_REG_SELECT),A
			XOR	A,A
			OUT	(G9K_REG_DATA),A

			
			LD	DE,tempPalette
			LD	HL,orgPalette
			LD	B,NR_COLORS*3
.loop					
			LD	A,(DE)
			CP	A,(HL)
			JR	Z,.next
			INC	A
			LD	(DE),A
.next			
			OUT	(G9K_PALETTE),A
			INC	HL
			INC	DE
			DJNZ	.loop
			RET
			
			
.setFadeOut:
			LD	A,30
			LD	(count),A
			XOR	A,A
			LD	(fade),A
			RET	

count			DB 	30
fade			DB	0
			
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
			
			
DisableV99x8Int
			DI
			LD	A,(0F3E0h)   ; GET CONTENTS V99X8 REG 1
			RES	5,A          ; Disable int
			LD	(0F3E0h),A   
			OUT	(099H),A
	            	LD	A,1+128
	             	OUT	(099H),A
	             	IN	A,(99h)
	             	
	             	LD	HL,038h
	             	LD	DE,oldInt
	             	LD	BC,3
	             	LDIR
	             	LD	HL,MyIntHook
	             	LD	DE,038h
	             	LD	BC,3
	             	LDIR
	             	EI
			RET
		
EnableV99x8Int
			DI
			LD	A,(0F3E0h)	; GET CONTENTS V99X8 REG 1
			SET	5,A
			LD	(0F3E0h),A   
			OUT	(099H),A
	            	LD	A,1+128
	             	OUT	(099H),A
	             	LD	HL,oldInt
	             	LD	DE,038h
	             	LD	BC,3
	             	LDIR
	             	EI
			RET

MyIntHook		JP	MyIntHandler
		
MyIntHandler
			DI
			PUSH	AF
			EX	AF,AF
			PUSH	AF
			PUSH	BC
			PUSH	DE
			PUSH	HL
			; Jump to fast responce routines here
			CALL	G9kFade		
			EXX
			PUSH	BC
			PUSH	DE
			PUSH	HL
			PUSH	IX
			PUSH	IY
			;CALL	IntHandler;//0fd9Ah
			POP	IY
			POP	IX
			POP	HL
			POP	DE
			POP	BC
			EXX
			POP	HL
			POP	DE
			POP	BC
			POP	AF
			EX	AF,AF
			POP	AF
			EI
			RET
	
oldInt			DS	3	
			
			include "file.asm"
			include "g9klib.asm"
			include "../string/string.asm"
			include "../bdos/bdos.asm"
			
			IFNDEF G9K_DISABLE_BITBUST
			include "../bitbuster/bitbuster.asm"
			ENDIF
		