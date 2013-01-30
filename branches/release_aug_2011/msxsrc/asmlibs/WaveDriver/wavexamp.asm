; Wavedriver example

				output "wavexamp.com"
		 	 
		 	 		 STRUCT	MAPPER
nrSegments	 BYTE			
priMapperSlot  	 BYTE
freeSegments	 BYTE
mapperSupport    WORD          ; mapper support routine address
AllocateSegment  BLOCK 3,0     ;  Allocate a 16k segment.
FreeSegment	 BLOCK 3,0     ;  Free a 16k segment.	
ReadSegmentByte  BLOCK 3,0     ;  Read byte from address A HL to A.
WriteSegmentByte BLOCK 3,0     ;  Write byte from E to address A HL.
InterSegCallReg  BLOCK 3,0     ;  Inter-segment call.     Address in IYh IX 	
InterSegCall     BLOCK 3,0     ;  Inter-segment call.     Address in line after the call instruction.
PutSegmentHL     BLOCK 3,0     ;  Put segment into page (HL).
GetSegmentHL     BLOCK 3,0     ;  Get current segment for page (HL)
PutSegment0      BLOCK 3,0     ;  Put segment into page 0.
GetSegment0      BLOCK 3,0     ;  Get current segment for page 0.
PutSegment1      BLOCK 3,0     ;  Put segment into page 1.
GetSegment1      BLOCK 3,0     ;  Get current segment for page 1.
PutSegment2      BLOCK 3,0     ;  Put segment into page 2.
GetSegment2      BLOCK 3,0     ;  Get current segment for page 2.
PutSegment3      BLOCK 3,0     ;  Not supported since page-3 must never be changed. Acts like a "NOP" if called.
GetSegment3      BLOCK 3,0     ;  Get current segment for page 3.
	 	 ENDS

		 STRUCT ENV_DATA
msxVersion	 BYTE	
sramSize	 WORD
waveDriverActive BYTE	 
		 ENDS
		 	 
			include "wavedriver.inc"
			include	"..\bdos\bdos.inc"
			include "..\bios\bios.inc"

			org	0100h
			
			;LD	A,(CMD_LENGTH)
			;OR	A,A
			;RET	Z
							
			call	InitMapper
			call	GetMsxVersion
			
			call	WaveDriverLoader.InitDriver
			JP	NZ,NoOpl4Found
			
			LD	B,1
			CallSegment WAVESEG,WAVE_DRV_JUMP.AllocateSongSegments
			
			LD	DE,file2
			CallSegment WAVESEG,WAVE_DRV_JUMP.LoadWaveKit
			RET	NZ	
			LD	DE,file
			LD	A,0
			CallSegment WAVESEG,WAVE_DRV_JUMP.LoadSong
			RET	NZ
				
			LD	A,0
			CallSegment WAVESEG,WAVE_DRV_JUMP.Play
			
			ld	de,.text
			BdosCall _STROUT
			
			BdosCall _CONIN
			
			call	WaveDriver.Release
					
			RET	
						
.text			DB	"*** press space to stop",CR,LF,LE				
			

NoOpl4Found:
			LD	DE,.text
			BdosCall _STROUT
			RET
			
.text			DB 	"*** No opl4 detected!!!",CR,LF,LE
			
mapper			MAPPER
envData			ENV_DATA
			
InitMapper:
			; Get mapper support routines
			XOR	A,A
			LD	DE,4*256+2
			CALL	EXT_BIO
			LD      (mapper.mapperSupport),HL
			LD	(mapper.nrSegments),A
			LD	A,B
			LD	(mapper.priMapperSlot),A
			LD	A,C
			LD	(mapper.freeSegments),A
			LD	DE,mapper.AllocateSegment
			LD	BC,16*3 ; size of jump table
			LDIR
			RET

GetMsxVersion:
			LD	A,(EXPTBL1)
			LD	hl,IDBYT2
			BIOS	RDSLT		 
			LD	(envData.msxVersion),a	
			EI
			RET

file:			DB "C:\\MBFILES\\WOLF\\CORAL.MWM",0
file2:			DB "C:\\MBFILES\\WOLF\\CORAL.MWK",0
			
			include "wavedriver.asm"
			include "file.asm"
			include	"..\bdos\bdos.asm"
			include "WaveDriverLoader.asm"
			
			
			
			
					




