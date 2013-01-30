			IFNDEF	WAVEDRIVERLOADER_ASM
			DEFINE	WAVEDRIVERLOADER_ASM
			
			include "wavedriver.inc"
			
			MODULE	WaveDriverLoader	

InitDriver:
; Output NZ, error loading
;         Z, Succefull	
; ERROR Code 		
			CALL	Opl4.Detect
			LD	A,2		; Return code no opl4 found
			RET	NZ
			
			LD	A,SYSTEM_SEGMENT
			LD	B,PRIMARY_MAPPER
			CALL	mapper.AllocateSegment
			JR	C,.noFreeRam
			LD	(WAVESEG),a
			PUSH	AF
			CALL    mapper.GetSegment2
			LD	(.OLD_SEG_PAGE2),A
			POP	AF
			CALL    mapper.PutSegment2 ; Set wavedriver segment on address #8000
		
			; Get msx version
			LD	A,(EXPTBL1)
			LD	hl,IDBYT2
			BIOS	RDSLT		
			CP	A,MSX_TURBO_R
			LD	DE,.WAVEZ80X_DRV
			JR	NZ,.loadDriver
			LD	DE,.WAVER800_DRV
.loadDriver:
			XOR	A,A
			CALL	File.FileOpen
			JR	NZ,.errorOpening
			PUSH	BC
			LD	HL,16384
			LD	DE,08000h
			CALL	File.FileRead
			POP	BC
			JR	NZ,.errorReading
			CALL	File.FileClose
						
			LD	HL,(mapper.mapperSupport)
			LD	(04000h+WAVE_DRV_JUMP.DOSJmpTab),HL     ; Set dos2 jump table address
	
			LD	HL,(04000h+WAVE_DRV_JUMP.playInt)
			LD	(PLAY_INT),HL
	
			
			IFNDEF	DISABLE_SET_EXTERNAL_FILE
			; Set pointer to external load functions here if needed
			; Set file functions
			LD	HL,File.FileOpen
			LD	(04000h+WAVE_DRV_JUMP.FileOpen + 1),HL

			LD	HL,File.FileRead
			LD	(04000h+WAVE_DRV_JUMP.FileRead + 1),HL

			LD	HL,File.FileClose
			LD	(04000h+WAVE_DRV_JUMP.FileClose + 1),HL
			ENDIF
			
			CALL	SetIntHook
			
			LD	A,(.OLD_SEG_PAGE2)
			CALL	 mapper.PutSegment2 
			
			XOR	A,A  ; Succesfull
			RET

.errorReading:
			CALL	File.FileClose		
.errorOpening:
			LD	A,(.OLD_SEG_PAGE2)
			CALL	mapper.PutSegment2 
			
			LD	A,(WAVESEG)
			LD	B,PRIMARY_MAPPER
			CALL	mapper.FreeSegment
.noFreeRam:
			LD	A,1
			OR	A,A ;
			RET	
				
.OLD_SEG_PAGE2		DB	0
.WAVER800_DRV		DB	"WAVER800.DRV",0
.WAVEZ80X_DRV		DB	"WAVEZ80X.DRV",0


SetIntHook:
			DI
			ld	hl,0fd9Ah
			ld	de,old_int
			ld	bc,5
			ldir		; save interrupt hook
		
		
			ld	de,21h
			ld	hl,(mapper.mapperSupport)
			add	hl,de
			ld	(Get_seg),hl	;Set get_p1 address
		
			ld	de,1Eh
			ld	hl,(mapper.mapperSupport)
			add	hl,de
			ld	(put_seg),hl	;Set put_p1 address
			ld	(put_old),hl
		
			ld	a,(WAVESEG)
			ld	(new_seg+1),a
		
			ld	a,(0F342h)
			ld	(Page_nmb),a
			ld	hl,opl4_int_han
			ld	de,0fb04h
			ld	bc,opl4_int_end-opl4_int_han
			ldir
		
			ld	hl,0FD9Ah	;Init On Hook 0FD9Ah a Jump to empty RS232 area
			ld	(hl),0C3h	; JP   
			inc	hl
			ld	(hl),004h	; 04
			inc	hl
			ld	(hl),0FBh	; FB
			EI
			ret


opl4_int_han:
			in	a,(0C4H)	; Put this shit in the RS232 area
			rla			; this is to prevent 50 or 60 CALLFs
			jr	nc,old_int
			db	0CDh	; Call get_seg_1
Get_seg:		dw	0
			push	af
new_seg:		ld	a,0
			db	0CDh	; Call Set_seg_1
put_seg:		dw	0
			rst	030h
Page_nmb:		db	0
PLAY_INT:		dw	0
			pop	af
			db	0CDh ;	CALL	 Set_seg_1
put_old:		dw	0

old_int:		; Old int from hook 0FD9Ah
			ret
			ret
			ret
			ret
			ret
opl4_int_end:

			ENDMODULE	
			ENDIF